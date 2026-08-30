/**
 * ============================================================================
 * MECHA-WHISPERER: The Stethoscope for Machines
 * Predictive Maintenance & Micro-Vibration Anomaly Diagnostic Tool
 * Target Hardware: Waveshare ESP32-S3-Touch-AMOLED-1.8
 * ============================================================================
 */

#include <Arduino.h>
#include "hardware_config.h"
#include "dsp_engine.h"
#include "sensor_manager.h"
#include "ui_engine.h"
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

/**
 * High-Priority Sensor Acquisition & DSP Task (Runs on Core 0)
 * 1000 Hz Accelerometer sampling + 16 kHz I2S Mic + Radix-2 FFT Engine
 */
void SensorDSPTask(void *pvParameters) {
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(IMU_UPDATE_INTERVAL_MS); // 20ms (50 Hz DSP cycle)

    while (true) {
        // 1. Update Hardware Sensors (IMU, Mic, PMU)
        sensor_mgr.update();

        // 2. Fetch raw vibration and audio buffers
        if (dsp_engine.isDemoMode()) {
            // Generate synthetic signal for live pitch demo
            dsp_engine.generateDemoSamples(raw_vibration_samples, FFT_SIZE, IMU_SAMPLE_RATE_HZ);
        } else {
            sensor_mgr.getVibrationBuffer(raw_vibration_samples, FFT_SIZE);
        }
        sensor_mgr.getAudioBuffer(raw_audio_samples, AUDIO_BUFFER_SAMPLES);

        // 3. Execute Real-Time FFT & Vibration Severity Extraction
        dsp_engine.processVibration(raw_vibration_samples, FFT_SIZE, IMU_SAMPLE_RATE_HZ);
        dsp_engine.processAudio(raw_audio_samples, AUDIO_BUFFER_SAMPLES);

        // 4. Thread-Safe Copy of Diagnostic State to UI
        if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
            shared_metrics = dsp_engine.getMetrics();
            const float* osc = sensor_mgr.getOscilloscopeWaveform();
            if (dsp_engine.isDemoMode()) {
                // In demo mode, use the synthetic wave for oscilloscope
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

        // 5. Sound Alert Tone on Severe Anomaly Transition
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

    // Initialize Mutex
    data_mutex = xSemaphoreCreateMutex();

    // 1. Initialize DSP Engine
    dsp_engine.init();

    // 2. Initialize Hardware Sensors & Power Management
    sensor_mgr.init();

    // 3. Initialize 368x448 AMOLED Display & Touch
    ui_engine.init();

    // 4. Spawn Core 0 Real-Time Sensor & DSP Task
    xTaskCreatePinnedToCore(
        SensorDSPTask,
        "DSP_Task",
        8192,
        NULL,
        5,              // High priority
        &dsp_task_handle,
        0               // Core 0
    );

    USBSerial.println("[SYSTEM] Dual-core DSP & UI pipeline running.");
}

void loop() {
    // 1. Read Capacitive Touch Input
    TouchPoint tp;
    tp.is_pressed = false;
    tp.x = 0;
    tp.y = 0;

    // Check Touch (DriveBus FT3168/CST820)
    // Note: Touch driver interrupt flag check can also be wired here
    ui_engine.handleTouch(tp, dsp_engine, sensor_mgr);

    // 2. Local Snapshot of Current Metrics for UI Rendering
    DiagnosticMetrics local_metrics;
    float local_osc[WAVEFORM_POINTS];

    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        local_metrics = shared_metrics;
        memcpy(local_osc, shared_osc_waveform, sizeof(local_osc));
        xSemaphoreGive(data_mutex);
    }

    // 3. Render AMOLED Frame (Runs on Core 1 at ~30-60 FPS)
    ui_engine.render(local_metrics, local_osc, WAVEFORM_POINTS, sensor_mgr, dsp_engine);

    // 4. Output Serial JSON Telemetry for PC/Web Live Plotting
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

    delay(10); // Yield to FreeRTOS Core 1 idle task
}
