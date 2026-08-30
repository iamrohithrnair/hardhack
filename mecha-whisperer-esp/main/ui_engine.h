#pragma once
#include "dsp_engine.h"
#include "lvgl.h"

void ui_engine_init(void);
void ui_engine_update(const DiagnosticMetrics* metrics, const float* osc_waveform, size_t osc_count);
