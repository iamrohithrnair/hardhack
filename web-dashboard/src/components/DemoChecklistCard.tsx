"use client";

import React from "react";
import { Check, Activity, ShieldCheck, AlertTriangle } from "lucide-react";
import { TelemetryData } from "../hooks/useDeviceStream";

interface DemoChecklistCardProps {
  telemetry: TelemetryData;
  onSelectMode: (mode: "healthy" | "unbalance" | "bearing") => void;
}

export const DemoChecklistCard: React.FC<DemoChecklistCardProps> = ({
  telemetry,
  onSelectMode
}) => {
  const isFault = telemetry.score < 50;

  return (
    <div className="bg-white rounded-[28px] p-6 border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col justify-between">
      {/* Header */}
      <div>
        <div className="flex justify-between items-center mb-3">
          <span className="text-base font-bold text-[#12141A]">Diagnostic Health</span>
          <span className="text-xl font-extrabold font-mono text-[#12141A]">
            {telemetry.score}%
          </span>
        </div>

        {/* Segmented Selector Tabs */}
        <div className="flex gap-1.5 bg-[#F3F4F6] p-1 rounded-full mb-4">
          <button
            onClick={() => onSelectMode("healthy")}
            className={`flex-1 py-1.5 rounded-full text-[11px] font-semibold transition-all cursor-pointer ${
              telemetry.state === 1
                ? "bg-[#F5C544] text-[#12141A] font-bold shadow-xs"
                : "text-[#6B7280] hover:text-[#12141A]"
            }`}
          >
            Nominal
          </button>
          <button
            onClick={() => onSelectMode("unbalance")}
            className={`flex-1 py-1.5 rounded-full text-[11px] font-semibold transition-all cursor-pointer ${
              telemetry.state === 3
                ? "bg-rose-500 text-white font-bold shadow-xs"
                : "text-[#6B7280] hover:text-[#12141A]"
            }`}
          >
            Imbalance
          </button>
          <button
            onClick={() => onSelectMode("bearing")}
            className={`flex-1 py-1.5 rounded-full text-[11px] font-semibold transition-all cursor-pointer ${
              telemetry.state === 4
                ? "bg-amber-500 text-white font-bold shadow-xs"
                : "text-[#6B7280] hover:text-[#12141A]"
            }`}
          >
            Bearing
          </button>
        </div>
      </div>

      {/* Sleek Dark Task Checklist Card (#1C1F26) */}
      <div className="bg-[#1C1F26] text-white rounded-[22px] p-4 flex flex-col gap-3">
        <div className="flex justify-between items-center pb-2 border-b border-white/10">
          <span className="text-xs font-bold tracking-tight">1-Min Pitch Exam</span>
          <span className="text-xs font-extrabold font-mono text-[#F5C544]">
            {isFault ? "5/5" : "4/5"}
          </span>
        </div>

        <div className="flex flex-col gap-2">
          {/* Step 1 */}
          <div className="flex items-center gap-2.5 p-2 rounded-xl bg-[#282B34] text-xs">
            <div className="w-6 h-6 rounded-full bg-white/10 flex items-center justify-center text-white">
              <ShieldCheck className="w-3.5 h-3.5" />
            </div>
            <div className="flex-1">
              <div className="font-semibold text-[11px]">1. Solid-State Contact</div>
              <div className="text-[10px] text-[#9CA3AF]">IMU pressed firmly to fan base</div>
            </div>
            <div className="text-[#F5C544]">
              <Check className="w-3.5 h-3.5" />
            </div>
          </div>

          {/* Step 2 */}
          <div className="flex items-center gap-2.5 p-2 rounded-xl bg-[#282B34] text-xs">
            <div className="w-6 h-6 rounded-full bg-white/10 flex items-center justify-center text-white">
              <Activity className="w-3.5 h-3.5" />
            </div>
            <div className="flex-1">
              <div className="font-semibold text-[11px]">2. 1X RPM Fundamental Lock</div>
              <div className="text-[10px] text-[#9CA3AF]">F0: 48.5 Hz (2,910 RPM) verified</div>
            </div>
            <div className="text-[#F5C544]">
              <Check className="w-3.5 h-3.5" />
            </div>
          </div>

          {/* Step 3 */}
          <div className="flex items-center gap-2.5 p-2 rounded-xl bg-[#282B34] text-xs">
            <div className="w-6 h-6 rounded-full bg-white/10 flex items-center justify-center text-white">
              <AlertTriangle className="w-3.5 h-3.5 text-amber-400" />
            </div>
            <div className="flex-1">
              <div className="font-semibold text-[11px]">3. Rotor Imbalance (Blu-Tack)</div>
              <div className="text-[10px] text-[#9CA3AF]">
                {isFault ? "Mass Unbalance (+18dB 1X Spike)" : "Attach tape to blade for anomaly"}
              </div>
            </div>
            <div>
              {isFault ? (
                <div className="text-rose-400">
                  <Check className="w-3.5 h-3.5" />
                </div>
              ) : (
                <div className="w-3.5 h-3.5 rounded-full border-2 border-neutral-500" />
              )}
            </div>
          </div>

          {/* Step 4 */}
          <div className="flex items-center gap-2.5 p-2 rounded-xl bg-[#282B34] text-xs">
            <div className="w-6 h-6 rounded-full bg-white/10 flex items-center justify-center text-white">
              <ShieldCheck className="w-3.5 h-3.5" />
            </div>
            <div className="flex-1">
              <div className="font-semibold text-[11px]">4. ISO 10816 Classification</div>
              <div className="text-[10px] text-[#9CA3AF]">Class A (&lt; 1.12 mm/s) Optimal</div>
            </div>
            <div className="text-[#F5C544]">
              <Check className="w-3.5 h-3.5" />
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
