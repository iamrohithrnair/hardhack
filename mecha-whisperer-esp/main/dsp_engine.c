#include "dsp_engine.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static DiagnosticMetrics s_metrics;
static BaselineProfile s_baseline;

static float s_fft_real[FFT_SIZE];
static float s_fft_imag[FFT_SIZE];
static float s_hanning[FFT_SIZE];
static float s_fft_mag[FFT_HALF_SIZE];

static bool s_calibrating = false;
static uint16_t s_cal_counter = 0;
static uint16_t s_cal_target = 60;
static float s_cal_accum_rms = 0.0f;
static float s_cal_accum_freq = 0.0f;
static float s_cal_accum_kurt = 0.0f;

static bool s_demo_mode = false;
static DiagnosticState s_demo_fault = STATE_HEALTHY;
static float s_demo_phase = 0.0f;
static float s_demo_harmonic_phase = 0.0f;

static void compute_fft(float* real, float* imag, size_t n) {
    size_t j = 0;
    for (size_t i = 0; i < n - 1; i++) {
        if (i < j) {
            float temp_r = real[i];
            real[i] = real[j];
            real[j] = temp_r;
            float temp_i = imag[i];
            imag[i] = imag[j];
            imag[j] = temp_i;
        }
        size_t k = n >> 1;
        while (k <= j) {
            j -= k;
            k >>= 1;
        }
        j += k;
    }
    
    for (size_t len = 2; len <= n; len <<= 1) {
        float angle = -2.0f * (float)M_PI / (float)len;
        float wlen_r = cosf(angle);
        float wlen_i = sinf(angle);
        
        for (size_t i = 0; i < n; i += len) {
            float w_r = 1.0f;
            float w_i = 0.0f;
            for (size_t k = 0; k < len / 2; k++) {
                size_t u = i + k;
                size_t v = i + k + len / 2;
                float tr = w_r * real[v] - w_i * imag[v];
                float ti = w_r * imag[v] + w_i * real[v];
                real[v] = real[u] - tr;
                imag[v] = imag[u] - ti;
                real[u] = real[u] + tr;
                imag[u] = imag[u] + ti;
                float next_w_r = w_r * wlen_r - w_i * wlen_i;
                float next_w_i = w_r * wlen_i + w_i * wlen_r;
                w_r = next_w_r;
                w_i = next_w_i;
            }
        }
    }
}

void dsp_engine_init(void) {
    memset(&s_metrics, 0, sizeof(s_metrics));
    memset(&s_baseline, 0, sizeof(s_baseline));
    
    for (size_t i = 0; i < FFT_SIZE; i++) {
        s_hanning[i] = 0.5f * (1.0f - cosf((2.0f * (float)M_PI * (float)i) / (float)(FFT_SIZE - 1)));
    }
    
    for (int i = 0; i < BARS_COUNT; i++) {
        s_metrics.visual_spectrum[i] = 0.05f;
        s_metrics.visual_spectrum_peak[i] = 0.05f;
    }
    
    s_baseline.is_calibrated = false;
    s_baseline.base_rms_g = 0.08f;
    s_baseline.base_peak_freq_hz = 48.0f;
    s_baseline.base_kurtosis = 3.0f;
    
    s_metrics.health_score = 98;
    s_metrics.anomaly_index = 2.0f;
    s_metrics.state = STATE_HEALTHY;
    s_metrics.diagnosis_text = "NOMINAL HARMONIC";
    s_metrics.recommendation = "Optimal baseline. ISO 10816 Class A.";
}

void dsp_engine_process_vibration(const float* raw_samples, size_t count, float sample_rate_hz) {
    if (count < FFT_SIZE) return;
    
    float sum = 0.0f;
    float min_val = raw_samples[0];
    float max_val = raw_samples[0];
    
    for (size_t i = 0; i < FFT_SIZE; i++) {
        sum += raw_samples[i];
        if (raw_samples[i] < min_val) min_val = raw_samples[i];
        if (raw_samples[i] > max_val) max_val = raw_samples[i];
    }
    float mean = sum / (float)FFT_SIZE;
    s_metrics.peak_to_peak_g = max_val - min_val;
    
    float sum_sq = 0.0f;
    float sum_quad = 0.0f;
    float peak_abs = 0.0f;
    
    for (size_t i = 0; i < FFT_SIZE; i++) {
        float dev = raw_samples[i] - mean;
        float dev_sq = dev * dev;
        sum_sq += dev_sq;
        sum_quad += dev_sq * dev_sq;
        float abs_v = fabsf(dev);
        if (abs_v > peak_abs) peak_abs = abs_v;
    }
    
    float variance = sum_sq / (float)FFT_SIZE;
    s_metrics.rms_acceleration_g = sqrtf(variance);
    
    if (variance > 0.00001f) {
        s_metrics.kurtosis = (sum_quad / (float)FFT_SIZE) / (variance * variance);
        s_metrics.crest_factor = peak_abs / s_metrics.rms_acceleration_g;
    } else {
        s_metrics.kurtosis = 3.0f;
        s_metrics.crest_factor = 1.414f;
    }
    
    for (size_t i = 0; i < FFT_SIZE; i++) {
        s_fft_real[i] = (raw_samples[i] - mean) * s_hanning[i];
        s_fft_imag[i] = 0.0f;
    }
    
    compute_fft(s_fft_real, s_fft_imag, FFT_SIZE);
    
    float scale = 2.0f / (float)FFT_SIZE;
    for (size_t i = 0; i < FFT_HALF_SIZE; i++) {
        s_fft_mag[i] = sqrtf(s_fft_real[i] * s_fft_real[i] + s_fft_imag[i] * s_fft_imag[i]) * scale;
    }
    
    float bin_resolution = sample_rate_hz / (float)FFT_SIZE;
    float max_mag = 0.0f;
    size_t peak_bin = 2;
    float total_energy = 0.0f;
    
    for (size_t i = 2; i < FFT_HALF_SIZE; i++) {
        total_energy += s_fft_mag[i];
        if (s_fft_mag[i] > max_mag) {
            max_mag = s_fft_mag[i];
            peak_bin = i;
        }
    }
    
    float delta_freq = 0.0f;
    if (peak_bin > 2 && peak_bin < FFT_HALF_SIZE - 1) {
        float alpha = s_fft_mag[peak_bin - 1];
        float beta  = s_fft_mag[peak_bin];
        float gamma = s_fft_mag[peak_bin + 1];
        float denom = (alpha - 2.0f * beta + gamma);
        if (fabsf(denom) > 0.0001f) {
            delta_freq = 0.5f * (alpha - gamma) / denom;
        }
    }
    
    if (max_mag > 0.015f) {
        s_metrics.peak_freq_hz = ((float)peak_bin + delta_freq) * bin_resolution;
        s_metrics.estimated_rpm = (uint32_t)(s_metrics.peak_freq_hz * 60.0f);
    } else {
        s_metrics.peak_freq_hz = 30.0f;
        s_metrics.estimated_rpm = 1800;
    }
    
    if (s_metrics.peak_freq_hz > 5.0f && s_metrics.rms_acceleration_g > 0.02f) {
        s_metrics.iso_vibration_vel = (s_metrics.rms_acceleration_g * 9806.65f) / 
                                      (2.0f * (float)M_PI * s_metrics.peak_freq_hz);
    } else {
        s_metrics.iso_vibration_vel = 0.12f;
    }
    
    // Visual Bars Mapping
    for (int b = 0; b < BARS_COUNT; b++) {
        size_t start_idx = 2 + (b * (FFT_HALF_SIZE - 4)) / BARS_COUNT;
        size_t end_idx   = 2 + ((b + 1) * (FFT_HALF_SIZE - 4)) / BARS_COUNT;
        if (end_idx <= start_idx) end_idx = start_idx + 1;
        if (end_idx > FFT_HALF_SIZE) end_idx = FFT_HALF_SIZE;
        
        float b_max = 0.0f;
        for (size_t i = start_idx; i < end_idx; i++) {
            if (s_fft_mag[i] > b_max) b_max = s_fft_mag[i];
        }
        
        float norm = b_max * 3.5f;
        if (norm > 1.0f) norm = 1.0f;
        
        s_metrics.visual_spectrum[b] = 0.6f * s_metrics.visual_spectrum[b] + 0.4f * norm;
        if (s_metrics.visual_spectrum[b] > s_metrics.visual_spectrum_peak[b]) {
            s_metrics.visual_spectrum_peak[b] = s_metrics.visual_spectrum[b];
        } else {
            s_metrics.visual_spectrum_peak[b] -= 0.08f;
            if (s_metrics.visual_spectrum_peak[b] < 0.0f) s_metrics.visual_spectrum_peak[b] = 0.0f;
        }
    }
    
    // Anomaly evaluation
    float ref_rms = s_baseline.is_calibrated ? s_baseline.base_rms_g : 0.15f;
    if (ref_rms < 0.02f) ref_rms = 0.02f;
    float rms_ratio = s_metrics.rms_acceleration_g / ref_rms;
    
    float anomaly = 0.0f;
    if (s_metrics.rms_acceleration_g > 0.05f) {
        if (rms_ratio > 1.2f) anomaly += (rms_ratio - 1.2f) * 22.0f;
        if (s_metrics.kurtosis > 4.2f) anomaly += (s_metrics.kurtosis - 4.2f) * 15.0f;
        if (s_metrics.iso_vibration_vel > 1.12f) anomaly += (s_metrics.iso_vibration_vel - 1.12f) * 12.0f;
    }
    
    if (anomaly < 0.0f) anomaly = 0.0f;
    if (anomaly > 100.0f) anomaly = 100.0f;
    s_metrics.anomaly_index = anomaly;
    s_metrics.health_score = (uint8_t)(100.0f - anomaly);
    
    if (anomaly >= 60.0f || rms_ratio > 2.8f) {
        if (s_metrics.kurtosis > 5.5f) {
            s_metrics.state = STATE_BEARING_DAMAGE;
            s_metrics.diagnosis_text = "BEARING DAMAGE (IMPACTS)";
            s_metrics.recommendation = "Severe spalling. Replace bearing.";
        } else {
            s_metrics.state = STATE_CRITICAL_UNBALANCE;
            s_metrics.diagnosis_text = "CRITICAL ROTOR UNBALANCE (1X)";
            s_metrics.recommendation = "Mass eccentricity. Rebalance rotor.";
        }
    } else if (anomaly >= 30.0f || rms_ratio > 1.8f) {
        s_metrics.state = STATE_WARNING;
        s_metrics.diagnosis_text = "ELEVATED VIBRATION ANOMALY";
        s_metrics.recommendation = "Early wear detected. Monitor trend.";
    } else {
        s_metrics.state = STATE_HEALTHY;
        s_metrics.diagnosis_text = "HEALTHY / NOMINAL HARMONIC";
        s_metrics.recommendation = "Machine is running within ISO Class A.";
    }
}

void dsp_engine_start_calibration(uint16_t samples_to_gather) {
    s_calibrating = true;
    s_cal_counter = 0;
    s_cal_target = samples_to_gather;
    s_cal_accum_rms = 0.0f;
    s_cal_accum_freq = 0.0f;
    s_cal_accum_kurt = 0.0f;
}

bool dsp_engine_is_calibrating(void) {
    return s_calibrating;
}

const DiagnosticMetrics* dsp_engine_get_metrics(void) {
    return &s_metrics;
}

void dsp_engine_set_demo_mode(bool enabled) {
    s_demo_mode = enabled;
}

bool dsp_engine_is_demo_mode(void) {
    return s_demo_mode;
}

void dsp_engine_set_demo_fault(DiagnosticState fault_type) {
    s_demo_fault = fault_type;
}

DiagnosticState dsp_engine_get_demo_fault(void) {
    return s_demo_fault;
}

void dsp_engine_generate_demo_samples(float* buffer, size_t count, float sample_rate_hz) {
    float base_freq = 48.5f;
    float dt = 1.0f / sample_rate_hz;
    
    for (size_t i = 0; i < count; i++) {
        s_demo_phase += 2.0f * (float)M_PI * base_freq * dt;
        if (s_demo_phase >= 2.0f * (float)M_PI) s_demo_phase -= 2.0f * (float)M_PI;
        
        s_demo_harmonic_phase += 2.0f * (float)M_PI * (base_freq * 2.0f) * dt;
        if (s_demo_harmonic_phase >= 2.0f * (float)M_PI) s_demo_harmonic_phase -= 2.0f * (float)M_PI;
        
        float noise = ((float)(rand() % 1000) / 500.0f - 1.0f) * 0.02f;
        
        if (s_demo_fault == STATE_HEALTHY) {
            buffer[i] = 0.08f * sinf(s_demo_phase) + noise;
        } else if (s_demo_fault == STATE_CRITICAL_UNBALANCE) {
            float modulation = 1.0f + 0.15f * sinf(s_demo_phase * 0.2f);
            buffer[i] = 0.95f * sinf(s_demo_phase) * modulation + 0.25f * sinf(s_demo_harmonic_phase) + noise * 3.0f;
        } else if (s_demo_fault == STATE_BEARING_DAMAGE) {
            float impact = 0.0f;
            if ((rand() % 40) == 0) {
                impact = ((rand() % 2 == 0) ? 1.0f : -1.0f) * 0.9f;
            }
            buffer[i] = 0.18f * sinf(s_demo_phase) + impact + noise * 4.0f;
        } else {
            buffer[i] = 0.45f * sinf(s_demo_phase * 0.5f) + 0.35f * sinf(s_demo_phase) + noise * 2.5f;
        }
    }
}
