"use client";

import React from "react";
import { Clock, Activity, Zap, Sparkles, Settings2 } from "lucide-react";
import { TelemetryData } from "../hooks/useDeviceStream";
import { MachineProfile } from "../types/machine";

interface HeroSectionProps {
  telemetry: TelemetryData;
  machine: MachineProfile;
  onOpenCustomizer?: () => void;
  onOpenProfiler?: () => void;
}

export const HeroSection: React.FC<HeroSectionProps> = ({
  telemetry,
  machine,
  onOpenCustomizer,
  onOpenProfiler
}) => {
  const isHealthy = telemetry.score >= 70;
  const isCritical = telemetry.score < 30;

  // Real or machine-configured RPM
  const displayRPM = telemetry.rpm > 300 ? telemetry.rpm : machine.nominalRPM;
  const displayFreq = telemetry.f0 > 5 ? telemetry.f0 : machine.fundamentalHz;

  return (
    <section className="flex flex-col lg:flex-row justify-between items-start lg:items-end gap-6 mt-1">
      {/* Left Greeting & Segmented Health Pill */}
      <div className="flex flex-col gap-4 flex-1">
        <div className="flex flex-wrap items-center gap-3">
          <h1 className="text-3xl font-extrabold tracking-tight text-[#12141A]">
            Diagnostic Center,{" "}
            <span className="text-[#6B7280] font-semibold">{machine.name}</span>
          </h1>

          {onOpenCustomizer && (
            <button
              onClick={onOpenCustomizer}
              className="px-3 py-1 rounded-full bg-white/80 hover:bg-neutral-100 border border-black/5 text-xs font-bold text-[#12141A] transition-all cursor-pointer flex items-center gap-1.5 shadow-xs"
            >
              <Settings2 className="w-3.5 h-3.5" />
              Switch Target
            </button>
          )}

          {onOpenProfiler && (
            <button
              onClick={onOpenProfiler}
              className="px-3 py-1 rounded-full bg-gradient-to-r from-amber-500 to-yellow-500 hover:from-amber-400 hover:to-yellow-400 text-black text-xs font-extrabold transition-all cursor-pointer flex items-center gap-1.5 shadow-xs"
            >
              <Sparkles className="w-3.5 h-3.5" />
              + AI Profile Custom Event
            </button>
          )}
        </div>

        <div className="flex flex-wrap items-center gap-2 bg-black/[0.04] p-1.5 rounded-full max-w-2xl border border-black/5">
          {/* Active Status Badge */}
          <div
            className={`px-4 py-1.5 rounded-full text-xs font-bold text-white flex items-center gap-2 shadow-xs ${
              isCritical
                ? "bg-rose-500"
                : !isHealthy
                ? "bg-amber-500"
                : "bg-[#1C1F26]"
            }`}
          >
            <span className="w-2 h-2 rounded-full bg-white animate-pulse" />
            {isCritical
              ? "Critical Imbalance"
              : !isHealthy
              ? "Warning Anomaly"
              : "Nominal Harmonic"}
          </div>

          {/* Health Score Pill */}
          <div className="px-4 py-1.5 rounded-full bg-white border border-black/5 text-xs font-extrabold text-[#12141A] shadow-xs flex items-center gap-1.5">
            <span className="text-[#F5C544]">●</span>
            Score: {telemetry.score}%
          </div>

          {/* ISO Standard Pill */}
          <div className="px-4 py-1.5 rounded-full bg-white/80 text-xs font-bold text-[#6B7280]">
            {machine.isoClass}
          </div>

          {/* Target Type Pill */}
          <div className="px-3 py-1 rounded-full bg-amber-500/10 text-amber-700 font-mono text-[11px] font-bold">
            {machine.detectionTarget?.toUpperCase() || "VIBRATION"}
          </div>
        </div>
      </div>

      {/* Right Real-time Dynamic Metric Pill Counters */}
      <div className="flex items-center gap-6 sm:gap-8 bg-white/60 backdrop-blur-md p-4 rounded-[28px] border border-black/5 shadow-[0_8px_24px_rgba(0,0,0,0.02)]">
        {/* Nominal RPM */}
        <div className="flex items-center gap-3">
          <div className="w-11 h-11 rounded-full bg-white/80 border border-black/5 flex items-center justify-center text-[#12141A] shadow-xs">
            <Clock className="w-4 h-4 text-emerald-500" />
          </div>
          <div className="flex flex-col">
            <span className="text-2xl font-extrabold tracking-tight font-mono text-[#12141A]">
              {displayRPM.toLocaleString()}
            </span>
            <span className="text-[11px] text-[#6B7280] font-semibold">
              Rotor RPM
            </span>
          </div>
        </div>

        {/* Fundamental Frequency */}
        <div className="flex items-center gap-3">
          <div className="w-11 h-11 rounded-full bg-white/80 border border-black/5 flex items-center justify-center text-[#12141A] shadow-xs">
            <Activity className="w-4 h-4 text-sky-500" />
          </div>
          <div className="flex flex-col">
            <span className="text-2xl font-extrabold tracking-tight font-mono text-[#12141A]">
              {displayFreq.toFixed(1)}
            </span>
            <span className="text-[11px] text-[#6B7280] font-semibold">
              Freq (Hz)
            </span>
          </div>
        </div>

        {/* RMS Acceleration */}
        <div className="flex items-center gap-3">
          <div className="w-11 h-11 rounded-full bg-white/80 border border-black/5 flex items-center justify-center text-[#12141A] shadow-xs">
            <Zap className="w-4 h-4 text-amber-500" />
          </div>
          <div className="flex flex-col">
            <span className="text-2xl font-extrabold tracking-tight font-mono text-[#12141A]">
              {telemetry.rms.toFixed(3)}
            </span>
            <span className="text-[11px] text-[#6B7280] font-semibold">
              RMS (g)
            </span>
          </div>
        </div>
      </div>
    </section>
  );
};
