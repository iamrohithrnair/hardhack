#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "hardware_config.h"
#include "SensorQMI8658.hpp"
#include "XPowersLib.h"
#include <Adafruit_XCA9554.h>
#include "ESP_I2S.h"
#include "es8311.h"

#define WAVEFORM_POINTS     180     // Points displayed on the AMOLED oscilloscope

class SensorManager {
public:
    SensorManager();
    bool init();
    
    // Fast Polling loop (called in high-priority Core 0 FreeRTOS task)
    void update();
    
    // Retrieve latest time-domain vibration samples for FFT / Oscilloscope
    void getVibrationBuffer(float* dest, size_t count);
    
    // Retrieve latest audio samples
    void getAudioBuffer(int16_t* dest, size_t count);
    
    // Waveform history for rendering oscilloscope on AMOLED
    const float* getOscilloscopeWaveform() const { return osc_waveform; }
    size_t getOscilloscopeCount() const { return WAVEFORM_POINTS; }
    
    // Battery & Telemetry
    uint8_t getBatteryPercent();
    float getBatteryVoltage();
    bool isCharging();
    float getBoardTemperature();
    
    // Audio Stethoscope Pass-Through
    void setAudioPassthrough(bool enable);
    bool isAudioPassthrough() const { return audio_passthrough_enabled; }
    void playAlertTone(uint16_t freq_hz, uint16_t duration_ms);

private:
    SensorQMI8658 imu;
    XPowersPMU pmu;
    Adafruit_XCA9554 io_expander;
    I2SClass i2s;
    
    bool imu_online;
    bool pmu_online;
    bool codec_online;
    bool audio_passthrough_enabled;
    
    // Circular Buffers
    float vib_fifo[IMU_FIFO_SIZE];
    size_t vib_fifo_idx;
    
    float osc_waveform[WAVEFORM_POINTS];
    size_t osc_idx;
    
    int16_t audio_fifo[AUDIO_BUFFER_SAMPLES];
    
    uint32_t last_telemetry_check;
    uint8_t cached_battery_pct;
    float cached_battery_volts;
    bool cached_charging;
    float cached_temp;
    
    bool initIOExpander();
    bool initPMU();
    bool initIMU();
    bool initAudioCodec();
};
