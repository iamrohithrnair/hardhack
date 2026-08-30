#include "sensor_manager.h"

SensorManager::SensorManager() {
    imu_online = false;
    pmu_online = false;
    codec_online = false;
    audio_passthrough_enabled = false;
    
    vib_fifo_idx = 0;
    osc_idx = 0;
    last_telemetry_check = 0;
    
    cached_battery_pct = 95;
    cached_battery_volts = 4.12f;
    cached_charging = false;
    cached_temp = 28.5f;
    
    memset(vib_fifo, 0, sizeof(vib_fifo));
    memset(osc_waveform, 0, sizeof(osc_waveform));
    memset(audio_fifo, 0, sizeof(audio_fifo));
}

bool SensorManager::init() {
    Wire.begin(IIC_SDA, IIC_SCL, IIC_FREQ_HZ);
    delay(50);
    
    initIOExpander();
    initPMU();
    initIMU();
    initAudioCodec();
    
    return true;
}

bool SensorManager::initIOExpander() {
    if (io_expander.begin(IO_EXPANDER_ADDR, &Wire)) {
        // Configure LCD & Touch reset pins as outputs
        io_expander.pinMode(EXPANDER_PIN_LCD_RST, OUTPUT);
        io_expander.pinMode(EXPANDER_PIN_TP_RST, OUTPUT);
        io_expander.pinMode(EXPANDER_PIN_PMU_IRQ, INPUT);
        
        // Reset Display and Touchscreen
        io_expander.digitalWrite(EXPANDER_PIN_LCD_RST, LOW);
        io_expander.digitalWrite(EXPANDER_PIN_TP_RST, LOW);
        delay(20);
        io_expander.digitalWrite(EXPANDER_PIN_LCD_RST, HIGH);
        io_expander.digitalWrite(EXPANDER_PIN_TP_RST, HIGH);
        delay(50);
        return true;
    }
    return false;
}

bool SensorManager::initPMU() {
    pmu_online = pmu.begin(Wire, PMU_AXP2101_ADDR, IIC_SDA, IIC_SCL);
    if (pmu_online) {
        pmu.enableTemperatureMeasure();
        pmu.enableBattDetection();
        pmu.enableVbusVoltageMeasure();
        pmu.enableBattVoltageMeasure();
        pmu.enableSystemVoltageMeasure();
        pmu.setChargeTargetVoltage(3); // 4.2V target
        pmu.clearIrqStatus();
    }
    return pmu_online;
}

bool SensorManager::initIMU() {
    // Probe primary address (0x6B) then alternate (0x6A)
    if (imu.begin(Wire, IMU_QMI8658_ADDR_L, IIC_SDA, IIC_SCL) ||
        imu.begin(Wire, IMU_QMI8658_ADDR_H, IIC_SDA, IIC_SCL)) {
        imu_online = true;
        
        // Configure Accelerometer for 1000Hz ODR, 4G range, low-pass filter
        imu.configAccelerometer(SensorQMI8658::ACC_RANGE_4G, SensorQMI8658::ACC_ODR_1000Hz, SensorQMI8658::LPF_MODE_0);
        imu.enableAccelerometer();
        
        // Configure Gyroscope for 1000Hz ODR, 512 dps
        imu.configGyroscope(SensorQMI8658::GYR_RANGE_512DPS, SensorQMI8658::GYR_ODR_1000Hz, SensorQMI8658::LPF_MODE_0);
        imu.enableGyroscope();
        return true;
    }
    return false;
}

bool SensorManager::initAudioCodec() {
    // Configure Speaker PA pin
    pinMode(POWER_AMP_PA_IO, OUTPUT);
    digitalWrite(POWER_AMP_PA_IO, LOW); // Muted by default
    
    // Setup ES8311 Audio Codec
    es8311_handle_t es_handle = es8311_create(0, CODEC_ES8311_ADDR);
    if (es_handle != NULL) {
        const es8311_clock_config_t es_clk = {
            .mclk_inverted = false,
            .sclk_inverted = false,
            .mclk_from_mclk_pin = true,
            .mclk_frequency = AUDIO_SAMPLE_RATE * 256,
            .sample_frequency = AUDIO_SAMPLE_RATE
        };
        
        if (es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16) == ESP_OK) {
            es8311_sample_frequency_config(es_handle, es_clk.mclk_frequency, es_clk.sample_frequency);
            es8311_microphone_config(es_handle, false); // Analog mic
            es8311_voice_volume_set(es_handle, 85, NULL);
            es8311_microphone_gain_set(es_handle, (es8311_mic_gain_t)4);
            codec_online = true;
        }
    }
    
    // Initialize I2S
    i2s.setPins(I2S_BCK_IO, I2S_WS_IO, I2S_DO_IO, I2S_DI_IO, I2S_MCK_IO);
    if (i2s.begin(I2S_MODE_STD, AUDIO_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
        codec_online = true;
    }
    
    return codec_online;
}

void SensorManager::update() {
    // 1. Read High-Speed Accelerometer
    if (imu_online) {
        IMUdata acc;
        if (imu.getAccelerometer(acc.x, acc.y, acc.z)) {
            // Compute vibration magnitude vector (or use Z-axis for perpendicular contact)
            float vib_magnitude = sqrtf(acc.x * acc.x + acc.y * acc.y + acc.z * acc.z) - 1.0f; // Subtract 1g static gravity
            
            // Store in FFT circular FIFO
            vib_fifo[vib_fifo_idx] = vib_magnitude;
            vib_fifo_idx = (vib_fifo_idx + 1) % IMU_FIFO_SIZE;
            
            // Store in Oscilloscope display array
            osc_waveform[osc_idx] = vib_magnitude;
            osc_idx = (osc_idx + 1) % WAVEFORM_POINTS;
        }
    }
    
    // 2. Read I2S Microphone Audio Buffer
    if (codec_online) {
        size_t bytes_read = i2s.readBytes((char*)audio_fifo, sizeof(audio_fifo));
        if (bytes_read > 0 && audio_passthrough_enabled) {
            // Live Stethoscope Mode: stream amplified mic to speaker
            i2s.write((const uint8_t*)audio_fifo, bytes_read);
        }
    }
    
    // 3. Periodic PMU Battery Telemetry (every 2 seconds)
    uint32_t now = millis();
    if (now - last_telemetry_check > 2000) {
        last_telemetry_check = now;
        if (pmu_online) {
            cached_battery_pct = (uint8_t)pmu.getBatteryPercent();
            cached_battery_volts = pmu.getBattVoltage() / 1000.0f;
            cached_charging = pmu.isVbusIn();
            cached_temp = pmu.getTemperature();
        }
    }
}

void SensorManager::getVibrationBuffer(float* dest, size_t count) {
    if (count > IMU_FIFO_SIZE) count = IMU_FIFO_SIZE;
    
    // Unroll circular buffer so destination has contiguous chronological samples
    size_t start = vib_fifo_idx;
    for (size_t i = 0; i < count; i++) {
        dest[i] = vib_fifo[(start + i) % IMU_FIFO_SIZE];
    }
}

void SensorManager::getAudioBuffer(int16_t* dest, size_t count) {
    if (count > AUDIO_BUFFER_SAMPLES) count = AUDIO_BUFFER_SAMPLES;
    memcpy(dest, audio_fifo, count * sizeof(int16_t));
}

uint8_t SensorManager::getBatteryPercent() {
    return cached_battery_pct;
}

float SensorManager::getBatteryVoltage() {
    return cached_battery_volts;
}

bool SensorManager::isCharging() {
    return cached_charging;
}

float SensorManager::getBoardTemperature() {
    return cached_temp;
}

void SensorManager::setAudioPassthrough(bool enable) {
    audio_passthrough_enabled = enable;
    digitalWrite(POWER_AMP_PA_IO, enable ? HIGH : LOW);
}

void SensorManager::playAlertTone(uint16_t freq_hz, uint16_t duration_ms) {
    if (!codec_online) return;
    
    digitalWrite(POWER_AMP_PA_IO, HIGH);
    int16_t tone_buf[256];
    float phase = 0.0f;
    float phase_step = (2.0f * (float)M_PI * (float)freq_hz) / (float)AUDIO_SAMPLE_RATE;
    
    uint32_t total_samples = (AUDIO_SAMPLE_RATE * duration_ms) / 1000;
    while (total_samples > 0) {
        size_t chunk = (total_samples > 256) ? 256 : total_samples;
        for (size_t i = 0; i < chunk; i++) {
            tone_buf[i] = (int16_t)(sinf(phase) * 16000.0f);
            phase += phase_step;
            if (phase >= 2.0f * (float)M_PI) phase -= 2.0f * (float)M_PI;
        }
        i2s.write((const uint8_t*)tone_buf, chunk * sizeof(int16_t));
        total_samples -= chunk;
    }
    
    if (!audio_passthrough_enabled) {
        digitalWrite(POWER_AMP_PA_IO, LOW);
    }
}
