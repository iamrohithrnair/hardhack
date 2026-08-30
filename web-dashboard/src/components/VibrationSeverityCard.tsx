"use client";

import React from "react";
import { ArrowUpRight } from "lucide-react";
import { TelemetryData } from "../hooks/useDeviceStream";

interface VibrationSeverityCardProps {
  telemetry: TelemetryData;
  onOpenReport?: () => void;
}

export const VibrationSeverityCard: React.FC<VibrationSeverityCardProps> = ({
  telemetry,
  onOpenReport
}) => {
  // Height percent of active bar
  const liveBarHeight = Math.min(95, Math.max(15, telemetry.rms * 60));

  return (
    <div className="bg-white rounded-[28px] p-6 border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col justify-between">
      {/* Card Header */}
      <div>
        <div className="flex justify-between items-center mb-4">
          <span className="text-base font-bold text-[#12141A]">Vibration Severity</span>
          <button
            onClick={onOpenReport}
            className="w-8 h-8 rounded-full border border-black/5 bg-[#FAFAFA] hover:bg-[#12141A] hover:text-white flex items-center justify-center text-[#6B7280] transition-all cursor-pointer"
            title="Inspect"
          >
            <ArrowUpRight className="w-4 h-4" />
          </button>
        </div>

        {/* Big Metric */}
        <div className="flex items-baseline gap-2 mb-6">
          <span className="text-4xl font-extrabold tracking-tight font-mono text-[#12141A]">
            {telemetry.rms.toFixed(2)}
          </span>
          <div className="flex flex-col text-[11px] text-[#6B7280] font-semibold leading-tight">
            <span>RMS Accel</span>
            <span>g-force</span>
          </div>
        </div>
      </div>

      {/* Vertical Session Bar Chart */}
      <div className="flex justify-between items-end h-40 pt-4 border-b border-neutral-100">
        {[
          { day: "S", height: "25%", active: false },
          { day: "M", height: "60%", active: false },
          { day: "T", height: "40%", active: false },
          { day: "W", height: "35%", active: false },
          { day: "T", height: `${liveBarHeight}%`, active: true, val: `${telemetry.rms.toFixed(2)}g` },
          { day: "F", height: "30%", active: false },
          { day: "S", height: "20%", active: false }
        ].map((item, idx) => (
          <div key={idx} className="flex flex-col items-center gap-2 h-full justify-end w-7 relative">
            {item.active && (
              <div className="absolute -top-6 left-1/2 -translate-x-1/2 bg-[#1C1F26] text-white text-[10px] font-bold px-2 py-0.5 rounded-md shadow-xs whitespace-nowrap">
                {item.val}
              </div>
            )}
            <div
              className={`w-2 rounded-full transition-all duration-300 ${
                item.active
                  ? "bg-[#F5C544] w-2.5 shadow-[0_0_8px_rgba(245,197,68,0.5)]"
                  : "bg-neutral-200"
              }`}
              style={{ height: item.height }}
            />
            <span className="text-[11px] font-semibold text-[#9CA3AF]">{item.day}</span>
          </div>
        ))}
      </div>
    </div>
  );
};
