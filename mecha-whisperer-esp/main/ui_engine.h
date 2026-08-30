#pragma once
#include "dsp_engine.h"
#include "lvgl.h"

typedef enum {
    VIEW_SINE_WAVE_TRANSDUCER = 0, // View 1: Multi-chromatic glowing sine waves (Hero Default)
    VIEW_FFT_SPECTRUM,             // View 2: 24-Band FFT Spectral Equalizer & Harmonics
    VIEW_KURTOSIS_IMPACT,          // View 3: Kurtosis & Impulse Shock History
    VIEW_TRIPLE_ACTIVITY_RINGS,    // View 4: Concentric Activity Rings & Health Score
    VIEW_PROGRESS_ARC,             // View 5: High-Precision Progress Arc & Severity
    VIEW_ROTATION_TACHOMETER,      // View 6: Rotational Tachometer Speedometer Gauge
    VIEW_FLUID_ENERGY_TANK,        // View 7: Fluid Energy Density Wave Level
    VIEW_HARMONIC_GOAL_PILLARS,    // View 8: Harmonic Energy Goal Pillars (1X-7X)
    VIEW_DOT_MATRIX_HEATMAP,       // View 9: Spatio-Temporal Dot Matrix Heatmap
    VIEW_BOLD_DIAGNOSTICS,         // View 10: Bold Diagnosis & ISO 10816 Evaluation
    VIEW_MAX_COUNT
} ui_view_mode_t;

void ui_engine_init(void);
void ui_engine_update(const DiagnosticMetrics* metrics, const float* osc_waveform, size_t osc_count);
void ui_engine_next_view(void);
void ui_engine_set_view(ui_view_mode_t view);
ui_view_mode_t ui_engine_get_view(void);
