export type AIModelType = "gemini-3.7-flash" | "gpt-5.6-sol" | "gpt-5.6-terra" | "gpt-5.6-luna";

export interface MachineProfile {
  id: string;
  name: string;
  category: string;
  motorType: string;
  nominalRPM: number;
  fundamentalHz: number;
  isoClass: "Class I" | "Class II" | "Class III" | "Class IV";
  warningRms: number;
  criticalRms: number;
  kurtosisThreshold?: number;
  image: string;
  bearingType: string;
  mountTorque: string;
  // Custom Device & Event Extensions
  isCustom?: boolean;
  isLearnedFromHardware?: boolean;
  detectionTarget?: "vibration" | "acoustic" | "seismic" | "impact" | "hybrid";
  aiModel?: AIModelType;
  customPrompt?: string;
  physicsSummary?: string;
  anomalyRules?: string[];
  baselineRms?: number;
  baselineKurt?: number;
  baselineF0?: number;
  learnedAt?: string;
}

export const PRESET_DEVICE_TEMPLATES: Partial<MachineProfile>[] = [
  {
    id: "fridge-compressor",
    name: "Smart Inverter Refrigerator Compressor",
    category: "Home Appliance & Refrigeration",
    motorType: "Variable-Speed BLDC Inverter Compressor",
    nominalRPM: 1800,
    fundamentalHz: 30.0,
    isoClass: "Class I",
    warningRms: 0.15,
    criticalRms: 0.65,
    kurtosisThreshold: 3.8,
    image: "https://images.unsplash.com/photo-1571175443880-49e1d25b2bc5?w=600&auto=format&fit=crop&q=80",
    bearingType: "Hermetic Hydrodynamic Journal Bearing",
    mountTorque: "14.0 Nm Rubber Damper",
    detectionTarget: "hybrid",
    physicsSummary: "Monitors reciprocating piston stroke, suction valve reed flutter, refrigerant flow cavitation hiss, and inverter carrier harmonics.",
    anomalyRules: [
      "Piston slap / Knock: High impulse kurtosis > 3.8 indicating mechanical shock",
      "Valve reed flutter: 2X harmonic spike at 60.0 Hz exceeding 0.15g",
      "Refrigerant leak / Gas starvation: Ultrasonic acoustic turbulence > 200 Hz"
    ]
  },
  {
    id: "earthquake-seismic",
    name: "Seismic Early Warning & Tremor Detector",
    category: "Geophysical & Structural",
    motorType: "P-Wave & S-Wave Seismic Transducer",
    nominalRPM: 0,
    fundamentalHz: 4.5,
    isoClass: "Class I",
    warningRms: 0.045,
    criticalRms: 0.15,
    kurtosisThreshold: 4.2,
    image: "https://images.unsplash.com/photo-1509198397868-475647b2a1e5?w=600&auto=format&fit=crop&q=80",
    bearingType: "Tectonic Bedrock Anchor",
    mountTorque: "Rigid Concrete Coupling",
    detectionTarget: "seismic",
    physicsSummary: "Detects low-frequency P-wave vertical compressional tremors (1-8 Hz) prior to destructive S-wave lateral shear displacement.",
    anomalyRules: [
      "P-wave threshold: RMS acceleration > 0.045g in 2-8 Hz band",
      "S-wave critical alert: Lateral shear RMS > 0.15g with high duration",
      "Building resonance alert: Peak frequency matches 1.2 Hz structural sway"
    ]
  },
  {
    id: "fall-detection",
    name: "Human Fall & Slip Impact Detector",
    category: "Biomechanical & Health",
    motorType: "Wearable 6-Axis Inertial Stethoscope",
    nominalRPM: 0,
    fundamentalHz: 1.5,
    isoClass: "Class I",
    warningRms: 0.60,
    criticalRms: 2.80,
    kurtosisThreshold: 6.5,
    image: "https://images.unsplash.com/photo-1576091160550-2173dba999ef?w=600&auto=format&fit=crop&q=80",
    bearingType: "Torso Belt / Lanyard Clip",
    mountTorque: "Elastic Body Contact",
    detectionTarget: "impact",
    physicsSummary: "Identifies free-fall weightlessness (< 0.2g for >150ms) followed immediately by severe high-g ground impact spike (> 2.8g) with high kurtosis.",
    anomalyRules: [
      "Free-fall phase: Vector magnitude drops below 0.2g for 150-500ms",
      "Impact spike: Hard deceleration exceeding 2.8g with kurtosis > 6.5",
      "Post-fall immobility: Static baseline (<0.05g AC) for > 5 seconds"
    ]
  },
  {
    id: "espresso-pump",
    name: "La Marzocco Rotary Vane Pump",
    category: "Commercial Beverage",
    motorType: "Single-Phase Rotary Vane 9-Bar Pump",
    nominalRPM: 1400,
    fundamentalHz: 23.3,
    isoClass: "Class I",
    warningRms: 0.18,
    criticalRms: 0.55,
    kurtosisThreshold: 3.8,
    image: "https://images.unsplash.com/photo-1514432324607-a09d9b4aefdd?w=600&auto=format&fit=crop&q=80",
    bearingType: "Fluid-O-Tech Carbon Graphite Bearing",
    mountTorque: "18.0 Nm Dampered Mount",
    detectionTarget: "hybrid",
    physicsSummary: "Monitors 9-bar brewing pressure cavitation, pump head scale build-up, and dry-run cavitation acoustic flutter.",
    anomalyRules: [
      "Cavitation noise: Ultrasonic acoustic hiss > 250 Hz with RMS > 0.35g",
      "Vane blade wear: 4X pump harmonic spike at 93.2 Hz",
      "Dry boil / water inlet starvation: High-frequency chatter with drop in flow load"
    ]
  },
  {
    id: "turbopump-rocket",
    name: "Cryogenic Turbopump & Gas Generator",
    category: "Aerospace Propulsion",
    motorType: "High-Pressure Liquid Oxygen (LOX) Turbopump",
    nominalRPM: 32000,
    fundamentalHz: 533.3,
    isoClass: "Class IV",
    warningRms: 0.85,
    criticalRms: 3.20,
    kurtosisThreshold: 5.8,
    image: "https://images.unsplash.com/photo-1517976487504-59a1c0188b8c?w=600&auto=format&fit=crop&q=80",
    bearingType: "Cryogenic Hybrid Ceramic Ball Bearings",
    mountTorque: "120.0 Nm Inconel Flange",
    detectionTarget: "vibration",
    physicsSummary: "Detects sub-synchronous whirl, inducer cavitation instability, and blade pass frequency (BPF) excitation at supersonic velocities.",
    anomalyRules: [
      "Inducer cavitation: Broad-band high energy between 100-300 Hz",
      "Blade pass flutter: Severe 12X harmonic spike at 6,400 Hz",
      "Bearing cage fracture: Kurtosis spike exceeding 6.0"
    ]
  },
  {
    id: "bridge-resonance",
    name: "Suspension Bridge Cable Stay",
    category: "Civil Infrastructure",
    motorType: "Vortex-Induced Wind Resonance Monitor",
    nominalRPM: 0,
    fundamentalHz: 0.85,
    isoClass: "Class III",
    warningRms: 0.12,
    criticalRms: 0.45,
    kurtosisThreshold: 3.5,
    image: "https://images.unsplash.com/photo-1545893835-abaa50cbe628?w=600&auto=format&fit=crop&q=80",
    bearingType: "Viscous Tuned Mass Damper",
    mountTorque: "750.0 Nm High-Tensile Clamp",
    detectionTarget: "seismic",
    physicsSummary: "Measures wind-lock vortex shedding and cable galloping tension oscillations to prevent Tacoma Narrows-style catastrophic aeroelastic flutter.",
    anomalyRules: [
      "Vortex lock-in: Persistent sine oscillation at 0.85 Hz exceeding 0.12g",
      "Cable slack / anchorage slip: Step-function jerk spike (> 0.4g)",
      "Rain-wind vibration: Coupled 3.2 Hz 4th-mode harmonic amplification"
    ]
  },
  {
    id: "cardiac-stethoscope",
    name: "Acoustic Cardiac Stethoscope",
    category: "Medical & Diagnostic",
    motorType: "Phonocardiogram (PCG) Stethoscope Transducer",
    nominalRPM: 72,
    fundamentalHz: 1.2,
    isoClass: "Class I",
    warningRms: 0.08,
    criticalRms: 0.32,
    kurtosisThreshold: 4.8,
    image: "https://images.unsplash.com/photo-1584515979956-d9f6e5d09982?w=600&auto=format&fit=crop&q=80",
    bearingType: "Acoustic Silicone Diaphragm",
    mountTorque: "Chest Contact Pressure",
    detectionTarget: "acoustic",
    physicsSummary: "Captures S1 (mitral/tricuspid) and S2 (aortic/pulmonic) cardiac valve closures and turbulent systolic murmur flutters.",
    anomalyRules: [
      "Systolic murmur: High-frequency turbulent acoustic energy between S1 and S2",
      "Arrhythmia: Premature ventricular contraction interval irregularity (>20% jitter)",
      "Aortic stenosis: Diamond-shaped crescendo-decrescendo ejection flutter"
    ]
  }
];

export const DEFAULT_MACHINES: MachineProfile[] = [
  {
    id: "fridge-compressor",
    name: "Smart Inverter Refrigerator Compressor",
    category: "Home Appliance & Refrigeration",
    motorType: "Variable-Speed BLDC Inverter Compressor",
    nominalRPM: 1800,
    fundamentalHz: 30.0,
    isoClass: "Class I",
    warningRms: 0.15,
    criticalRms: 0.65,
    kurtosisThreshold: 3.8,
    image: "https://images.unsplash.com/photo-1571175443880-49e1d25b2bc5?w=600&auto=format&fit=crop&q=80",
    bearingType: "Hermetic Hydrodynamic Journal Bearing",
    mountTorque: "14.0 Nm Rubber Damper",
    detectionTarget: "hybrid",
    physicsSummary: "Monitors reciprocating piston stroke, suction valve reed flutter, refrigerant flow cavitation hiss, and inverter carrier harmonics.",
    anomalyRules: [
      "Piston slap / Knock: High impulse kurtosis > 3.8 indicating mechanical shock",
      "Valve reed flutter: 2X harmonic spike at 60.0 Hz exceeding 0.15g",
      "Refrigerant leak / Gas starvation: Ultrasonic acoustic turbulence > 200 Hz"
    ]
  },
  {
    id: "desk-fan",
    name: "Industrial Desk Fan 01",
    category: "Cooling Fan & Blower",
    motorType: "4-Pole AC Induction Motor",
    nominalRPM: 2910,
    fundamentalHz: 48.5,
    isoClass: "Class I",
    warningRms: 0.25,
    criticalRms: 0.80,
    kurtosisThreshold: 4.0,
    image: "https://images.unsplash.com/photo-1581092160607-ee22621dd758?w=600&auto=format&fit=crop&q=80",
    bearingType: "Dual Deep-Groove 608RS",
    mountTorque: "12.5 Nm",
    detectionTarget: "vibration",
    physicsSummary: "Monitors 1X rotor balance and 608RS ball bearing race spalling.",
    anomalyRules: [
      "1X Unbalance: Fundamental amplitude at 48.5 Hz > 0.25g",
      "Bearing defect: Kurtosis > 4.2 indicating impulsive shock waves"
    ]
  },
  {
    id: "3d-printer",
    name: "Voron 2.4 Extruder & Stepper",
    category: "3D Printer CoreXY",
    motorType: "NEMA 17 Stepper (1.8°)",
    nominalRPM: 4800,
    fundamentalHz: 80.0,
    isoClass: "Class I",
    warningRms: 0.35,
    criticalRms: 1.10,
    kurtosisThreshold: 4.5,
    image: "https://images.unsplash.com/photo-1631553127988-3482a991f24d?w=600&auto=format&fit=crop&q=80",
    bearingType: "MR85ZZ Precision Linear",
    mountTorque: "3.2 Nm",
    detectionTarget: "vibration",
    physicsSummary: "Monitors CoreXY belt tension, linear rail binding, and nozzle extruder clicking."
  },
  {
    id: "washing-machine",
    name: "Whirlpool Drum Inverter Pump",
    category: "Home Appliance",
    motorType: "Direct-Drive Inverter BLDC",
    nominalRPM: 1200,
    fundamentalHz: 20.0,
    isoClass: "Class II",
    warningRms: 0.40,
    criticalRms: 1.50,
    kurtosisThreshold: 4.0,
    image: "https://images.unsplash.com/photo-1626806787461-102c1bfaaea1?w=600&auto=format&fit=crop&q=80",
    bearingType: "Heavy-Duty 6205-2RS",
    mountTorque: "45.0 Nm",
    detectionTarget: "vibration",
    physicsSummary: "Monitors high-spin dampener wear and uneven laundry load distribution."
  },
  {
    id: "cnc-spindle",
    name: "CNC 2.2kW Water-Cooled Spindle",
    category: "CNC Milling Machine",
    motorType: "High-Speed 3-Phase Brushless",
    nominalRPM: 24000,
    fundamentalHz: 400.0,
    isoClass: "Class I",
    warningRms: 0.20,
    criticalRms: 0.65,
    kurtosisThreshold: 5.0,
    image: "https://images.unsplash.com/photo-1504917599217-d4dc5ebe6122?w=600&auto=format&fit=crop&q=80",
    bearingType: "Angular Contact 7005C P4",
    mountTorque: "28.0 Nm",
    detectionTarget: "hybrid",
    physicsSummary: "Monitors endmill tool chatter, collet runout, and 24,000 RPM bearing wear."
  },
  {
    id: "auto-engine",
    name: "Automotive Alternator & Serpentine",
    category: "Automotive Powertrain",
    motorType: "Claw-Pole Rotor w/ Slip Rings",
    nominalRPM: 3600,
    fundamentalHz: 60.0,
    isoClass: "Class II",
    warningRms: 0.30,
    criticalRms: 0.95,
    kurtosisThreshold: 4.2,
    image: "https://images.unsplash.com/photo-1486006920555-c77dce18193b?w=600&auto=format&fit=crop&q=80",
    bearingType: "Sealed Alternator Ball Bearing",
    mountTorque: "35.0 Nm",
    detectionTarget: "hybrid",
    physicsSummary: "Monitors serpentine belt slip, alternator diode ripple hum, and bearing squeal."
  }
];
