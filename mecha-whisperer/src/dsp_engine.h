#pragma once
#include <Arduino.h>
#include <math.h>

#define FFT_SIZE            256     // 256-point FFT for micro-vibration analysis
#define FFT_HALF_SIZE       128     // Usable frequency bins
#define BARS_COUNT          24      // Visualizer spectral bar count
#define ACOUSTIC_FFT_SIZE   256     // Microphone audio FFT size

// Diagnostic Health State Enumeration
enum DiagnosticState {
    STATE_CALIBRATING = 0,
    STATE_HEALTHY,          // Normal harmonious operation (Cyan/Green)
    STATE_WARNING,          // Mild anomaly/wear (Amber)
    STATE_CRITICAL_UNBALANCE, // Severe 1X unbalance (Aggressive Red/Orange)
    STATE_BEARING_DAMAGE,   // High Kurtosis impact chatter (Flashing Red)
    STATE_LOOSE_MOUNT       // Subharmonic resonance/rattle (Orange)
};

// Vibration & Acoustic Analysis Metrics Structure
struct DiagnosticMetrics {
    float peak_freq_hz;         // Dominant vibration frequency (Hz)
    uint32_t estimated_rpm;     // Rotational speed (RPM)
    float rms_acceleration_g;   // RMS vibration in g-force
    float peak_to_peak_g;       // Peak-to-peak amplitude
    float iso_vibration_vel;    // ISO 10816 velocity in mm/s
    float kurtosis;             // Kurtosis factor (impulsive impacts)
    float crest_factor;         // Crest factor
    float acoustic_db;          // Acoustic loudness in dB
    float harmonic_distortion;  // THD of vibration wave
    
    // Overall Health Score & Anomaly Index
    float anomaly_index;        // 0.0 to 100.0% (0 = pristine, 100 = catastrophic)
    uint8_t health_score;       // 100% - Anomaly% (98% = Healthy, 15% = Critical)
    DiagnosticState state;      // Current diagnostic classification
    const char* diagnosis_text; // Short diagnosis string for AMOLED HUD
    const char* recommendation; // Prescriptive maintenance guidance
    
    // Spectral Visualizer Bins (0 to 1.0 normalized)
    float visual_spectrum[BARS_COUNT];
    float visual_spectrum_peak[BARS_COUNT];
    float acoustic_spectrum[BARS_COUNT];
};

// Baseline Reference Profile for Anomaly Comparison
struct BaselineProfile {
    bool is_calibrated;
    float base_rms_g;
    float base_peak_freq_hz;
    float base_kurtosis;
    float base_acoustic_db;
    float base_spectrum[BARS_COUNT];
};

class DSPEngine {
public:
    DSPEngine();
    void init();
    
    // Process new batch of vibration samples
    void processVibration(const float* raw_samples, size_t count, float sample_rate_hz);
    
    // Process new batch of microphone audio samples
    void processAudio(const int16_t* audio_samples, size_t count);
    
    // Calibration routine: Learn machine nominal baseline
    void startCalibration(uint16_t samples_to_gather = 60);
    void updateCalibration();
    bool isCalibrating() const { return calibration_active; }
    
    // Retrieve current diagnostic metrics
    const DiagnosticMetrics& getMetrics() const { return current_metrics; }
    const BaselineProfile& getBaseline() const { return baseline; }
    
    // Demo Mode Injection (Generates synthetic unbalance/healthy signal for testing)
    void setDemoMode(bool enabled) { demo_mode = enabled; }
    bool isDemoMode() const { return demo_mode; }
    void setDemoFault(DiagnosticState fault_type) { demo_fault = fault_type; }
    DiagnosticState getDemoFault() const { return demo_fault; }
    void generateDemoSamples(float* buffer, size_t count, float sample_rate_hz);

private:
    DiagnosticMetrics current_metrics;
    BaselineProfile baseline;
    
    // FFT Computation Buffers (In-place Radix-2)
    float fft_real[FFT_SIZE];
    float fft_imag[FFT_SIZE];
    float hanning_window[FFT_SIZE];
    float fft_mag[FFT_HALF_SIZE];
    
    // Audio FFT Buffers
    float audio_real[ACOUSTIC_FFT_SIZE];
    float audio_imag[ACOUSTIC_FFT_SIZE];
    float audio_mag[ACOUSTIC_FFT_SIZE / 2];
    
    // Calibration State
    bool calibration_active;
    uint16_t calibration_counter;
    uint16_t calibration_target;
    float cal_accum_rms;
    float cal_accum_freq;
    float cal_accum_kurtosis;
    float cal_accum_db;
    
    // Demo Simulation State
    bool demo_mode;
    DiagnosticState demo_fault;
    float demo_phase;
    float demo_harmonic_phase;
    
    // Helper Internal Algorithms
    void applyHanningWindow(float* data, size_t size);
    void computeFFT(float* real, float* imag, size_t n);
    void computeMagnitudes(const float* real, const float* imag, float* mag, size_t half_n);
    void extractSpectralFeatures(float sample_rate_hz);
    void evaluateDiagnosticState();
    void mapToVisualBins();
};
