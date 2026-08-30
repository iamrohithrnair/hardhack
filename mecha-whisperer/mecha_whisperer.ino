/**
 * ============================================================================
 * MECHA-WHISPERER: The Stethoscope for Machines
 * Predictive Maintenance & Micro-Vibration Anomaly Diagnostic Tool
 * Target Hardware: Waveshare ESP32-S3-Touch-AMOLED-1.8
 * ============================================================================
 */

#include <Arduino.h>
#include "src/hardware_config.h"
#include "src/dsp_engine.h"
#include "src/sensor_manager.h"
#include "src/ui_engine.h"
#include "HWCDC.h"

HWCDC USBSerial;

// Core Engines
DSPEngine dsp_engine;
SensorManager sensor_mgr;
UIEngine ui_engine;

// Inter-Task Communication & Synchronization
SemaphoreHandle_t data_mutex;
float raw_vibration_samples[FFT_SIZE];
int16_t raw_audio_samples[AUDIO_BUFFER_SAMPLES];
DiagnosticMetrics shared_metrics;
float shared_osc_waveform[WAVEFORM_POINTS];
DiagnosticState last_state = STATE_HEALTHY;

// FreeRTOS Task Handles
TaskHandle_t dsp_task_handle = NULL;

void SensorDSPTask(void *pvParameters) {
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(IMU_UPDATE_INTERVAL_MS);

    while (true) {
        sensor_mgr.update();

        if (dsp_engine.isDemoMode()) {
            dsp_engine.generateDemoSamples(raw_vibration_samples, FFT_SIZE, IMU_SAMPLE_RATE_HZ);
        } else {
            sensor_mgr.getVibrationBuffer(raw_vibration_samples, FFT_SIZE);
        }
        sensor_mgr.getAudioBuffer(raw_audio_samples, AUDIO_BUFFER_SAMPLES);

        dsp_engine.processVibration(raw_vibration_samples, FFT_SIZE, IMU_SAMPLE_RATE_HZ);
        dsp_engine.processAudio(raw_audio_samples, AUDIO_BUFFER_SAMPLES);

        if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
            shared_metrics = dsp_engine.getMetrics();
            const float* osc = sensor_mgr.getOscilloscopeWaveform();
            if (dsp_engine.isDemoMode()) {
                for (size_t i = 0; i < WAVEFORM_POINTS; i++) {
                    shared_osc_waveform[i] = raw_vibration_samples[i % FFT_SIZE];
                }
            } else {
                for (size_t i = 0; i < WAVEFORM_POINTS; i++) {
                    shared_osc_waveform[i] = osc[i];
                }
            }
            xSemaphoreGive(data_mutex);
        }

        if (shared_metrics.state == STATE_CRITICAL_UNBALANCE && last_state != STATE_CRITICAL_UNBALANCE) {
            sensor_mgr.playAlertTone(1800, 150);
        }
        last_state = shared_metrics.state;

        vTaskDelayUntil(&last_wake_time, period);
    }
}

void setup() {
    USBSerial.begin(115200);
    USBSerial.setTxTimeoutMs(0);

    USBSerial.println("\n============================================");
    USBSerial.println("  MECHA-WHISPERER: The Stethoscope for Machines");
    USBSerial.println("  Waveshare ESP32-S3 Touch AMOLED 1.8-inch");
    USBSerial.println("============================================\n");

    data_mutex = xSemaphoreCreateMutex();
    dsp_engine.init();
    sensor_mgr.init();
    ui_engine.init();

    xTaskCreatePinnedToCore(
        SensorDSPTask,
        "DSP_Task",
        8192,
        NULL,
        5,
        &dsp_task_handle,
        0
    );
}

void loop() {
    TouchPoint tp;
    tp.is_pressed = false;
    tp.x = 0;
    tp.y = 0;

    ui_engine.handleTouch(tp, dsp_engine, sensor_mgr);

    DiagnosticMetrics local_metrics;
    float local_osc[WAVEFORM_POINTS];

    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        local_metrics = shared_metrics;
        memcpy(local_osc, shared_osc_waveform, sizeof(local_osc));
        xSemaphoreGive(data_mutex);
    }

    ui_engine.render(local_metrics, local_osc, WAVEFORM_POINTS, sensor_mgr, dsp_engine);

    static uint32_t last_serial = 0;
    if (millis() - last_serial > 100) {
        last_serial = millis();
        if (USBSerial) {
            USBSerial.printf("{\"rpm\":%lu,\"f0\":%.1f,\"rms\":%.3f,\"kurt\":%.2f,\"iso\":%.2f,\"score\":%d,\"state\":%d}\n",
                local_metrics.estimated_rpm,
                local_metrics.peak_freq_hz,
                local_metrics.rms_acceleration_g,
                local_metrics.kurtosis,
                local_metrics.iso_vibration_vel,
                local_metrics.health_score,
                (int)local_metrics.state
            );
        }
    }

    delay(10);
}
