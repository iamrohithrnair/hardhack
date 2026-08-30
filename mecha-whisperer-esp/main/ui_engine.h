#pragma once
#include "dsp_engine.h"
#include "lvgl.h"

typedef enum {
    VIEW_SINE_WAVE_TRANSDUCER = 0, // View 1: Multi-chromatic glowing sine waves (Hero Default)
    VIEW_FFT_SPECTRUM,             // View 2: 24-Band FFT Spectral Equalizer & Harmonics
    VIEW_KURTOSIS_IMPACT,          // View 3: Kurtosis & Impulse Shock History
    VIEW_TRIPLE_ACTIVITY_RINGS,    // View 4: Concentric Activity Rings & Health Score
    VIEW_BOLD_DIAGNOSTICS,         // View 5: Bold Diagnosis & ISO 10816 Evaluation
    VIEW_MAX_COUNT
} ui_view_mode_t;

void ui_engine_init(void);
void ui_engine_update(const DiagnosticMetrics* metrics, const float* osc_waveform, size_t osc_count);
void ui_engine_next_view(void);
void ui_engine_set_view(ui_view_mode_t view);
ui_view_mode_t ui_engine_get_view(void);
