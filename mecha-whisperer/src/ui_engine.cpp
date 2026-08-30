#include "ui_engine.h"

UIEngine::UIEngine() {
    bus = nullptr;
    gfx = nullptr;
    current_screen = UI_MODE_MAIN_HUD;
    need_full_redraw = true;
    frame_count = 0;
    last_fps_time = 0;
    current_fps = 60;
}

bool UIEngine::init() {
    // 1. Initialize QSPI Display Bus
    bus = new Arduino_ESP32QSPI(
        LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3
    );
    
    // 2. Initialize SH8601 AMOLED Display Driver (Auto-compatible with 368x448)
    gfx = new Arduino_SH8601(
        bus, GFX_NOT_DEFINED, 0, LCD_WIDTH, LCD_HEIGHT
    );
    
    if (!gfx->begin()) {
        return false;
    }
    
    gfx->fillScreen(COLOR_BLACK);
    gfx->setBrightness(255); // Full vibrant AMOLED luminance
    
    // 3. Initialize Touch Controller (FT3168 on V1 or CST820 on V2)
    iic_bus = std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);
    touch_v1 = std::unique_ptr<Arduino_IIC>(new Arduino_FT3x68(iic_bus, TOUCH_FT3168_ADDR, DRIVEBUS_DEFAULT_VALUE, TP_INT));
    touch_v1->begin();
    
    return true;
}

void UIEngine::setBrightness(uint8_t brightness) {
    if (gfx) {
        gfx->setBrightness(brightness);
    }
}

uint16_t UIEngine::getStateColor(DiagnosticState state) {
    switch (state) {
        case STATE_HEALTHY:
            return COLOR_CYAN;
        case STATE_WARNING:
            return COLOR_AMBER;
        case STATE_CRITICAL_UNBALANCE:
            return COLOR_ALERT_RED;
        case STATE_BEARING_DAMAGE:
            return COLOR_ALERT_ORANGE;
        case STATE_LOOSE_MOUNT:
            return COLOR_AMBER;
        case STATE_CALIBRATING:
            return COLOR_WHITE;
        default:
            return COLOR_CYAN;
    }
}

uint16_t UIEngine::getStateBgColor(DiagnosticState state) {
    switch (state) {
        case STATE_HEALTHY:
            return 0x0110; // Deep cyan tint
        case STATE_WARNING:
            return 0x2100; // Deep amber tint
        case STATE_CRITICAL_UNBALANCE:
            return 0x3800; // Deep red alert tint
        case STATE_BEARING_DAMAGE:
            return 0x3080; // Deep orange tint
        case STATE_LOOSE_MOUNT:
            return 0x2100;
        default:
            return COLOR_DARK_GRAY;
    }
}

void UIEngine::render(const DiagnosticMetrics& metrics, const float* osc_waveform, size_t osc_count,
                      SensorManager& sensor_mgr, DSPEngine& dsp_engine) {
    if (!gfx) return;
    
    // Calculate FPS
    frame_count++;
    uint32_t now = millis();
    if (now - last_fps_time >= 1000) {
        current_fps = frame_count;
        frame_count = 0;
        last_fps_time = now;
    }
    
    if (need_full_redraw) {
        gfx->fillScreen(COLOR_BLACK);
        need_full_redraw = false;
    }
    
    // Render Top Status Header (Height: 38px)
    renderTopBar(metrics, sensor_mgr);
    
    // Render Selected Screen
    switch (current_screen) {
        case UI_MODE_MAIN_HUD:
            renderMainHUD(metrics, osc_waveform, osc_count);
            break;
        case UI_MODE_SPECTRUM_ZOOM:
            renderSpectrumZoom(metrics);
            break;
        case UI_MODE_ISO_REPORT:
            renderISOReport(metrics, sensor_mgr);
            break;
        case UI_MODE_AUDIO_STETHOSCOPE:
            renderAudioStethoscope(metrics, sensor_mgr);
            break;
    }
    
    // Render Bottom Quick Action Navigation Bar (Height: 52px)
    renderBottomNav(metrics, dsp_engine);
}

void UIEngine::renderTopBar(const DiagnosticMetrics& metrics, SensorManager& sensor_mgr) {
    // Header Bar Background (y: 0 to 36)
    gfx->fillRect(0, 0, LCD_WIDTH, 36, COLOR_DARK_GRAY);
    gfx->drawFastHLine(0, 36, LCD_WIDTH, getStateColor(metrics.state));
    
    // Device Title
    gfx->setTextColor(COLOR_WHITE, COLOR_DARK_GRAY);
    gfx->setTextSize(2);
    gfx->setCursor(8, 10);
    gfx->print("MECHA-WHISPERER");
    
    // Mode / Heartbeat Badge
    uint16_t state_col = getStateColor(metrics.state);
    gfx->fillCircle(240, 18, 5, state_col);
    
    // Battery & Telemetry
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_CYAN, COLOR_DARK_GRAY);
    gfx->setCursor(260, 8);
    gfx->printf("BAT: %d%%", sensor_mgr.getBatteryPercent());
    
    gfx->setCursor(260, 20);
    gfx->setTextColor(COLOR_LIGHT_GRAY, COLOR_DARK_GRAY);
    gfx->printf("FPS: %d", current_fps);
}

void UIEngine::renderMainHUD(const DiagnosticMetrics& metrics, const float* osc_waveform, size_t osc_count) {
    uint16_t main_col = getStateColor(metrics.state);
    uint16_t bg_tint  = getStateBgColor(metrics.state);
    
    // ==========================================
    // 1. HEALTH SCORE & DIAGNOSIS CARD (y: 42 to 135)
    // ==========================================
    gfx->fillRoundRect(8, 42, LCD_WIDTH - 16, 92, 8, bg_tint);
    gfx->drawRoundRect(8, 42, LCD_WIDTH - 16, 92, 8, main_col);
    
    // Health Arc Gauge on the Left
    drawHealthArc(58, 88, 38, metrics.health_score, metrics.state);
    
    // Diagnostic State & RPM Information on the Right
    gfx->setCursor(112, 52);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_LIGHT_GRAY, bg_tint);
    gfx->print("MACHINE HEALTH SCORE");
    
    // Big Health Percentage
    gfx->setCursor(112, 66);
    gfx->setTextSize(3);
    gfx->setTextColor(main_col, bg_tint);
    gfx->printf("%d%%", metrics.health_score);
    
    // Diagnosis Text Banner
    gfx->setCursor(112, 96);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_WHITE, bg_tint);
    if (metrics.state == STATE_HEALTHY) {
        gfx->print("STATUS: HEALTHY NOMINAL");
    } else if (metrics.state == STATE_CRITICAL_UNBALANCE) {
        gfx->setTextColor(COLOR_ALERT_RED, bg_tint);
        gfx->print("CRITICAL: ROTOR IMBALANCE");
    } else if (metrics.state == STATE_BEARING_DAMAGE) {
        gfx->setTextColor(COLOR_ALERT_ORANGE, bg_tint);
        gfx->print("FAULT: BEARING IMPACTS");
    } else {
        gfx->print("STATUS: VIBRATION ANOMALY");
    }
    
    // Peak Frequency & RPM tag
    gfx->setCursor(112, 114);
    gfx->setTextColor(COLOR_CYAN, bg_tint);
    gfx->printf("F0: %.1f Hz (%lu RPM)", metrics.peak_freq_hz, metrics.estimated_rpm);
    
    // ==========================================
    // 2. OSCILLOSCOPE WAVEFORM CARD (y: 140 to 265)
    // ==========================================
    gfx->fillRoundRect(8, 140, LCD_WIDTH - 16, 124, 8, COLOR_BLACK);
    gfx->drawRoundRect(8, 140, LCD_WIDTH - 16, 124, 8, COLOR_DARK_GRAY);
    
    // Oscilloscope Title & Grid
    gfx->setCursor(16, 146);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_LIGHT_GRAY, COLOR_BLACK);
    gfx->print("LIVE MICRO-VIBRATION STETHOSCOPE");
    
    gfx->setCursor(240, 146);
    gfx->setTextColor(main_col, COLOR_BLACK);
    gfx->printf("Pk: %.2fg", metrics.peak_to_peak_g);
    
    drawOscilloscope(16, 162, LCD_WIDTH - 32, 94, osc_waveform, osc_count, metrics.state);
    
    // ==========================================
    // 3. FFT FREQUENCY SPECTRUM (y: 270 to 345)
    // ==========================================
    gfx->fillRoundRect(8, 270, LCD_WIDTH - 16, 74, 8, COLOR_BLACK);
    gfx->drawRoundRect(8, 270, LCD_WIDTH - 16, 74, 8, COLOR_DARK_GRAY);
    
    gfx->setCursor(16, 274);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_LIGHT_GRAY, COLOR_BLACK);
    gfx->print("VIBRATION FFT SPECTRUM (0 - 500 Hz)");
    
    drawFFTBars(16, 288, LCD_WIDTH - 32, 50, metrics.visual_spectrum, metrics.visual_spectrum_peak, BARS_COUNT, metrics.state);
    
    // ==========================================
    // 4. METRICS ROW (y: 350 to 390)
    // ==========================================
    int16_t card_w = (LCD_WIDTH - 28) / 3;
    
    // Card 1: RMS Acceleration
    gfx->fillRoundRect(8, 350, card_w, 40, 6, COLOR_DARK_GRAY);
    gfx->setCursor(14, 354);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_LIGHT_GRAY, COLOR_DARK_GRAY);
    gfx->print("RMS ACCEL");
    gfx->setCursor(14, 368);
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_WHITE, COLOR_DARK_GRAY);
    gfx->printf("%.2fg", metrics.rms_acceleration_g);
    
    // Card 2: Kurtosis (Impulse severity)
    gfx->fillRoundRect(12 + card_w, 350, card_w, 40, 6, COLOR_DARK_GRAY);
    gfx->setCursor(18 + card_w, 354);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_LIGHT_GRAY, COLOR_DARK_GRAY);
    gfx->print("KURTOSIS");
    gfx->setCursor(18 + card_w, 368);
    gfx->setTextSize(2);
    gfx->setTextColor((metrics.kurtosis > 4.0f) ? COLOR_ALERT_ORANGE : COLOR_WHITE, COLOR_DARK_GRAY);
    gfx->printf("%.1f", metrics.kurtosis);
    
    // Card 3: ISO 10816 Class
    gfx->fillRoundRect(16 + card_w * 2, 350, card_w, 40, 6, COLOR_DARK_GRAY);
    gfx->setCursor(22 + card_w * 2, 354);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_LIGHT_GRAY, COLOR_DARK_GRAY);
    gfx->print("ISO-10816");
    gfx->setCursor(22 + card_w * 2, 368);
    gfx->setTextSize(2);
    if (metrics.iso_vibration_vel < 1.12f) {
        gfx->setTextColor(COLOR_NEON_GREEN, COLOR_DARK_GRAY);
        gfx->print("CLS A");
    } else if (metrics.iso_vibration_vel < 2.8f) {
        gfx->setTextColor(COLOR_AMBER, COLOR_DARK_GRAY);
        gfx->print("CLS B");
    } else {
        gfx->setTextColor(COLOR_ALERT_RED, COLOR_DARK_GRAY);
        gfx->print("CLS D");
    }
}

void UIEngine::renderSpectrumZoom(const DiagnosticMetrics& metrics) {
    gfx->setCursor(16, 44);
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_CYAN, COLOR_BLACK);
    gfx->print("FFT SPECTRAL ANALYZER");
    
    gfx->setCursor(16, 68);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_WHITE, COLOR_BLACK);
    gfx->printf("Dominant 1X RPM Peak: %.1f Hz (%lu RPM)\n", metrics.peak_freq_hz, metrics.estimated_rpm);
    gfx->printf("Harmonic Distortion THD: %.1f%%\n", metrics.harmonic_distortion * 100.0f);
    
    // Big 24-Band Vibration FFT
    gfx->drawRect(12, 100, LCD_WIDTH - 24, 140, COLOR_DARK_GRAY);
    drawFFTBars(16, 104, LCD_WIDTH - 32, 130, metrics.visual_spectrum, metrics.visual_spectrum_peak, BARS_COUNT, metrics.state);
    
    // Acoustic Microphone Spectrum
    gfx->setCursor(16, 250);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_MAGENTA, COLOR_BLACK);
    gfx->print("ACOUSTIC / AUDIO SPECTRUM (0 - 8 kHz)");
    
    gfx->drawRect(12, 266, LCD_WIDTH - 24, 110, COLOR_DARK_GRAY);
    drawFFTBars(16, 270, LCD_WIDTH - 32, 100, metrics.acoustic_spectrum, metrics.acoustic_spectrum, BARS_COUNT, STATE_HEALTHY);
}

void UIEngine::renderISOReport(const DiagnosticMetrics& metrics, SensorManager& sensor_mgr) {
    gfx->setCursor(16, 44);
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_WHITE, COLOR_BLACK);
    gfx->print("MACHINE HEALTH REPORT");
    
    uint16_t col = getStateColor(metrics.state);
    gfx->fillRoundRect(12, 70, LCD_WIDTH - 24, 70, 8, getStateBgColor(metrics.state));
    gfx->drawRoundRect(12, 70, LCD_WIDTH - 24, 70, 8, col);
    
    gfx->setCursor(20, 80);
    gfx->setTextSize(2);
    gfx->setTextColor(col, getStateBgColor(metrics.state));
    gfx->print(metrics.diagnosis_text);
    
    gfx->setCursor(20, 106);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_WHITE, getStateBgColor(metrics.state));
    gfx->print(metrics.recommendation);
    
    // Detailed Metric Breakdown Table
    int y = 150;
    gfx->setTextColor(COLOR_LIGHT_GRAY, COLOR_BLACK);
    gfx->setTextSize(1);
    
    gfx->setCursor(16, y); gfx->print("Vibration Severity (ISO 10816):");
    gfx->setTextColor(COLOR_WHITE, COLOR_BLACK);
    gfx->setCursor(250, y); gfx->printf("%.2f mm/s", metrics.iso_vibration_vel);
    
    y += 24;
    gfx->setTextColor(COLOR_LIGHT_GRAY, COLOR_BLACK);
    gfx->setCursor(16, y); gfx->print("Rotational Speed (RPM):");
    gfx->setTextColor(COLOR_CYAN, COLOR_BLACK);
    gfx->setCursor(250, y); gfx->printf("%lu RPM", metrics.estimated_rpm);
    
    y += 24;
    gfx->setTextColor(COLOR_LIGHT_GRAY, COLOR_BLACK);
    gfx->setCursor(16, y); gfx->print("Impact Factor (Kurtosis):");
    gfx->setTextColor(COLOR_WHITE, COLOR_BLACK);
    gfx->setCursor(250, y); gfx->printf("%.2f", metrics.kurtosis);
    
    y += 24;
    gfx->setTextColor(COLOR_LIGHT_GRAY, COLOR_BLACK);
    gfx->setCursor(16, y); gfx->print("Acoustic Noise Level:");
    gfx->setTextColor(COLOR_MAGENTA, COLOR_BLACK);
    gfx->setCursor(250, y); gfx->printf("%.1f dBA", metrics.acoustic_db);
    
    y += 24;
    gfx->setTextColor(COLOR_LIGHT_GRAY, COLOR_BLACK);
    gfx->setCursor(16, y); gfx->print("Board PMU Temperature:");
    gfx->setTextColor(COLOR_WHITE, COLOR_BLACK);
    gfx->setCursor(250, y); gfx->printf("%.1f C", sensor_mgr.getBoardTemperature());
    
    y += 28;
    gfx->drawFastHLine(16, y, LCD_WIDTH - 32, COLOR_DARK_GRAY);
    y += 10;
    gfx->setCursor(16, y);
    gfx->setTextColor(COLOR_AMBER, COLOR_BLACK);
    gfx->print("PREDICTIVE LIFETIME ESTIMATE (RUL):");
    y += 16;
    gfx->setCursor(16, y);
    gfx->setTextSize(2);
    if (metrics.state == STATE_HEALTHY) {
        gfx->setTextColor(COLOR_NEON_GREEN, COLOR_BLACK);
        gfx->print("> 15,000 Hours (Healthy)");
    } else {
        gfx->setTextColor(COLOR_ALERT_RED, COLOR_BLACK);
        gfx->print("< 72 Hours (Imminent)");
    }
}

void UIEngine::renderAudioStethoscope(const DiagnosticMetrics& metrics, SensorManager& sensor_mgr) {
    gfx->setCursor(16, 44);
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_MAGENTA, COLOR_BLACK);
    gfx->print("AUDIO STETHOSCOPE");
    
    gfx->setCursor(16, 70);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_WHITE, COLOR_BLACK);
    gfx->printf("Real-time Acoustic Transducer: %s\n", sensor_mgr.isAudioPassthrough() ? "ACTIVE (SPEAKER ON)" : "MUTED");
    
    // Acoustic Radar Circles
    int16_t cx = LCD_WIDTH / 2;
    int16_t cy = 180;
    int16_t base_r = 30;
    
    gfx->drawCircle(cx, cy, base_r, COLOR_DARK_GRAY);
    gfx->drawCircle(cx, cy, base_r + 25, COLOR_DARK_GRAY);
    gfx->drawCircle(cx, cy, base_r + 50, COLOR_DARK_GRAY);
    
    int16_t dyn_r = base_r + (int16_t)(metrics.acoustic_db * 0.8f);
    if (dyn_r > 80) dyn_r = 80;
    gfx->drawCircle(cx, cy, dyn_r, COLOR_MAGENTA);
    gfx->fillCircle(cx, cy, 8, COLOR_MAGENTA);
    
    gfx->setCursor(cx - 30, cy + 60);
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_WHITE, COLOR_BLACK);
    gfx->printf("%.1f dB", metrics.acoustic_db);
    
    // Acoustic FFT
    gfx->setCursor(16, 270);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_LIGHT_GRAY, COLOR_BLACK);
    gfx->print("ACOUSTIC HIGH-FREQUENCY PROFILE");
    drawFFTBars(16, 285, LCD_WIDTH - 32, 90, metrics.acoustic_spectrum, metrics.acoustic_spectrum, BARS_COUNT, STATE_HEALTHY);
}

void UIEngine::renderBottomNav(const DiagnosticMetrics& metrics, DSPEngine& dsp_engine) {
    int16_t y = LCD_HEIGHT - 48;
    int16_t btn_w = (LCD_WIDTH - 32) / 4;
    
    // Button 1: CALIBRATE
    drawCyberButton(8, y, btn_w, 40, "CALIB", COLOR_CYAN, dsp_engine.isCalibrating());
    
    // Button 2: VIEW / MODE
    drawCyberButton(14 + btn_w, y, btn_w, 40, "MODE", COLOR_WHITE, false);
    
    // Button 3: DEMO SIMULATOR TRIGGER
    drawCyberButton(20 + btn_w * 2, y, btn_w, 40, "DEMO", 
                     dsp_engine.isDemoMode() ? COLOR_ALERT_ORANGE : COLOR_LIGHT_GRAY, dsp_engine.isDemoMode());
    
    // Button 4: AUDIO TOGGLE
    drawCyberButton(26 + btn_w * 3, y, btn_w, 40, "AUDIO", COLOR_MAGENTA, false);
}

void UIEngine::drawCyberButton(int16_t x, int16_t y, int16_t w, int16_t h, const char* label, uint16_t color, bool active) {
    if (active) {
        gfx->fillRoundRect(x, y, w, h, 4, color);
        gfx->setTextColor(COLOR_BLACK, color);
    } else {
        gfx->fillRoundRect(x, y, w, h, 4, COLOR_DARK_GRAY);
        gfx->drawRoundRect(x, y, w, h, 4, color);
        gfx->setTextColor(color, COLOR_DARK_GRAY);
    }
    gfx->setTextSize(1);
    gfx->setCursor(x + (w - strlen(label) * 6) / 2, y + (h - 8) / 2);
    gfx->print(label);
}

void UIEngine::drawHealthArc(int16_t cx, int16_t cy, int16_t radius, uint8_t score, DiagnosticState state) {
    uint16_t col = getStateColor(state);
    
    // Draw background track arc
    for (int r = radius - 4; r <= radius; r++) {
        gfx->drawCircle(cx, cy, r, COLOR_DARK_GRAY);
    }
    
    // Draw health arc segment based on score
    float end_angle = ((float)score / 100.0f) * 2.0f * (float)M_PI;
    for (float a = 0; a < end_angle; a += 0.08f) {
        int16_t px = cx + (int16_t)(sinf(a) * (float)radius);
        int16_t py = cy - (int16_t)(cosf(a) * (float)radius);
        gfx->fillCircle(px, py, 2, col);
    }
    
    // Heartbeat Icon in Center
    gfx->fillCircle(cx, cy, 6, col);
}

void UIEngine::drawOscilloscope(int16_t x, int16_t y, int16_t w, int16_t h, const float* waveform, size_t count, DiagnosticState state) {
    if (count < 2) return;
    
    int16_t mid_y = y + h / 2;
    
    // Zero reference line & subtle grid
    gfx->drawFastHLine(x, mid_y, w, COLOR_DARK_GRAY);
    gfx->drawFastHLine(x, mid_y - h / 4, w, 0x1082);
    gfx->drawFastHLine(x, mid_y + h / 4, w, 0x1082);
    
    uint16_t trace_color = getStateColor(state);
    
    // Draw continuous connected anti-aliased trace
    int16_t prev_x = x;
    int16_t prev_y = mid_y - (int16_t)(waveform[0] * (h / 2.5f));
    
    for (size_t i = 1; i < count && i < (size_t)w; i++) {
        int16_t cur_x = x + (i * w) / count;
        float val = waveform[i];
        if (val > 1.2f) val = 1.2f;
        if (val < -1.2f) val = -1.2f;
        int16_t cur_y = mid_y - (int16_t)(val * (h / 2.5f));
        
        gfx->drawLine(prev_x, prev_y, cur_x, cur_y, trace_color);
        // Trace glow (1 pixel thickness)
        gfx->drawLine(prev_x, prev_y + 1, cur_x, cur_y + 1, trace_color);
        
        prev_x = cur_x;
        prev_y = cur_y;
    }
}

void UIEngine::drawFFTBars(int16_t x, int16_t y, int16_t w, int16_t h, const float* spectrum, const float* peaks, int count, DiagnosticState state) {
    int16_t bar_w = (w - (count - 1) * 2) / count;
    if (bar_w < 2) bar_w = 2;
    
    uint16_t bar_color = getStateColor(state);
    
    for (int i = 0; i < count; i++) {
        int16_t bx = x + i * (bar_w + 2);
        int16_t bar_h = (int16_t)(spectrum[i] * (float)h);
        if (bar_h > h) bar_h = h;
        if (bar_h < 2) bar_h = 2;
        
        int16_t by = y + h - bar_h;
        gfx->fillRect(bx, by, bar_w, bar_h, bar_color);
        
        // Peak hold line
        int16_t peak_h = (int16_t)(peaks[i] * (float)h);
        if (peak_h > h) peak_h = h;
        int16_t peak_y = y + h - peak_h;
        gfx->drawFastHLine(bx, peak_y, bar_w, COLOR_WHITE);
    }
}

void UIEngine::handleTouch(const TouchPoint& tp, DSPEngine& dsp_engine, SensorManager& sensor_mgr) {
    if (tp.is_pressed) {
        checkButtonHits(tp.x, tp.y, dsp_engine, sensor_mgr);
    }
}

void UIEngine::checkButtonHits(int32_t x, int32_t y, DSPEngine& dsp_engine, SensorManager& sensor_mgr) {
    int16_t nav_y = LCD_HEIGHT - 48;
    int16_t btn_w = (LCD_WIDTH - 32) / 4;
    
    // Check Bottom Bar Touches
    if (y >= nav_y && y <= LCD_HEIGHT) {
        if (x >= 8 && x < 8 + btn_w) {
            // CALIBRATE BUTTON
            dsp_engine.startCalibration(60);
            sensor_mgr.playAlertTone(1200, 80);
        } else if (x >= 14 + btn_w && x < 14 + btn_w * 2) {
            // MODE BUTTON: Cycle through screens
            current_screen = (UIScreenMode)((current_screen + 1) % 4);
            need_full_redraw = true;
            sensor_mgr.playAlertTone(1600, 60);
        } else if (x >= 20 + btn_w * 2 && x < 20 + btn_w * 3) {
            // DEMO TOGGLE BUTTON
            if (!dsp_engine.isDemoMode()) {
                dsp_engine.setDemoMode(true);
                dsp_engine.setDemoFault(STATE_HEALTHY);
            } else if (dsp_engine.getDemoFault() == STATE_HEALTHY) {
                dsp_engine.setDemoFault(STATE_CRITICAL_UNBALANCE);
            } else {
                dsp_engine.setDemoMode(false);
            }
            sensor_mgr.playAlertTone(2000, 80);
        } else if (x >= 26 + btn_w * 3 && x < LCD_WIDTH) {
            // AUDIO STETHOSCOPE TOGGLE
            bool state = sensor_mgr.isAudioPassthrough();
            sensor_mgr.setAudioPassthrough(!state);
            sensor_mgr.playAlertTone(1000, 100);
        }
    }
}
