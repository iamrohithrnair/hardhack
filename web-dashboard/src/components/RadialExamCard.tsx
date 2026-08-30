"use client";

import React, { useMemo } from "react";
import { ArrowUpRight, Play, Pause, RotateCcw, Activity } from "lucide-react";
import { TelemetryData } from "../hooks/useDeviceStream";

interface RadialExamCardProps {
  telemetry: TelemetryData;
  examSeconds: number;
  isExamRunning: boolean;
  onTogglePlay: () => void;
  onCalibrate: () => void;
  onToggleDemo: () => void;
}

export const RadialExamCard: React.FC<RadialExamCardProps> = ({
  telemetry,
  examSeconds,
  isExamRunning,
  onTogglePlay,
  onCalibrate,
  onToggleDemo
}) => {
  const isCritical = telemetry.score < 30;

  // Format mm:ss
  const mins = String(Math.floor(examSeconds / 60)).padStart(2, "0");
  const secs = String(examSeconds % 60).padStart(2, "0");

  // Radial Gauge Arc calculation
  const circumference = 452; // 2 * pi * 72
  const strokeDashoffset = circumference - (telemetry.score / 100) * circumference;

  // Generate 36 ticks around circle
  const ticks = useMemo(() => {
    const arr = [];
    const count = 36;
    for (let i = 0; i < count; i++) {
      const angle = (i / count) * 360;
      const rad = (angle * Math.PI) / 180;
      const r1 = 82;
      const r2 = i % 3 === 0 ? 88 : 85;
      const x1 = 100 + r1 * Math.cos(rad);
      const y1 = 100 + r1 * Math.sin(rad);
      const x2 = 100 + r2 * Math.cos(rad);
      const y2 = 100 + r2 * Math.sin(rad);
      arr.push({
        x1,
        y1,
        x2,
        y2,
        isMajor: i % 3 === 0
      });
    }
    return arr;
  }, []);

  return (
    <div className="bg-white rounded-[28px] p-6 border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col justify-between items-center">
      {/* Card Header */}
      <div className="w-full flex justify-between items-center mb-2">
        <span className="text-base font-bold text-[#12141A]">Stethoscope Exam</span>
        <button
          className="w-8 h-8 rounded-full border border-black/5 bg-[#FAFAFA] hover:bg-[#12141A] hover:text-white flex items-center justify-center text-[#6B7280] transition-all cursor-pointer"
          title="Inspect"
        >
          <ArrowUpRight className="w-4 h-4" />
        </button>
      </div>

      {/* SVG Radial Gauge */}
      <div className="relative w-44 h-44 my-2 flex items-center justify-center">
        <svg className="w-full h-full -rotate-90" viewBox="0 0 200 200">
          {/* Ticks */}
          <g>
            {ticks.map((t, idx) => (
              <line
                key={idx}
                x1={t.x1}
                y1={t.y1}
                x2={t.x2}
                y2={t.y2}
                stroke="#E5E7EB"
                strokeWidth={t.isMajor ? "1.5" : "1"}
              />
            ))}
          </g>

          {/* Background Track */}
          <circle
            cx="100"
            cy="100"
            r="72"
            fill="none"
            stroke="#F3F4F6"
            strokeWidth="10"
          />

          {/* Active Progress Arc */}
          <circle
            cx="100"
            cy="100"
            r="72"
            fill="none"
            stroke={isCritical ? "#F43F5E" : "#F5C544"}
            strokeWidth="10"
            strokeLinecap="round"
            strokeDasharray="452"
            style={{
              strokeDashoffset,
              transition: "stroke-dashoffset 0.5s ease, stroke 0.3s ease"
            }}
          />
        </svg>

        {/* Center Digital Stopwatch */}
        <div className="absolute flex flex-col items-center">
          <span className="text-2xl font-extrabold font-mono text-[#12141A]">
            {mins}:{secs}
          </span>
          <span className="text-[11px] text-[#6B7280] font-semibold">Exam Time</span>
        </div>
      </div>

      {/* Radial Control Buttons */}
      <div className="flex items-center gap-3">
        <button
          onClick={onTogglePlay}
          className={`w-11 h-11 rounded-full flex items-center justify-center transition-all cursor-pointer ${
            isExamRunning
              ? "bg-[#1C1F26] text-white shadow-xs"
              : "bg-[#F5C544] text-[#12141A] shadow-md"
          }`}
          title={isExamRunning ? "Pause Exam" : "Resume Exam"}
        >
          {isExamRunning ? <Pause className="w-4 h-4" /> : <Play className="w-4 h-4 ml-0.5" />}
        </button>

        <button
          onClick={onCalibrate}
          className="w-11 h-11 rounded-full border border-black/5 bg-[#FAFAFA] hover:bg-[#1C1F26] hover:text-white flex items-center justify-center text-[#12141A] transition-all cursor-pointer"
          title="Recalibrate Machine Baseline"
        >
          <RotateCcw className="w-4 h-4" />
        </button>

        <button
          onClick={onToggleDemo}
          className="w-11 h-11 rounded-full border border-black/5 bg-[#FAFAFA] hover:bg-amber-500 hover:text-white flex items-center justify-center text-[#12141A] transition-all cursor-pointer"
          title="Inject Fault / Toggle Demo"
        >
          <Activity className="w-4 h-4 text-amber-600" />
        </button>
      </div>
    </div>
  );
};
