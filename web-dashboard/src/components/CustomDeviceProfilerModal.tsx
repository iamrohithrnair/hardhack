"use client";

import React, { useState, useEffect, useRef } from "react";
import { 
  X, 
  Sparkles, 
  Radio, 
  Cpu, 
  Activity, 
  CheckCircle2, 
  Flame, 
  Waves, 
  Layers, 
  ArrowRight, 
  Sliders, 
  ShieldCheck, 
  Zap,
  RotateCcw,
  Bot
} from "lucide-react";
import { MachineProfile, AIModelType, PRESET_DEVICE_TEMPLATES } from "../types/machine";
import { TelemetryData } from "../hooks/useDeviceStream";

interface CustomDeviceProfilerModalProps {
  isOpen: boolean;
  onClose: () => void;
  telemetry: TelemetryData;
  onSaveProfile: (profile: MachineProfile) => void;
}

export const CustomDeviceProfilerModal: React.FC<CustomDeviceProfilerModalProps> = ({
  isOpen,
  onClose,
  telemetry,
  onSaveProfile
}) => {
  const [deviceName, setDeviceName] = useState("");
  const [category, setCategory] = useState("Custom Transducer");
  const [userDescription, setUserDescription] = useState("");
  const [selectedModel, setSelectedModel] = useState<AIModelType>("gemini-3.7-flash");
  
  // Profiling State
  const [profilingMode, setProfilingMode] = useState<"hardware" | "ai_only">("hardware");
  const [isSampling, setIsSampling] = useState(false);
  const [sampleProgress, setSampleProgress] = useState(0);
  const [samplesBuffer, setSamplesBuffer] = useState<{ rms: number; f0: number; kurt: number }[]>([]);
  const [measuredBaseline, setMeasuredBaseline] = useState<{ rms: number; f0: number; kurt: number } | null>(null);

  // Synthesis & Result State
  const [isSynthesizing, setIsSynthesizing] = useState(false);
  const [synthesizedProfile, setSynthesizedProfile] = useState<MachineProfile | null>(null);

  // Hardware sampling timer loop
  useEffect(() => {
    let interval: any;
    if (isSampling) {
      interval = setInterval(() => {
        setSampleProgress((prev) => {
          if (prev >= 100) {
            setIsSampling(false);
            return 100;
          }
          return prev + 10;
        });

        setSamplesBuffer((prev) => [
          ...prev,
          { rms: telemetry.rms, f0: telemetry.f0, kurt: telemetry.kurt }
        ]);
      }, 500);
    }
    return () => clearInterval(interval);
  }, [isSampling, telemetry.f0, telemetry.kurt, telemetry.rms]);

  // When sampling finishes, compute average baseline
  useEffect(() => {
    if (sampleProgress >= 100 && samplesBuffer.length > 0 && !measuredBaseline) {
      const avgRms = samplesBuffer.reduce((acc, s) => acc + s.rms, 0) / samplesBuffer.length;
      const avgF0 = samplesBuffer.reduce((acc, s) => acc + s.f0, 0) / samplesBuffer.length;
      const avgKurt = samplesBuffer.reduce((acc, s) => acc + s.kurt, 0) / samplesBuffer.length;

      setMeasuredBaseline({
        rms: Number(avgRms.toFixed(3)),
        f0: Number(avgF0.toFixed(1)),
        kurt: Number(avgKurt.toFixed(2))
      });
    }
  }, [sampleProgress, samplesBuffer, measuredBaseline]);

  const startSampling = () => {
    setSampleProgress(0);
    setSamplesBuffer([]);
    setMeasuredBaseline(null);
    setIsSampling(true);
  };

  const handleApplyPreset = (preset: Partial<MachineProfile>) => {
    setDeviceName(preset.name || "");
    setCategory(preset.category || "Custom Transducer");
    setUserDescription(preset.physicsSummary || "");
    setSynthesizedProfile(null);
  };

  const handleSynthesize = async () => {
    if (!deviceName.trim()) {
      alert("Please enter a device or event name.");
      return;
    }

    setIsSynthesizing(true);
    try {
      const res = await fetch("/api/ai/synthesize-device", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          deviceName,
          category,
          userDescription,
          aiModel: selectedModel,
          hardwareSampledData: measuredBaseline
        })
      });

      const data = await res.json();
      if (data.success && data.profile) {
        setSynthesizedProfile(data.profile);
      } else {
        alert("Synthesis error: " + (data.error || "Unknown error"));
      }
    } catch (err: any) {
      alert("Network error: " + err.message);
    } finally {
      setIsSynthesizing(false);
    }
  };

  const handleDeployProfile = () => {
    if (synthesizedProfile) {
      onSaveProfile(synthesizedProfile);
      onClose();
    }
  };

  if (!isOpen) return null;

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/70 backdrop-blur-md animate-fade-in">
      <div className="bg-[#12141A] text-white rounded-[32px] border border-white/10 w-full max-w-4xl max-h-[92vh] flex flex-col shadow-2xl overflow-hidden">
        {/* Header */}
        <div className="px-6 py-5 border-b border-white/10 flex justify-between items-center bg-[#161B24]">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-2xl bg-amber-500/20 border border-amber-500/30 flex items-center justify-center text-[#F5C544]">
              <Sparkles className="w-5 h-5" />
            </div>
            <div>
              <h2 className="text-lg font-bold text-white flex items-center gap-2">
                Custom Device & Anomaly Synthesizer
                <span className="px-2.5 py-0.5 rounded-full text-[10px] font-mono bg-sky-500/20 text-sky-400 font-bold">
                  AI-SDK MULTIMODAL
                </span>
              </h2>
              <p className="text-xs text-neutral-400">
                Profile physical micro-vibrations with the Waveshare ESP32 or synthesize any object with frontier AI models.
              </p>
            </div>
          </div>
          <button
            onClick={onClose}
            className="w-8 h-8 rounded-full bg-white/5 hover:bg-white/10 flex items-center justify-center text-neutral-400 hover:text-white transition-all cursor-pointer"
          >
            <X className="w-4 h-4" />
          </button>
        </div>

        {/* Modal Body */}
        <div className="p-6 overflow-y-auto flex flex-col gap-6 flex-1 text-xs">
          {/* Step 1: AI Model Selector */}
          <div className="flex flex-col gap-2">
            <label className="font-bold text-neutral-300 uppercase tracking-wider text-[11px] flex items-center gap-1.5">
              <Bot className="w-4 h-4 text-[#F5C544]" />
              Select Frontier AI Physics Model:
            </label>
            <div className="grid grid-cols-1 sm:grid-cols-4 gap-2.5">
              {[
                {
                  id: "gemini-3.7-flash" as AIModelType,
                  name: "Gemini 3.7 Flash",
                  sub: "Google DeepMind Multimodal",
                  badge: "RECOMMENDED",
                  color: "border-sky-500/40 bg-sky-500/10 text-sky-400"
                },
                {
                  id: "gpt-5.6-sol" as AIModelType,
                  name: "ChatGPT GPT-5.6 Sol",
                  sub: "High-Velocity Dynamics",
                  badge: "FASTEST",
                  color: "border-amber-500/40 bg-amber-500/10 text-amber-400"
                },
                {
                  id: "gpt-5.6-terra" as AIModelType,
                  name: "ChatGPT GPT-5.6 Terra",
                  sub: "Heavy Industry & Seismic",
                  badge: "GEOPHYSICAL",
                  color: "border-emerald-500/40 bg-emerald-500/10 text-emerald-400"
                },
                {
                  id: "gpt-5.6-luna" as AIModelType,
                  name: "ChatGPT GPT-5.6 Luna",
                  sub: "Precision Acoustic & Bio",
                  badge: "ACOUSTIC",
                  color: "border-purple-500/40 bg-purple-500/10 text-purple-400"
                }
              ].map((m) => (
                <button
                  key={m.id}
                  onClick={() => setSelectedModel(m.id)}
                  className={`p-3 rounded-2xl border text-left flex flex-col justify-between transition-all cursor-pointer ${
                    selectedModel === m.id
                      ? "bg-[#1E2330] border-[#F5C544] shadow-md ring-1 ring-[#F5C544]"
                      : "bg-[#161B24] border-white/5 hover:border-white/20 text-neutral-400"
                  }`}
                >
                  <div className="flex justify-between items-start mb-1">
                    <span className="font-bold text-white text-xs">{m.name}</span>
                    <span className={`text-[9px] font-mono px-1.5 py-0.5 rounded-md font-bold ${m.color}`}>
                      {m.badge}
                    </span>
                  </div>
                  <span className="text-[10px] text-neutral-400">{m.sub}</span>
                </button>
              ))}
            </div>
          </div>

          {/* Step 2: Instant Template Presets */}
          <div className="flex flex-col gap-2">
            <label className="font-bold text-neutral-300 uppercase tracking-wider text-[11px]">
              Or Choose an Instant Domain Template:
            </label>
            <div className="flex flex-wrap gap-2">
              {PRESET_DEVICE_TEMPLATES.map((tpl) => (
                <button
                  key={tpl.id}
                  onClick={() => handleApplyPreset(tpl)}
                  className="px-3 py-1.5 rounded-xl bg-white/5 hover:bg-[#F5C544] hover:text-black border border-white/10 text-[11px] font-semibold text-neutral-300 transition-all cursor-pointer flex items-center gap-1.5"
                >
                  <span>{tpl.name}</span>
                </button>
              ))}
            </div>
          </div>

          {/* Step 3: Name & Custom Description Input */}
          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            <div className="flex flex-col gap-1.5">
              <label className="font-bold text-neutral-300">Device or Anomaly Event Name:</label>
              <input
                type="text"
                value={deviceName}
                onChange={(e) => setDeviceName(e.target.value)}
                placeholder="e.g. Earthquake Early Warning, Human Fall, Espresso Machine, Jet Turbine..."
                className="w-full px-3.5 py-2.5 rounded-xl bg-[#0D1017] border border-white/10 text-white placeholder:text-neutral-600 focus:outline-none focus:border-[#F5C544]"
              />
            </div>

            <div className="flex flex-col gap-1.5">
              <label className="font-bold text-neutral-300">Category / Domain:</label>
              <input
                type="text"
                value={category}
                onChange={(e) => setCategory(e.target.value)}
                placeholder="e.g. Seismic & Geophysical, Biomechanical, Household, Aerospace..."
                className="w-full px-3.5 py-2.5 rounded-xl bg-[#0D1017] border border-white/10 text-white placeholder:text-neutral-600 focus:outline-none focus:border-[#F5C544]"
              />
            </div>
          </div>

          <div className="flex flex-col gap-1.5">
            <label className="font-bold text-neutral-300">Custom Physical Behavior / Anomaly Goal (Optional):</label>
            <textarea
              value={userDescription}
              onChange={(e) => setUserDescription(e.target.value)}
              rows={2}
              placeholder="Describe what you want to detect (e.g., 'Detect early 2-8 Hz P-wave tremors before S-wave damage', 'Detect freefall weightlessness followed by severe 3g impact spike')..."
              className="w-full px-3.5 py-2 rounded-xl bg-[#0D1017] border border-white/10 text-white placeholder:text-neutral-600 focus:outline-none focus:border-[#F5C544]"
            />
          </div>

          {/* Step 4: Profiling Method Selection */}
          <div className="flex flex-col gap-3 p-4 rounded-2xl bg-[#161B24] border border-white/10">
            <div className="flex justify-between items-center">
              <div>
                <strong className="text-white text-sm">Physical Baseline Sampling</strong>
                <p className="text-neutral-400 text-[11px]">
                  Place the Waveshare ESP32 on your actual device for 5 seconds to sample its real-world harmonic baseline.
                </p>
              </div>

              <div className="flex items-center gap-2">
                <button
                  onClick={() => setProfilingMode("hardware")}
                  className={`px-3 py-1 rounded-xl text-xs font-bold transition-all cursor-pointer ${
                    profilingMode === "hardware" ? "bg-[#F5C544] text-black" : "bg-white/5 text-neutral-400"
                  }`}
                >
                  Hardware Sample
                </button>
                <button
                  onClick={() => setProfilingMode("ai_only")}
                  className={`px-3 py-1 rounded-xl text-xs font-bold transition-all cursor-pointer ${
                    profilingMode === "ai_only" ? "bg-sky-500 text-white" : "bg-white/5 text-neutral-400"
                  }`}
                >
                  AI Synthesize Only
                </button>
              </div>
            </div>

            {profilingMode === "hardware" && (
              <div className="flex flex-col sm:flex-row items-center justify-between gap-4 p-4 rounded-xl bg-[#0D1017] border border-white/5">
                <div className="flex items-center gap-4">
                  <div className="relative w-14 h-14 flex items-center justify-center">
                    <svg className="w-full h-full -rotate-90" viewBox="0 0 44 44">
                      <circle cx="22" cy="22" r="18" stroke="#1F2636" strokeWidth="4" fill="none" />
                      <circle
                        cx="22"
                        cy="22"
                        r="18"
                        stroke="#F5C544"
                        strokeWidth="4"
                        fill="none"
                        strokeDasharray="113"
                        strokeDashoffset={113 - (113 * sampleProgress) / 100}
                        strokeLinecap="round"
                        className="transition-all duration-300"
                      />
                    </svg>
                    <span className="absolute font-mono font-bold text-xs text-white">
                      {isSampling ? `${sampleProgress}%` : measuredBaseline ? "DONE" : "IDLE"}
                    </span>
                  </div>

                  <div>
                    <div className="font-bold text-white text-xs">
                      {isSampling
                        ? "Sampling real-time IMU & Microphone..."
                        : measuredBaseline
                        ? "Calibration Baseline Captured!"
                        : "Ready to Profile"}
                    </div>
                    <div className="text-[10px] text-neutral-400 font-mono mt-0.5">
                      Live Telemetry: RMS: {telemetry.rms.toFixed(3)}g · F0: {(telemetry.f0 || 48.5).toFixed(1)}Hz · Kurt: {telemetry.kurt.toFixed(2)}
                    </div>
                  </div>
                </div>

                <div className="flex items-center gap-2">
                  <button
                    disabled={isSampling}
                    onClick={startSampling}
                    className="px-4 py-2 rounded-xl bg-[#F5C544] hover:bg-amber-400 text-black font-bold text-xs shadow-md transition-all cursor-pointer disabled:opacity-50"
                  >
                    {isSampling ? "Recording (5s)..." : measuredBaseline ? "Re-Sample Baseline" : "Start 5s Sampling"}
                  </button>
                </div>
              </div>
            )}

            {measuredBaseline && (
              <div className="flex items-center gap-4 text-xs font-mono text-emerald-400 bg-emerald-500/10 px-4 py-2 rounded-xl border border-emerald-500/20">
                <CheckCircle2 className="w-4 h-4 shrink-0" />
                <span>
                  Captured Baseline: <strong>{measuredBaseline.rms}g RMS</strong> at <strong>{measuredBaseline.f0} Hz</strong> (Kurtosis: <strong>{measuredBaseline.kurt}</strong>)
                </span>
              </div>
            )}
          </div>

          {/* Step 5: Synthesized Result Preview */}
          {synthesizedProfile && (
            <div className="p-5 rounded-2xl bg-[#1E2330] border border-[#F5C544]/30 flex flex-col gap-3 animate-fade-in">
              <div className="flex justify-between items-center border-b border-white/10 pb-2">
                <div className="flex items-center gap-2">
                  <span className="w-2.5 h-2.5 rounded-full bg-emerald-400" />
                  <strong className="text-sm font-bold text-white">{synthesizedProfile.name}</strong>
                  <span className="px-2 py-0.5 rounded-md bg-amber-500/20 text-[#F5C544] font-bold text-[10px]">
                    {synthesizedProfile.isoClass}
                  </span>
                </div>
                <span className="text-[10px] font-mono text-neutral-400">
                  Model: {synthesizedProfile.aiModel}
                </span>
              </div>

              <p className="text-neutral-300 text-xs italic">
                "{synthesizedProfile.physicsSummary}"
              </p>

              <div className="grid grid-cols-2 sm:grid-cols-4 gap-2 pt-1 font-mono text-[11px]">
                <div className="bg-black/30 p-2 rounded-xl">
                  <span className="text-neutral-500 text-[10px]">WARN RMS:</span>
                  <div className="text-[#F5C544] font-bold">{synthesizedProfile.warningRms}g</div>
                </div>
                <div className="bg-black/30 p-2 rounded-xl">
                  <span className="text-neutral-500 text-[10px]">CRITICAL RMS:</span>
                  <div className="text-rose-400 font-bold">{synthesizedProfile.criticalRms}g</div>
                </div>
                <div className="bg-black/30 p-2 rounded-xl">
                  <span className="text-neutral-500 text-[10px]">TARGET HZ:</span>
                  <div className="text-sky-400 font-bold">{synthesizedProfile.fundamentalHz} Hz</div>
                </div>
                <div className="bg-black/30 p-2 rounded-xl">
                  <span className="text-neutral-500 text-[10px]">KURTOSIS MAX:</span>
                  <div className="text-emerald-400 font-bold">{synthesizedProfile.kurtosisThreshold || 4.0}</div>
                </div>
              </div>

              {synthesizedProfile.anomalyRules && (
                <div className="flex flex-col gap-1 pt-2">
                  <span className="text-[10px] font-bold uppercase tracking-wider text-neutral-400">
                    Synthesized Detection Rules:
                  </span>
                  {synthesizedProfile.anomalyRules.map((rule, idx) => (
                    <div key={idx} className="flex items-center gap-1.5 text-neutral-300 text-[11px]">
                      <span className="text-[#F5C544]">▸</span> {rule}
                    </div>
                  ))}
                </div>
              )}
            </div>
          )}
        </div>

        {/* Footer Actions */}
        <div className="px-6 py-4 border-t border-white/10 flex justify-between items-center bg-[#161B24]">
          <button
            onClick={onClose}
            className="px-4 py-2 rounded-xl bg-white/5 hover:bg-white/10 text-neutral-400 hover:text-white font-bold transition-all cursor-pointer"
          >
            Cancel
          </button>

          <div className="flex items-center gap-3">
            {!synthesizedProfile ? (
              <button
                disabled={isSynthesizing}
                onClick={handleSynthesize}
                className="px-6 py-2.5 rounded-xl bg-gradient-to-r from-amber-500 to-yellow-500 hover:from-amber-400 hover:to-yellow-400 text-black font-extrabold shadow-lg transition-all cursor-pointer flex items-center gap-2 disabled:opacity-50"
              >
                <Sparkles className="w-4 h-4" />
                {isSynthesizing ? "AI Synthesizing Physics..." : "Generate AI Profile"}
              </button>
            ) : (
              <button
                onClick={handleDeployProfile}
                className="px-6 py-2.5 rounded-xl bg-emerald-500 hover:bg-emerald-400 text-black font-extrabold shadow-lg transition-all cursor-pointer flex items-center gap-2"
              >
                <CheckCircle2 className="w-4 h-4" />
                Deploy to Device Stethoscope
              </button>
            )}
          </div>
        </div>
      </div>
    </div>
  );
};
