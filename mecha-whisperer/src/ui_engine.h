#pragma once
#include <Arduino.h>
#include "hardware_config.h"
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "dsp_engine.h"
#include "sensor_manager.h"

// UI View Modes
enum UIScreenMode {
    UI_MODE_MAIN_HUD = 0,       // Live Stethoscope (Oscilloscope + FFT + Health Score)
    UI_MODE_SPECTRUM_ZOOM,      // Expanded High-Res 32-Band FFT Analyzer
    UI_MODE_ISO_REPORT,         // Medical Machine Doctor & ISO 10816 Report
    UI_MODE_AUDIO_STETHOSCOPE   // Live Acoustic Listening & Audio Radar
};

struct TouchPoint {
    int32_t x;
    int32_t y;
    bool is_pressed;
};

class UIEngine {
public:
    UIEngine();
    bool init();
    
    // Core Frame Rendering Routine (Runs at 30-60 FPS)
    void render(const DiagnosticMetrics& metrics, const float* osc_waveform, size_t osc_count, 
                SensorManager& sensor_mgr, DSPEngine& dsp_engine);
    
    // Touchscreen Event Handling
    void handleTouch(const TouchPoint& tp, DSPEngine& dsp_engine, SensorManager& sensor_mgr);
    
    // Screen Navigation
    void setScreenMode(UIScreenMode mode) { current_screen = mode; need_full_redraw = true; }
    UIScreenMode getScreenMode() const { return current_screen; }
    
    // Brightness Control
    void setBrightness(uint8_t brightness);

private:
    Arduino_DataBus *bus;
    Arduino_GFX *gfx;
    std::shared_ptr<Arduino_IIC_DriveBus> iic_bus;
    std::unique_ptr<Arduino_IIC> touch_v1; // FT3168
    
    UIScreenMode current_screen;
    bool need_full_redraw;
    uint32_t frame_count;
    uint32_t last_fps_time;
    uint16_t current_fps;
    
    // Touch Hitboxes
    void checkButtonHits(int32_t x, int32_t y, DSPEngine& dsp_engine, SensorManager& sensor_mgr);
    
    // Screen Renderers
    void renderTopBar(const DiagnosticMetrics& metrics, SensorManager& sensor_mgr);
    void renderMainHUD(const DiagnosticMetrics& metrics, const float* osc_waveform, size_t osc_count);
    void renderSpectrumZoom(const DiagnosticMetrics& metrics);
    void renderISOReport(const DiagnosticMetrics& metrics, SensorManager& sensor_mgr);
    void renderAudioStethoscope(const DiagnosticMetrics& metrics, SensorManager& sensor_mgr);
    void renderBottomNav(const DiagnosticMetrics& metrics, DSPEngine& dsp_engine);
    
    // Custom High-Performance HUD Drawing Primitives
    void drawHealthArc(int16_t cx, int16_t cy, int16_t radius, uint8_t score, DiagnosticState state);
    void drawOscilloscope(int16_t x, int16_t y, int16_t w, int16_t h, const float* waveform, size_t count, DiagnosticState state);
    void drawFFTBars(int16_t x, int16_t y, int16_t w, int16_t h, const float* spectrum, const float* peaks, int count, DiagnosticState state);
    void drawCyberButton(int16_t x, int16_t y, int16_t w, int16_t h, const char* label, uint16_t color, bool active);
    void drawBadge(int16_t x, int16_t y, const char* text, uint16_t bg_color, uint16_t fg_color);
    
    uint16_t getStateColor(DiagnosticState state);
    uint16_t getStateBgColor(DiagnosticState state);
};
