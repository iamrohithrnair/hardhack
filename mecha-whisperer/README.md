# 🩺 MECHA-WHISPERER: The Stethoscope for Machines
### Edge TinyML & Micro-Vibration Diagnostic Stethoscope on ESP32-S3 Touch AMOLED 1.8"

![Mecha-Whisperer AMOLED HUD](simulator/preview.png)

---

## 💡 Concept Overview
Cameras and computer vision are built for humans, but **machines speak in micro-vibrations**. 

**MECHA-WHISPERER** is a handheld, tactile diagnostic instrument designed for instant physical medical examination of rotating machinery (3D printers, CNC spindles, industrial pumps, car engines, turbines, and washing machines).

By pressing the board physically against a machine housing:
1. **6-Axis QMI8658 IMU** measures micro-vibrations at **1,000 Hz** with sub-milli-g sensitivity.
2. **ES8311 I2S Audio Codec & Digital Microphone** captures acoustic hums, bearing friction chatter, and cavitation up to **8 kHz**.
3. **ESP32-S3 Dual-Core DSP Engine** computes real-time Radix-2 FFT spectral breakdowns, ISO 10816 vibration velocity, Kurtosis impact metrics, and anomaly scores at 50 Hz.
4. **368 × 448 QSPI AMOLED Display** renders real-time glowing cyberpunk medical HUDs, fluid neon oscilloscopes, 24-band FFT visualizers, and instant alert state transitions.

---

## 🛠️ Hardware Specifications

| Component | Specification | Function |
| :--- | :--- | :--- |
| **MCU** | ESP32-S3R8 (Dual-Core LX7 @ 240MHz, 8MB PSRAM, 16MB Flash) | Core 0: 1kHz DSP / FFT; Core 1: 60 FPS AMOLED UI |
| **Display** | 1.8-inch AMOLED (368 × 448 resolution, QSPI 4-wire) | True zero blacks, ultra-vivid contrast HUD |
| **Touch** | Capacitive Touchscreen (FT3168 / CST820) | One-tap baseline calibration, screen modes |
| **IMU** | QMI8658 6-Axis (Accelerometer + Gyroscope) | Solid-state physical vibration transducer |
| **Audio Codec** | ES8311 Audio Codec + Microphone + Speaker PA (GPIO46) | Acoustic stethoscope listening & audible alerts |
| **PMU** | AXP2101 Power Management Unit | LiPo battery charging, battery voltage & temp |
| **Expander** | XCA9554 8-bit I2C IO Expander | Hardware reset controls for Display & Touch |

---

## 📐 Mathematical & DSP Anomaly Engine

### 1. In-Place Radix-2 Fast Fourier Transform (FFT)
The system applies a precomputed Hanning window function $w[n] = 0.5 \left(1 - \cos\left(\frac{2\pi n}{N-1}\right)\right)$ to suppress spectral leakage, followed by in-place Danielson-Lanczos FFT:
$$X[k] = \sum_{n=0}^{N-1} x[n] \cdot e^{-j 2\pi k n / N}$$

### 2. ISO 10816 Vibration Severity Velocity
Calculates RMS vibration velocity $v_{rms}$ (in mm/s) to classify industrial machinery health:
$$v_{rms} \approx \frac{a_{rms} \cdot 9806.65}{2\pi f_0}$$
* **Class A (< 1.12 mm/s):** Optimal / Healthy (Cyan HUD)
* **Class B (1.12 - 2.8 mm/s):** Acceptable Operation (Yellow HUD)
* **Class C (2.8 - 4.5 mm/s):** Unsatisfactory / Warning (Amber HUD)
* **Class D (> 4.5 mm/s):** Unacceptable / Critical Failure (Fiery Red Alert)

### 3. Kurtosis & Impact Detection (Bearing Spalling)
$$Kurtosis = \frac{\frac{1}{N} \sum_{i=1}^N (x_i - \bar{x})^4}{\left(\frac{1}{N} \sum_{i=1}^N (x_i - \bar{x})^2\right)^2}$$
* **Normal Gaussian vibration:** $Kurtosis \approx 3.0$
* **Bearing race pitting / ball damage:** $Kurtosis > 4.5 - 12.0$ (High impulsive impact peaks)

---

## 🖥️ AMOLED Screen Modes

1. **🩺 Main Medical HUD (Default):**
   * Medical Health Score Gauge ($0 - 100\%$)
   * Live Micro-Vibration Oscilloscope (Neon Cyan vs Fiery Red)
   * 24-Band Vibration FFT Spectrum ($0 - 500 \text{ Hz}$)
   * Real-time RMS, Kurtosis, and ISO 10816 Class badges
2. **📈 FFT Spectral Zoom:**
   * Expanded 32-Band Vibration Spectrum with $1\times, 2\times, 3\times$ RPM harmonic peaks
   * $0 - 8 \text{ kHz}$ Acoustic microphone spectrum
3. **📋 ISO 10816 Machine Doctor Report:**
   * Full diagnostic checklist, remaining useful life estimate, and prescriptive action
4. **🔊 Audio Stethoscope Station:**
   * Live acoustic transducer listening mode (routes mechanical hum through onboard speaker)
   * Dynamic acoustic radar visualizer

---

## 🚀 How to Build & Flash

### Option A: PlatformIO (Recommended)
```bash
cd mecha-whisperer
pio run -t upload
pio device monitor -b 115200
```

### Option B: Arduino IDE
1. Open `mecha-whisperer/mecha_whisperer.ino` in the Arduino IDE.
2. Select Board: **ESP32S3 Dev Module**.
3. Settings:
   * **PSRAM:** OPI PSRAM
   * **Flash Size:** 16MB (128Mb)
   * **USB CDC On Boot:** Enabled
   * **Upload Speed:** 921600
4. Click **Upload**.

---

## 🌐 Interactive Web Simulator
To test and interact with the virtual AMOLED screen and Web Audio mechanical synthesizer directly in your browser:
Open `mecha-whisperer/simulator/index.html` in any web browser!
