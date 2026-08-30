#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FFT_SIZE            256
#define FFT_HALF_SIZE       128
#define BARS_COUNT          24

typedef enum {
    STATE_CALIBRATING = 0,
    STATE_HEALTHY,
    STATE_WARNING,
    STATE_CRITICAL_UNBALANCE,
    STATE_BEARING_DAMAGE,
    STATE_LOOSE_MOUNT
} DiagnosticState;

typedef struct {
    float peak_freq_hz;
    uint32_t estimated_rpm;
    float rms_acceleration_g;
    float peak_to_peak_g;
    float iso_vibration_vel;
    float kurtosis;
    float crest_factor;
    float acoustic_db;
    float harmonic_distortion;
    float anomaly_index;
    uint8_t health_score;
    DiagnosticState state;
    const char* diagnosis_text;
    const char* recommendation;
    float visual_spectrum[BARS_COUNT];
    float visual_spectrum_peak[BARS_COUNT];
} DiagnosticMetrics;

typedef struct {
    bool is_calibrated;
    float base_rms_g;
    float base_peak_freq_hz;
    float base_kurtosis;
    float base_acoustic_db;
    float base_spectrum[BARS_COUNT];
} BaselineProfile;

void dsp_engine_init(void);
void dsp_engine_process_vibration(const float* raw_samples, size_t count, float sample_rate_hz);
void dsp_engine_start_calibration(uint16_t samples_to_gather);
bool dsp_engine_is_calibrating(void);
const DiagnosticMetrics* dsp_engine_get_metrics(void);
void dsp_engine_set_demo_mode(bool enabled);
bool dsp_engine_is_demo_mode(void);
void dsp_engine_set_demo_fault(DiagnosticState fault_type);
DiagnosticState dsp_engine_get_demo_fault(void);
void dsp_engine_generate_demo_samples(float* buffer, size_t count, float sample_rate_hz);
