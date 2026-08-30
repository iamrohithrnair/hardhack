#include "dsp_engine.h"
#include <string.h>

DSPEngine::DSPEngine() {
    calibration_active = false;
    calibration_counter = 0;
    calibration_target = 60;
    cal_accum_rms = 0.0f;
    cal_accum_freq = 0.0f;
    cal_accum_kurtosis = 0.0f;
    cal_accum_db = 0.0f;
    
    demo_mode = false;
    demo_fault = STATE_HEALTHY;
    demo_phase = 0.0f;
    demo_harmonic_phase = 0.0f;
    
    memset(&current_metrics, 0, sizeof(current_metrics));
    memset(&baseline, 0, sizeof(baseline));
    
    // Default baseline for initial power-on
    baseline.is_calibrated = false;
    baseline.base_rms_g = 0.08f;
    baseline.base_peak_freq_hz = 48.0f;
    baseline.base_kurtosis = 3.0f;
    baseline.base_acoustic_db = 45.0f;
    
    current_metrics.health_score = 98;
    current_metrics.anomaly_index = 2.0f;
    current_metrics.state = STATE_HEALTHY;
    current_metrics.diagnosis_text = "NOMINAL HARMONIC";
    current_metrics.recommendation = "Optimal baseline. No action required.";
}

void DSPEngine::init() {
    // Precompute Hanning Window coefficients
    for (size_t i = 0; i < FFT_SIZE; i++) {
        hanning_window[i] = 0.5f * (1.0f - cosf((2.0f * (float)M_PI * (float)i) / (float)(FFT_SIZE - 1)));
    }
    
    for (int i = 0; i < BARS_COUNT; i++) {
        current_metrics.visual_spectrum[i] = 0.05f;
        current_metrics.visual_spectrum_peak[i] = 0.05f;
        current_metrics.acoustic_spectrum[i] = 0.05f;
    }
}

void DSPEngine::applyHanningWindow(float* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        data[i] *= hanning_window[i];
    }
}

// In-Place Radix-2 Decimation-in-Time Fast Fourier Transform
void DSPEngine::computeFFT(float* real, float* imag, size_t n) {
    // Bit-reversal permutation
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
    
    // Danielson-Lanczos algorithm
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

void DSPEngine::computeMagnitudes(const float* real, const float* imag, float* mag, size_t half_n) {
    // Scaling factor (normalize FFT energy)
    float scale = 2.0f / (float)FFT_SIZE;
    mag[0] = fabsf(real[0]) / (float)FFT_SIZE; // DC component
    
    for (size_t i = 1; i < half_n; i++) {
        mag[i] = sqrtf(real[i] * real[i] + imag[i] * imag[i]) * scale;
    }
}

void DSPEngine::processVibration(const float* raw_samples, size_t count, float sample_rate_hz) {
    if (count < FFT_SIZE) return;
    
    // 1. Calculate Time-Domain Statistical Features (Mean, RMS, Peak-to-Peak, Kurtosis, Crest Factor)
    float sum = 0.0f;
    float min_val = raw_samples[0];
    float max_val = raw_samples[0];
    
    for (size_t i = 0; i < FFT_SIZE; i++) {
        sum += raw_samples[i];
        if (raw_samples[i] < min_val) min_val = raw_samples[i];
        if (raw_samples[i] > max_val) max_val = raw_samples[i];
    }
    float mean = sum / (float)FFT_SIZE;
    current_metrics.peak_to_peak_g = max_val - min_val;
    
    // 2. High-Order Moments (Variance & 4th Moment for Kurtosis)
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
    current_metrics.rms_acceleration_g = sqrtf(variance);
    
    // Kurtosis: (4th moment) / (variance^2). Normal Gaussian noise = ~3.0
    if (variance > 0.00001f) {
        current_metrics.kurtosis = (sum_quad / (float)FFT_SIZE) / (variance * variance);
        current_metrics.crest_factor = peak_abs / current_metrics.rms_acceleration_g;
    } else {
        current_metrics.kurtosis = 3.0f;
        current_metrics.crest_factor = 1.414f;
    }
    
    // 3. Prepare FFT Input Buffers & Apply Windowing
    for (size_t i = 0; i < FFT_SIZE; i++) {
        fft_real[i] = (raw_samples[i] - mean); // Remove DC offset
        fft_imag[i] = 0.0f;
    }
    applyHanningWindow(fft_real, FFT_SIZE);
    
    // 4. Execute Radix-2 FFT
    computeFFT(fft_real, fft_imag, FFT_SIZE);
    computeMagnitudes(fft_real, fft_imag, fft_mag, FFT_HALF_SIZE);
    
    // 5. Extract Spectral Features & Fundamental RPM
    extractSpectralFeatures(sample_rate_hz);
    
    // 6. Map to UI Visualizer Bars with Smooth Peak Hold Decay
    mapToVisualBins();
    
    // 7. Execute Multi-Metric Anomaly Diagnosis Engine
    if (calibration_active) {
        updateCalibration();
    } else {
        evaluateDiagnosticState();
    }
}

void DSPEngine::extractSpectralFeatures(float sample_rate_hz) {
    float bin_resolution = sample_rate_hz / (float)FFT_SIZE;
    
    // Find dominant fundamental peak (ignore lowest 2 DC bins)
    float max_mag = 0.0f;
    size_t peak_bin = 2;
    float total_spectral_energy = 0.0f;
    
    for (size_t i = 2; i < FFT_HALF_SIZE; i++) {
        total_spectral_energy += fft_mag[i];
        if (fft_mag[i] > max_mag) {
            max_mag = fft_mag[i];
            peak_bin = i;
        }
    }
    
    // Parabolic Interpolation for Sub-Bin Frequency Accuracy
    float delta_freq = 0.0f;
    if (peak_bin > 2 && peak_bin < FFT_HALF_SIZE - 1) {
        float alpha = fft_mag[peak_bin - 1];
        float beta  = fft_mag[peak_bin];
        float gamma = fft_mag[peak_bin + 1];
        float denom = (alpha - 2.0f * beta + gamma);
        if (fabsf(denom) > 0.0001f) {
            delta_freq = 0.5f * (alpha - gamma) / denom;
        }
    }
    
    current_metrics.peak_freq_hz = ((float)peak_bin + delta_freq) * bin_resolution;
    current_metrics.estimated_rpm = (uint32_t)(current_metrics.peak_freq_hz * 60.0f);
    
    // ISO 10816 Vibration Severity Velocity (mm/s RMS)
    // v = a / (2 * pi * f) * 9806.65 mm/s^2 per g
    if (current_metrics.peak_freq_hz > 5.0f) {
        current_metrics.iso_vibration_vel = (current_metrics.rms_acceleration_g * 9806.65f) / 
                                            (2.0f * (float)M_PI * current_metrics.peak_freq_hz);
    } else {
        current_metrics.iso_vibration_vel = current_metrics.rms_acceleration_g * 10.0f;
    }
    
    // Total Harmonic Distortion of Vibration (Harmonic energy vs fundamental)
    float fundamental_energy = max_mag;
    float harmonic_energy = (total_spectral_energy > fundamental_energy) ? 
                            (total_spectral_energy - fundamental_energy) : 0.0f;
    current_metrics.harmonic_distortion = (fundamental_energy > 0.01f) ? 
                                          (harmonic_energy / fundamental_energy) : 0.0f;
}

void DSPEngine::processAudio(const int16_t* audio_samples, size_t count) {
    if (count < ACOUSTIC_FFT_SIZE) return;
    
    // Compute RMS acoustic energy
    float sum_sq = 0.0f;
    for (size_t i = 0; i < ACOUSTIC_FFT_SIZE; i++) {
        float s = (float)audio_samples[i] / 32768.0f;
        sum_sq += s * s;
        audio_real[i] = s;
        audio_imag[i] = 0.0f;
    }
    
    float audio_rms = sqrtf(sum_sq / (float)ACOUSTIC_FFT_SIZE);
    
    // Convert to relative dB SPL (calibration offset: ~94dB SPL at full scale)
    float db = 20.0f * log10f(fmaxf(audio_rms, 0.00001f)) + 94.0f;
    current_metrics.acoustic_db = 0.8f * current_metrics.acoustic_db + 0.2f * db; // Smooth filter
    
    // Compute Audio FFT for acoustic harmonic visualizer
    computeFFT(audio_real, audio_imag, ACOUSTIC_FFT_SIZE);
    computeMagnitudes(audio_real, audio_imag, audio_mag, ACOUSTIC_FFT_SIZE / 2);
    
    // Map acoustic energy to visualizer
    for (int b = 0; b < BARS_COUNT; b++) {
        size_t idx = (b * (ACOUSTIC_FFT_SIZE / 2)) / BARS_COUNT;
        float val = audio_mag[idx] * 4.0f;
        if (val > 1.0f) val = 1.0f;
        current_metrics.acoustic_spectrum[b] = 0.7f * current_metrics.acoustic_spectrum[b] + 0.3f * val;
    }
}

void DSPEngine::mapToVisualBins() {
    // Map 128 FFT bins logarithmically / linearly into BARS_COUNT visual bars
    float decay_rate = 0.08f;
    
    for (int b = 0; b < BARS_COUNT; b++) {
        size_t start_idx = 2 + (b * (FFT_HALF_SIZE - 4)) / BARS_COUNT;
        size_t end_idx   = 2 + ((b + 1) * (FFT_HALF_SIZE - 4)) / BARS_COUNT;
        if (end_idx <= start_idx) end_idx = start_idx + 1;
        if (end_idx > FFT_HALF_SIZE) end_idx = FFT_HALF_SIZE;
        
        float bin_max = 0.0f;
        for (size_t i = start_idx; i < end_idx; i++) {
            if (fft_mag[i] > bin_max) bin_max = fft_mag[i];
        }
        
        // Auto-gain normalized for display (0.0 to 1.0)
        float normalized = bin_max * 3.5f;
        if (normalized > 1.0f) normalized = 1.0f;
        
        // Exponential smoothing
        current_metrics.visual_spectrum[b] = 0.6f * current_metrics.visual_spectrum[b] + 0.4f * normalized;
        
        // Peak hold animation
        if (current_metrics.visual_spectrum[b] > current_metrics.visual_spectrum_peak[b]) {
            current_metrics.visual_spectrum_peak[b] = current_metrics.visual_spectrum[b];
        } else {
            current_metrics.visual_spectrum_peak[b] -= decay_rate;
            if (current_metrics.visual_spectrum_peak[b] < 0.0f) {
                current_metrics.visual_spectrum_peak[b] = 0.0f;
            }
        }
    }
}

void DSPEngine::startCalibration(uint16_t samples_to_gather) {
    calibration_active = true;
    calibration_counter = 0;
    calibration_target = samples_to_gather;
    cal_accum_rms = 0.0f;
    cal_accum_freq = 0.0f;
    cal_accum_kurtosis = 0.0f;
    cal_accum_db = 0.0f;
    for (int i = 0; i < BARS_COUNT; i++) {
        baseline.base_spectrum[i] = 0.0f;
    }
    current_metrics.state = STATE_CALIBRATING;
    current_metrics.diagnosis_text = "CALIBRATING BASELINE...";
    current_metrics.recommendation = "Hold stethoscope firmly against machine.";
}

void DSPEngine::updateCalibration() {
    cal_accum_rms += current_metrics.rms_acceleration_g;
    cal_accum_freq += current_metrics.peak_freq_hz;
    cal_accum_kurtosis += current_metrics.kurtosis;
    cal_accum_db += current_metrics.acoustic_db;
    
    for (int i = 0; i < BARS_COUNT; i++) {
        baseline.base_spectrum[i] += current_metrics.visual_spectrum[i];
    }
    
    calibration_counter++;
    if (calibration_counter >= calibration_target) {
        float n = (float)calibration_counter;
        baseline.base_rms_g = cal_accum_rms / n;
        baseline.base_peak_freq_hz = cal_accum_freq / n;
        baseline.base_kurtosis = cal_accum_kurtosis / n;
        baseline.base_acoustic_db = cal_accum_db / n;
        for (int i = 0; i < BARS_COUNT; i++) {
            baseline.base_spectrum[i] /= n;
        }
        baseline.is_calibrated = true;
        calibration_active = false;
    }
}

void DSPEngine::evaluateDiagnosticState() {
    float ref_rms = baseline.is_calibrated ? baseline.base_rms_g : 0.08f;
    if (ref_rms < 0.01f) ref_rms = 0.01f;
    
    float rms_ratio = current_metrics.rms_acceleration_g / ref_rms;
    float kurt = current_metrics.kurtosis;
    float iso_v = current_metrics.iso_vibration_vel;
    
    // Anomaly index computation (multi-metric composite score)
    float anomaly = 0.0f;
    
    // 1. RMS Vibration Elevation (Dominant weight for unbalance)
    if (rms_ratio > 1.2f) {
        anomaly += (rms_ratio - 1.2f) * 22.0f;
    }
    
    // 2. Kurtosis Penalty (Impulsive ball/race damage impacts)
    if (kurt > 3.8f) {
        anomaly += (kurt - 3.8f) * 15.0f;
    }
    
    // 3. ISO 10816 Vibration Severity Penalty
    if (iso_v > 1.12f) { // ISO Class B/C/D
        anomaly += (iso_v - 1.12f) * 12.0f;
    }
    
    // Clamp anomaly 0 to 100%
    if (anomaly < 0.0f) anomaly = 0.0f;
    if (anomaly > 100.0f) anomaly = 100.0f;
    
    current_metrics.anomaly_index = anomaly;
    current_metrics.health_score = (uint8_t)(100.0f - anomaly);
    if (current_metrics.health_score > 100) current_metrics.health_score = 100;
    
    // State Classification Decision Tree
    if (anomaly >= 60.0f || rms_ratio > 2.8f) {
        // High severity failure
        if (kurt > 5.5f) {
            current_metrics.state = STATE_BEARING_DAMAGE;
            current_metrics.diagnosis_text = "BEARING RACE DAMAGE (IMPACTS)";
            current_metrics.recommendation = "Severe pitting detected. Replace bearing assembly.";
        } else {
            current_metrics.state = STATE_CRITICAL_UNBALANCE;
            current_metrics.diagnosis_text = "CRITICAL ROTOR UNBALANCE (1X)";
            current_metrics.recommendation = "Mass eccentricity detected. Rebalance fan blades.";
        }
    } else if (anomaly >= 30.0f || rms_ratio > 1.8f) {
        if (current_metrics.harmonic_distortion > 1.5f) {
            current_metrics.state = STATE_LOOSE_MOUNT;
            current_metrics.diagnosis_text = "MECHANICAL LOOSENESS / RESONANCE";
            current_metrics.recommendation = "Check chassis mounting bolts and belt tension.";
        } else {
            current_metrics.state = STATE_WARNING;
            current_metrics.diagnosis_text = "ELEVATED VIBRATION ANOMALY";
            current_metrics.recommendation = "Early wear detected. Monitor trend closely.";
        }
    } else {
        current_metrics.state = STATE_HEALTHY;
        current_metrics.diagnosis_text = "HEALTHY / NOMINAL HARMONIC";
        current_metrics.recommendation = "Machine is running within ISO 10816 Class A tolerance.";
    }
}

// Synthetic Demo Generator for Fail-safe 1-Minute Live Pitch
void DSPEngine::generateDemoSamples(float* buffer, size_t count, float sample_rate_hz) {
    float base_freq = 48.5f; // ~2910 RPM (standard 4-pole / small fan speed)
    float dt = 1.0f / sample_rate_hz;
    
    for (size_t i = 0; i < count; i++) {
        demo_phase += 2.0f * (float)M_PI * base_freq * dt;
        if (demo_phase >= 2.0f * (float)M_PI) demo_phase -= 2.0f * (float)M_PI;
        
        demo_harmonic_phase += 2.0f * (float)M_PI * (base_freq * 2.0f) * dt;
        if (demo_harmonic_phase >= 2.0f * (float)M_PI) demo_harmonic_phase -= 2.0f * (float)M_PI;
        
        // Random slight white noise
        float noise = ((float)(rand() % 1000) / 500.0f - 1.0f) * 0.02f;
        
        if (demo_fault == STATE_HEALTHY) {
            // Pristine, smooth, fluid blue sine wave
            buffer[i] = 0.08f * sinf(demo_phase) + noise;
        } else if (demo_fault == STATE_CRITICAL_UNBALANCE) {
            // Violent 1X unbalance: 5x amplitude + wobbling modulation
            float modulation = 1.0f + 0.15f * sinf(demo_phase * 0.2f);
            buffer[i] = 0.95f * sinf(demo_phase) * modulation + 0.25f * sinf(demo_harmonic_phase) + noise * 3.0f;
        } else if (demo_fault == STATE_BEARING_DAMAGE) {
            // High Kurtosis impulsive impact shock pulses
            float impact = 0.0f;
            if ((rand() % 40) == 0) {
                impact = ((rand() % 2 == 0) ? 1.0f : -1.0f) * (0.8f + ((float)(rand() % 100) / 100.0f));
            }
            buffer[i] = 0.18f * sinf(demo_phase) + impact + noise * 4.0f;
        } else {
            // Loose chassis rattle
            buffer[i] = 0.45f * sinf(demo_phase * 0.5f) + 0.35f * sinf(demo_phase) + noise * 2.5f;
        }
    }
}
