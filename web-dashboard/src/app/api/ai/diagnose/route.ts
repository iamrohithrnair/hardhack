import { NextRequest, NextResponse } from "next/server";

export async function POST(req: NextRequest) {
  try {
    const body = await req.json();
    const { rpm, f0, rms, kurt, iso, score, state, machine } = body;

    const machineName = machine?.name || "Target Asset";
    const targetType = machine?.detectionTarget || "vibration";
    const warnRms = machine?.warningRms || 0.25;
    const critRms = machine?.criticalRms || 0.85;

    let faultType = "Nominal Dynamic Baseline";
    let rootCause = `${machineName} is operating within optimal physical baselines (${rms}g RMS vs ${warnRms}g limit). Harmonic symmetry is preserved.`;
    let urgency = "LOW (Nominal Health)";
    let rulHours = "> 10,000 Operating Hours";
    let actions = [
      "Maintain nominal baseline monitoring.",
      "Record acoustic and vibration spectral stamps.",
      "No anomaly intervention required."
    ];

    // Domain-Specific Diagnostic Rules
    if (targetType === "seismic") {
      if (score < 40 || rms > critRms) {
        faultType = "CRITICAL SEISMIC EVENT: Severe S-Wave Shear Resonance";
        rootCause = `High acceleration (${rms}g RMS) detected at ${f0} Hz fundamental. Lateral shear wave energy exceeds building dampening limits with potential structural hazard.`;
        urgency = "RED ALERT / IMMEDIATE EVACUATION";
        rulHours = "Event In Progress";
        actions = [
          "Trigger automated gas and elevator shutoff valves.",
          "Broadcast building seismic early warning evacuation protocol.",
          "Inspect structural joints, expansion dampers, and foundation bolts for micro-fractures."
        ];
      } else if (rms > warnRms) {
        faultType = "P-Wave Compressional Tremor / Wind Vortex Shedding";
        rootCause = `Elevated low-frequency acceleration (${rms}g RMS at ${f0} Hz) indicates primary tectonic tremor or high-wind aeroelastic vortex lock-in.`;
        urgency = "AMBER / SEISMIC ADVISORY";
        rulHours = "Monitor Real-Time Trend";
        actions = [
          "Engage tuned mass damper systems.",
          "Alert facility safety team to monitor structural deflection gauges."
        ];
      }
    } else if (targetType === "impact") {
      if (score < 40 || kurt > 5.5 || rms > critRms) {
        faultType = "HARD IMPACT / HUMAN FALL DETECTED";
        rootCause = `Severe deceleration spike (${rms}g RMS) with high kurtosis (${kurt}) matches biomechanical signature of a hard ground impact following free-fall loss of traction.`;
        urgency = "EMERGENCY / SOS DISPATCH";
        rulHours = "Immediate Responder Required";
        actions = [
          "Dispatch automated caregiver / emergency contact SMS with GPS coordinates.",
          "Initiate 2-way audio voice verification over device speaker.",
          "Check for post-impact mobility or prolonged static posture."
        ];
      } else if (score < 70) {
        faultType = "Stumble / High-G Movement Warning";
        rootCause = `Transient dynamic acceleration (${rms}g) without severe impact spike. Indicates sudden posture adjustment or rapid motion.`;
        urgency = "INFO / ACTIVITY DETECTED";
        rulHours = "Active Normal Tracking";
        actions = ["Log activity event to daily biomechanical timeline."];
      }
    } else {
      // Rotating Machine & General Asset
      if (score < 30 || state === 3 || rms > critRms) {
        faultType = `Critical Mass Imbalance / Cavitation (${f0} Hz 1X Harmonic)`;
        rootCause = `Severe vibration energy (${rms}g RMS) exceeds critical threshold (${critRms}g). Dominant peak at ${f0 || 48.5} Hz (${rpm || 2910} RPM) indicates rotor blade unbalance, impeller cavitation, or shaft eccentricity.`;
        urgency = "CRITICAL / IMMINENT FAILURE";
        rulHours = "< 48 Operating Hours (Fatigue risk)";
        actions = [
          "IMMEDIATE ACTION: Halt rotating equipment to prevent catastrophic failure.",
          "Perform dynamic 2-plane balancing on rotor/impeller assembly.",
          "Inspect set screws, fluid inlet flow, and foreign mass accumulation."
        ];
      } else if (kurt > 4.5 || state === 4) {
        faultType = "Bearing Raceway Impact Pitting (BPFO / BPFI)";
        rootCause = `High Kurtosis (${kurt}) exceeds Gaussian baseline (3.0), indicating impulsive shock waves characteristic of ball-bearing race micro-spalling or mechanical clicking.`;
        urgency = "HIGH (Unscheduled Maintenance)";
        rulHours = "< 250 Operating Hours";
        actions = [
          "Schedule precision bearing replacement.",
          "Inspect lubricant for metallic particulate debris.",
          "Verify housing alignment and preload."
        ];
      } else if (score < 70 || rms > warnRms) {
        faultType = "Mechanical Looseness / Elevated Resonant Anomaly";
        rootCause = `Elevated vibration (${rms}g RMS vs ${warnRms}g baseline) indicates structural bolt loosening, belt slack, or early wear.`;
        urgency = "MEDIUM (Monitor Trend)";
        rulHours = "~ 2,500 Operating Hours";
        actions = [
          "Torque mounting fasteners to specified specification.",
          "Check anti-vibration rubber isolation dampers."
        ];
      }
    }

    return NextResponse.json({
      success: true,
      timestamp: new Date().toISOString(),
      diagnosis: {
        faultType,
        healthScore: score,
        isoClass: iso < 1.12 ? "Class A (Optimal)" : (iso < 2.8 ? "Class B (Acceptable)" : (iso < 4.5 ? "Class C (Warning)" : "Class D (Critical Danger)")),
        rootCause,
        urgency,
        estimatedRUL: rulHours,
        recommendedActions: actions,
        technicalMetrics: {
          assetName: machineName,
          detectionDomain: targetType,
          rotationalSpeedRPM: rpm,
          fundamentalFrequencyHz: f0,
          rmsAccelerationG: rms,
          kurtosisImpactFactor: kurt,
          vibrationVelocityMmS: iso
        }
      }
    });
  } catch (error: any) {
    return NextResponse.json(
      { success: false, error: error.message || "Failed to generate diagnosis" },
      { status: 500 }
    );
  }
}
