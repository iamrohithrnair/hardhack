# 🎙️ MECHA-WHISPERER: 1-Minute Live Demo & Pitch Script

> **"We spend billions on visual AI, but machines die in the dark. Mecha-Whisperer feels the failure before you ever see it."**

---

## ⏱️ The 60-Second Timed Choreography

```
+-----------------------------------------------------------------------------------+
| TIME    | PHYSICAL ACTION                  | DISPLAY STATE     | AUDIO / VERBAL   |
+-----------------------------------------------------------------------------------+
| 00-15s  | Place desk fan on table, turn on | Idle Medical HUD  | "The Blindspot"  |
| 15-30s  | Press Mecha-Whisperer to fan base| Glowing Cyan Sine | "Physical Exam"  |
| 30-45s  | Stick Blu-Tack / tape to blade   | Ready             | "Creating Fault" |
| 45-60s  | Press Mecha-Whisperer to fan base| Flashing Red Alert| "The Stethoscope"|
+-----------------------------------------------------------------------------------+
```

---

### Phase 1: The Hook (00:00 - 00:15)
* **Physical Action:** Place a small, smooth-running USB desk fan in front of the judges. Turn it on.
* **Verbal Delivery:** 
  > *"Every factory in the world relies on rotating machinery—turbines, pumps, conveyors, and car engines. But when they fail, they cost $50,000 an hour in downtime. Today, we use cameras and vision models, but machines don’t break visually first. **They speak in micro-vibrations.**"*
* **Device Action:** Hold the Mecha-Whisperer in hand. AMOLED is on, showing the high-contrast dark cyberpunk HUD.

---

### Phase 2: The Physical Medical Exam (00:15 - 00:30)
* **Physical Action:** Physically press the Mecha-Whisperer board firmly against the base/chassis of the running fan.
* **Display State:** 
  * The AMOLED instantly renders a **fluid, glowing Electric Cyan sine wave**.
  * The Health Meter displays **`98% HEALTHY`**.
  * FFT visualizer displays a clean single fundamental spike at **`48.5 Hz (2,910 RPM)`**.
  * ISO-10816 badge indicates **`CLASS A [OPTIMAL]`**.
* **Verbal Delivery:**
  > *"Like a doctor with a stethoscope, I press Mecha-Whisperer against the metal. The 6-axis IMU samples micro-vibrations at 1,000 Hz, while our on-chip FFT breaks down the mechanical harmonics. Look at the screen: smooth, rhythmic blue sine wave. The machine is healthy."*

---

### Phase 3: Inducing the Failure (00:30 - 00:45)
* **Physical Action:** Turn off the fan for 2 seconds. Stick a small piece of **Blu-Tack** or folded tape onto one single blade. Turn the fan back on.
* **Verbal Delivery:**
  > *"Now, let's introduce an invisible defect: 2 grams of unbalance on a rotor blade—the most common cause of catastrophic bearing destruction in wind turbines and aircraft."*

---

### Phase 4: The Anomaly Detection (00:45 - 01:00)
* **Physical Action:** Press the Mecha-Whisperer against the fan chassis again.
* **Display State:**
  * **Screen instantly flashes AGGRESSIVE ALERT RED & ORANGE**.
  * The waveform violently wobbles and distorts into a chaotic multi-harmonic trace.
  * Health score plummets to **`18% CRITICAL`**.
  * Diagnosis Banner flashes: **`CRITICAL: ROTOR IMBALANCE (1X-RPM)`**.
  * Onboard speaker emits a crisp alert pulse.
* **Verbal Delivery:**
  > *"Instantly, the screen turns violent red. The anomaly index jumped by 400%, detecting a massive 1X RPM imbalance spike and bearing race impact chatter before any human eye could ever see it. Predictive maintenance for under $50. That is Mecha-Whisperer."*

---

## 🎯 Key Judge Q&A & Defenses

| Question | Winning Answer |
| :--- | :--- |
| **Why not just use a microphone?** | *"Microphones capture ambient room echoes, people talking, and background noise. By pressing our 6-axis IMU directly against the chassis, we get direct mechanical solid-state coupling. We then fuse IMU micro-vibration with acoustic ultrasound via the ES8311 codec for true dual-modal diagnostics."* |
| **How does the algorithm work?** | *"We run an in-place Radix-2 FFT and compute high-order statistical moments on Core 0 of the ESP32-S3: RMS acceleration, Kurtosis for bearing impact pitting, Crest Factor, and fundamental frequency tracking according to ISO 10816 vibration severity standards."* |
| **Can it learn custom machine baselines?** | *"Yes! A single tap on `[CALIB]` calibrates the baseline of any novel machine in 2 seconds, recording nominal RPM, spectral centroid, and baseline vibration energy."* |
| **What is the Bill of Materials (BOM)?** | *"The entire system—ESP32-S3, 1.8" AMOLED, capacitive touch, 6-axis IMU, ES8311 codec, AXP2101 PMU, and LiPo battery—is under $25 at scale, replacing $10,000 industrial vibration analyzer carts."* |
