"use client";

import React from "react";
import { Clock, Activity, Zap } from "lucide-react";
import { TelemetryData } from "../hooks/useDeviceStream";
import { MachineProfile } from "../types/machine";

interface HeroSectionProps {
  telemetry: TelemetryData;
  machine: MachineProfile;
}

export const HeroSection: React.FC<HeroSectionProps> = ({ telemetry, machine }) => {
  const isHealthy = telemetry.score >= 70;
  const isCritical = telemetry.score < 30;

  // Real or machine-configured RPM
  const displayRPM = telemetry.rpm > 300 ? telemetry.rpm : machine.nominalRPM;
  const displayFreq = telemetry.f0 > 5 ? telemetry.f0 : machine.fundamentalHz;

  return (
    <section className="flex flex-col lg:flex-row justify-between items-start lg:items-end gap-6 mt-1">
      {/* Left Greeting & Segmented Health Pill */}
      <div className="flex flex-col gap-4 flex-1">
        <h1 className="text-3xl font-extrabold tracking-tight text-[#12141A]">
          Diagnostic Center,{" "}
          <span className="text-[#6B7280] font-semibold">{machine.name}</span>
        </h1>

        <div className="flex flex-wrap items-center gap-2 bg-black/[0.04] p-1.5 rounded-full max-w-2xl border border-black/5">
          {/* Health Segment */}
          <div
            className={`flex items-center justify-between px-3.5 py-1.5 rounded-full text-[11px] font-semibold transition-all ${
              isCritical
                ? "bg-rose-600 text-white"
                : isHealthy
                ? "bg-[#1C1F26] text-white"
                : "bg-amber-500 text-white"
            }`}
            style={{ minWidth: "120px" }}
          >
            <span>Health</span>
            <span className="font-bold">{telemetry.score}%</span>
          </div>

          {/* Balance Segment */}
          <div
            className="flex items-center justify-between px-3.5 py-1.5 rounded-full text-[11px] font-semibold bg-[#F5C544] text-[#12141A]"
            style={{ minWidth: "110px" }}
          >
            <span>Balance</span>
            <span className="font-bold">
              {telemetry.state === 3 ? "18%" : "96%"}
            </span>
          </div>

          {/* ISO 10816 Segment */}
          <div
            className="flex items-center justify-between px-3.5 py-1.5 rounded-full text-[11px] font-semibold bg-white text-[#6B7280] border border-black/5 shadow-xs"
            style={{ minWidth: "120px" }}
          >
            <span>{machine.isoClass}</span>
            <span
              className={`font-bold ${
                telemetry.iso < 1.12
                  ? "text-emerald-600"
                  : telemetry.iso < 2.8
                  ? "text-amber-600"
                  : "text-rose-600"
              }`}
            >
              {telemetry.iso < 1.12 ? "Class A" : telemetry.iso < 2.8 ? "Class B" : "Class D"}
            </span>
          </div>

          {/* Kurtosis Segment */}
          <div
            className="flex items-center justify-between px-3.5 py-1.5 rounded-full text-[11px] font-semibold bg-white/60 text-[#9CA3AF]"
            style={{ minWidth: "100px" }}
          >
            <span>Kurtosis</span>
            <span
              className={`font-bold ${
                telemetry.kurt > 4.0 ? "text-amber-600" : "text-[#12141A]"
              }`}
            >
              {telemetry.kurt.toFixed(1)}
            </span>
          </div>
        </div>
      </div>

      {/* Right Rolling Stat Numbers */}
      <div className="flex items-center gap-6 sm:gap-8">
        {/* RPM */}
        <div className="flex items-center gap-3">
          <div className="w-11 h-11 rounded-full bg-white/80 border border-black/5 flex items-center justify-center text-[#12141A] shadow-xs">
            <Clock className="w-4 h-4" />
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
