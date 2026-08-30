#pragma once
#include "dsp_engine.h"
#include "lvgl.h"

typedef enum {
    VIEW_ACOUSTIC_ECG_VU = 0,    // View 1: Mic / Acoustic Pulse (ECG pulse + Segmented VU Meter)
    VIEW_VIBRATION_TRANSDUCER,   // View 2: IMU Vibration (Dual Spline + Harmonic Goal Bars + Fluid Tank)
    VIEW_GYRO_TACHOMETER,        // View 3: Gyroscope Dynamics (Tachometer Arc Gauge + Angular Rate)
    VIEW_TRIPLE_RINGS_MATRIX,    // View 4: Multi-Sensor Health (Triple Activity Rings + Dot Matrix Heatmap)
    VIEW_BOLD_DIAGNOSTICS,       // View 5: Bold Typography & Diagnosis (Progress Arc + Giant Neon Readouts)
    VIEW_MAX_COUNT
} ui_view_mode_t;

void ui_engine_init(void);
void ui_engine_update(const DiagnosticMetrics* metrics, const float* osc_waveform, size_t osc_count);
void ui_engine_next_view(void);
void ui_engine_set_view(ui_view_mode_t view);
ui_view_mode_t ui_engine_get_view(void);
