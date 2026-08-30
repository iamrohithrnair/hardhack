#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bsp/esp-bsp.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "qmi8658.h"

#include "dsp_engine.h"
#include "ui_engine.h"
#include "wifi_ble_manager.h"

static const char *TAG = "mecha_whisperer";

#define BOOT_BUTTON_GPIO     GPIO_NUM_0
#define IMU_SAMPLE_PERIOD_MS 4     // 250 Hz sampling rate
#define DSP_PERIOD_MS        20    // 50 Hz DSP & UI refresh rate
#define BUFFER_SIZE          FFT_SIZE

static qmi8658_dev_t s_imu = {0};
static bool s_imu_ready = false;

static float s_vibration_ring[BUFFER_SIZE];
static size_t s_ring_idx = 0;
static float s_vibration_snapshot[BUFFER_SIZE];
static SemaphoreHandle_t s_data_mutex = NULL;

static esp_err_t imu_init_board(void) {
    i2c_master_bus_handle_t bus_handle = bsp_i2c_get_handle();
    if (bus_handle == NULL) {
        ESP_LOGE(TAG, "BSP I2C bus not initialized");
        return ESP_FAIL;
    }

    uint8_t candidates[] = {QMI8658_ADDRESS_HIGH, QMI8658_ADDRESS_LOW};
    uint8_t imu_addr = 0;
    for (size_t i = 0; i < sizeof(candidates); i++) {
        if (i2c_master_probe(bus_handle, candidates[i], 50) == ESP_OK) {
            imu_addr = candidates[i];
            break;
        }
    }

    if (imu_addr == 0) {
        ESP_LOGW(TAG, "QMI8658 not detected via I2C probe; will use demo mode fallback if needed");
        return ESP_ERR_NOT_FOUND;
    }

    ESP_LOGI(TAG, "Detected QMI8658 at 0x%02x", imu_addr);
    esp_err_t ret = qmi8658_init(&s_imu, bus_handle, imu_addr);
    if (ret != ESP_OK) return ret;

    // Reset & Configure
    qmi8658_reset(&s_imu);
    vTaskDelay(pdMS_TO_TICKS(10));
    qmi8658_set_accel_range(&s_imu, QMI8658_ACCEL_RANGE_8G);
    qmi8658_set_accel_odr(&s_imu, QMI8658_ACCEL_ODR_250HZ);
    qmi8658_set_gyro_range(&s_imu, QMI8658_GYRO_RANGE_512DPS);
    qmi8658_set_gyro_odr(&s_imu, QMI8658_GYRO_ODR_250HZ);
    qmi8658_enable_sensors(&s_imu, QMI8658_ENABLE_ACCEL | QMI8658_ENABLE_GYRO);

    s_imu_ready = true;
    ESP_LOGI(TAG, "QMI8658 IMU configured at 250 Hz, ±8g");
    return ESP_OK;
}

static void imu_sampler_task(void *pvParameters) {
    (void)pvParameters;
    qmi8658_data_t imu_data;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    float s_dc_ax = 0.0f, s_dc_ay = 0.0f, s_dc_az = 0.0f;

    while (true) {
        if (qmi8658_read_sensor_data(&s_imu, &imu_data) == ESP_OK) {
            s_imu_ready = true;
            // Exponential moving average for gravity cancellation (AC high-pass filter)
            s_dc_ax = s_dc_ax * 0.98f + imu_data.accelX * 0.02f;
            s_dc_ay = s_dc_ay * 0.98f + imu_data.accelY * 0.02f;
            s_dc_az = s_dc_az * 0.98f + imu_data.accelZ * 0.02f;

            float ac_x = imu_data.accelX - s_dc_ax;
            float ac_y = imu_data.accelY - s_dc_ay;
            float ac_z = imu_data.accelZ - s_dc_az;

            // Compute dynamic AC vibration magnitude in Gs
            float dynamic_vib = sqrtf(ac_x*ac_x + ac_y*ac_y + ac_z*ac_z) / 1000.0f;

            if (xSemaphoreTake(s_data_mutex, 0) == pdTRUE) {
                s_vibration_ring[s_ring_idx] = dynamic_vib;
                s_ring_idx = (s_ring_idx + 1) % BUFFER_SIZE;
                xSemaphoreGive(s_data_mutex);
            }
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(IMU_SAMPLE_PERIOD_MS));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "=============================================");
    ESP_LOGI(TAG, "  MECHA-WHISPERER: Edge Acoustic Stethoscope ");
    ESP_LOGI(TAG, "  Target: Waveshare ESP32-S3-Touch-AMOLED-1.8");
    ESP_LOGI(TAG, "=============================================");

    s_data_mutex = xSemaphoreCreateMutex();
    dsp_engine_init();

    // 0. Initialize BOOT Button (GPIO 0)
    gpio_config_t btn_cfg = {
        .pin_bit_mask = (1ULL << BOOT_BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&btn_cfg);

    // 1. Initialize Display and LVGL
    lv_display_t *display = bsp_display_start();
    if (display == NULL) {
        ESP_LOGE(TAG, "Display initialization failed");
        return;
    }
    bsp_display_brightness_set(100);

    // 2. Initialize Wireless Wi-Fi SoftAP & WebSocket Server
    wifi_ble_manager_init();

    // 3. Initialize IMU
    bsp_i2c_init();
    imu_init_board();

    // 4. Build UI
    if (bsp_display_lock(1000)) {
        ESP_LOGI(TAG, "Initializing UI Engine...");
        ui_engine_init();
        bsp_display_unlock();
        ESP_LOGI(TAG, "UI Engine initialized successfully");
    } else {
        ESP_LOGE(TAG, "Failed to acquire display lock for UI init!");
    }

    // 5. Start high-speed IMU sampling task on Core 0
    xTaskCreatePinnedToCore(imu_sampler_task, "imu_task", 4096, NULL, 5, NULL, 0);

    // 6. Main UI & DSP Loop
    static bool s_btn_last_state = true;

    while (true) {
        // Handle BOOT Switch press to cycle views
        bool btn_state = gpio_get_level(BOOT_BUTTON_GPIO);
        if (!btn_state && s_btn_last_state) {
            ESP_LOGI(TAG, "BOOT switch clicked! Switching graph view...");
            if (bsp_display_lock(50)) {
                ui_engine_next_view();
                bsp_display_unlock();
            }
        }
        s_btn_last_state = btn_state;

        // Prepare linear sample buffer
        if (dsp_engine_is_demo_mode() || !s_imu_ready) {
            dsp_engine_generate_demo_samples(s_vibration_snapshot, BUFFER_SIZE, 250.0f);
        } else {
            if (xSemaphoreTake(s_data_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                size_t start = s_ring_idx;
                for (size_t i = 0; i < BUFFER_SIZE; i++) {
                    s_vibration_snapshot[i] = s_vibration_ring[(start + i) % BUFFER_SIZE];
                }
                xSemaphoreGive(s_data_mutex);
            }
        }

        // Run DSP FFT & Anomaly Engine
        dsp_engine_process_vibration(s_vibration_snapshot, BUFFER_SIZE, 250.0f);
        const DiagnosticMetrics *metrics = dsp_engine_get_metrics();

        // Update UI
        if (bsp_display_lock(50)) {
            ui_engine_update(metrics, s_vibration_snapshot, BUFFER_SIZE);
            bsp_display_unlock();
        }

        // Emit Serial & Wireless Telemetry
        static uint32_t last_log = 0;
        uint32_t now = esp_timer_get_time() / 1000;
        if (now - last_log >= 100) {
            last_log = now;
            char json_buf[192];
            snprintf(json_buf, sizeof(json_buf),
                   "{\"rpm\":%lu,\"f0\":%.1f,\"rms\":%.3f,\"kurt\":%.2f,\"iso\":%.2f,\"score\":%d,\"state\":%d,\"view\":%d}\n",
                   (unsigned long)metrics->estimated_rpm,
                   metrics->peak_freq_hz,
                   metrics->rms_acceleration_g,
                   metrics->kurtosis,
                   metrics->iso_vibration_vel,
                   metrics->health_score,
                   (int)metrics->state,
                   (int)ui_engine_get_view());
            printf("%s", json_buf);
            wifi_ble_broadcast_telemetry(json_buf);
        }

        vTaskDelay(pdMS_TO_TICKS(DSP_PERIOD_MS));
    }
}
