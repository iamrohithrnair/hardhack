"use client";

import React, { useState, useEffect, useRef } from "react";
import { TelemetryData } from "../hooks/useDeviceStream";
import { 
  Activity, 
  BarChart3, 
  Gauge, 
  Disc3, 
  Flame, 
  Radio, 
  Sparkles, 
  Waves, 
  Sliders, 
  Cpu
} from "lucide-react";

interface GraphGalleryStudioProps {
  telemetry: TelemetryData;
}

export const GraphGalleryStudio: React.FC<GraphGalleryStudioProps> = ({ telemetry }) => {
  const [selectedStyle, setSelectedStyle] = useState<number>(0);
  const canvasECGRef = useRef<HTMLCanvasElement | null>(null);
  const canvasWaveRef = useRef<HTMLCanvasElement | null>(null);
  const phaseRef = useRef<number>(0);

  const isFault = telemetry.score < 50;

  // Real-time canvas rendering for ECG pulse and Fluid Waves
  useEffect(() => {
    let animId: number;

    const draw = () => {
      phaseRef.current += 0.08;

      // 1. Draw Neon ECG Line (Chart 8 style)
      if (canvasECGRef.current) {
        const cvs = canvasECGRef.current;
        const ctx = cvs.getContext("2d");
        if (ctx) {
          ctx.fillStyle = "#12141A";
          ctx.fillRect(0, 0, cvs.width, cvs.height);

          // Grid lines
          ctx.strokeStyle = "#1F2636";
          ctx.lineWidth = 1;
          for (let x = 0; x < cvs.width; x += 30) {
            ctx.beginPath();
            ctx.moveTo(x, 0);
            ctx.lineTo(x, cvs.height);
            ctx.stroke();
          }

          // Cardiac / Acoustic trace
          ctx.strokeStyle = isFault ? "#F43F5E" : "#FF2E7E";
          ctx.lineWidth = 3;
          ctx.shadowColor = isFault ? "#F43F5E" : "#FF2E7E";
          ctx.shadowBlur = 12;
          ctx.beginPath();

          const midY = cvs.height / 2;
          for (let x = 0; x < cvs.width; x++) {
            const t = (x / 40) - phaseRef.current * 2;
            const beat = Math.sin(t) * Math.exp(-Math.pow((t % (Math.PI * 2)) - Math.PI, 2) * 4);
            const spike = Math.sin(t * 3) * (telemetry.rms * 50);
            const y = midY - beat * 40 - spike;

            if (x === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
          }
          ctx.stroke();
          ctx.shadowBlur = 0;
        }
      }

      // 2. Draw Fluid Wave Tank (Chart 7 style)
      if (canvasWaveRef.current) {
        const cvs = canvasWaveRef.current;
        const ctx = cvs.getContext("2d");
        if (ctx) {
          ctx.fillStyle = "#12141A";
          ctx.fillRect(0, 0, cvs.width, cvs.height);

          const fillHeight = Math.min(cvs.height * 0.85, Math.max(cvs.height * 0.2, (telemetry.rms * 300) + 40));
          const baseWaterY = cvs.height - fillHeight;

          // Wave 1 (Deep Blue)
          ctx.fillStyle = "rgba(14, 165, 233, 0.4)";
          ctx.beginPath();
          ctx.moveTo(0, cvs.height);
          for (let x = 0; x <= cvs.width; x += 4) {
            const y = baseWaterY + Math.sin(x * 0.03 + phaseRef.current) * 8;
            ctx.lineTo(x, y);
          }
          ctx.lineTo(cvs.width, cvs.height);
          ctx.closePath();
          ctx.fill();

          // Wave 2 (Cyan Crest)
          ctx.fillStyle = "rgba(56, 189, 248, 0.75)";
          ctx.beginPath();
          ctx.moveTo(0, cvs.height);
          for (let x = 0; x <= cvs.width; x += 4) {
            const y = baseWaterY + Math.sin(x * 0.04 - phaseRef.current * 1.5) * 6;
            ctx.lineTo(x, y);
          }
          ctx.lineTo(cvs.width, cvs.height);
          ctx.closePath();
          ctx.fill();
        }
      }

      animId = requestAnimationFrame(draw);
    };

    draw();
    return () => cancelAnimationFrame(animId);
  }, [isFault, telemetry.rms]);

  return (
    <div className="bg-white rounded-[32px] p-6 lg:p-8 border border-black/5 shadow-[0_16px_40px_rgba(0,0,0,0.03)] flex flex-col gap-6">
      {/* Header Row */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 border-b border-neutral-100 pb-5">
        <div>
          <div className="flex items-center gap-2.5">
            <span className="w-3 h-3 rounded-full bg-[#F5C544] shadow-[0_0_10px_rgba(245,197,68,0.8)]" />
            <h2 className="text-xl font-bold text-[#12141A] tracking-tight">
              Sensor Chart Studio (10 Real-Time Graph Styles)
            </h2>
          </div>
          <p className="text-xs text-[#6B7280] font-medium mt-1">
            Physical device AMOLED screen and web dashboard mirror all 10 chart styles switchable via hardware BOOT switch.
          </p>
        </div>

        <div className="flex items-center gap-2 bg-[#F6F4ED] p-1.5 rounded-2xl border border-black/5">
          <span className="text-[11px] font-bold text-[#8C6B10] px-2 font-mono">
            PRESS ESP32 BOOT SWITCH TO CYCLE
          </span>
        </div>
      </div>

      {/* 10-Chart Interactive Style Grid */}
      <div className="grid grid-cols-2 sm:grid-cols-5 gap-3">
        {[
          { id: 0, title: "1. Goal Pillars", icon: BarChart3, color: "text-amber-500" },
          { id: 1, title: "2. Dot Matrix", icon: Cpu, color: "text-emerald-500" },
          { id: 2, title: "3. Segmented VU", icon: Sliders, color: "text-rose-500" },
          { id: 3, title: "4. Triple Rings", icon: Disc3, color: "text-sky-500" },
          { id: 4, title: "5. Progress Arc", icon: Gauge, color: "text-cyan-500" },
          { id: 5, title: "6. Tachometer", icon: Flame, color: "text-orange-500" },
          { id: 6, title: "7. Fluid Waves", icon: Waves, color: "text-blue-500" },
          { id: 7, title: "8. ECG Pulse", icon: Activity, color: "text-pink-500" },
          { id: 8, title: "9. Dual Spline", icon: Radio, color: "text-purple-500" },
          { id: 9, title: "10. Bold Metric", icon: Sparkles, color: "text-emerald-600" },
        ].map((item) => {
          const Icon = item.icon;
          const isSel = selectedStyle === item.id;
          return (
            <button
              key={item.id}
              onClick={() => setSelectedStyle(item.id)}
              className={`p-3.5 rounded-2xl flex flex-col items-center gap-2 text-center transition-all cursor-pointer border ${
                isSel
                  ? "bg-[#1C1F26] text-white border-[#1C1F26] shadow-md scale-102"
                  : "bg-[#FAFAFA] hover:bg-neutral-100 text-[#4B5563] border-black/5"
              }`}
            >
              <Icon className={`w-5 h-5 ${isSel ? "text-[#F5C544]" : item.color}`} />
              <span className="text-[11px] font-bold tracking-tight">{item.title}</span>
            </button>
          );
        })}
      </div>

      {/* Main Interactive Graph Showcase Container */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6 pt-2">
        {/* Card 1: Neon ECG Pulse Line (Chart 8 style) */}
        <div className="bg-[#12141A] rounded-[28px] p-6 border border-white/10 shadow-xl flex flex-col justify-between relative overflow-hidden">
          <div className="flex justify-between items-center mb-4">
            <span className="text-xs font-bold text-neutral-400 uppercase tracking-wider">
              Acoustic Stethoscope Pulse
            </span>
            <span className="px-2.5 py-0.5 rounded-full text-[10px] font-mono bg-pink-500/20 text-pink-400 font-bold">
              MIC TRANSDUCER
            </span>
          </div>

          <div className="relative w-full h-36 rounded-xl overflow-hidden bg-[#0A0D14] border border-white/5">
            <canvas ref={canvasECGRef} width={340} height={144} className="w-full h-full object-cover" />
          </div>

          <div className="flex justify-between items-baseline mt-4 pt-3 border-t border-white/5">
            <div className="flex items-baseline gap-2">
              <span className="text-3xl font-black font-mono text-white">{(telemetry.f0 || 48.5).toFixed(1)}</span>
              <span className="text-xs font-semibold text-neutral-400">Hz Fundamental</span>
            </div>
            <span className="text-xs font-bold text-pink-400 font-mono">
              {telemetry.state === 1 ? "65 BPM NOMINAL" : "128 BPM IMPACT"}
            </span>
          </div>
        </div>

        {/* Card 2: Triple Activity Rings (Chart 4 style) */}
        <div className="bg-[#12141A] rounded-[28px] p-6 border border-white/10 shadow-xl flex flex-col items-center justify-between relative">
          <div className="w-full flex justify-between items-center mb-2">
            <span className="text-xs font-bold text-neutral-400 uppercase tracking-wider">
              Multi-Ring Diagnostics
            </span>
            <span className="px-2.5 py-0.5 rounded-full text-[10px] font-mono bg-sky-500/20 text-sky-400 font-bold">
              TRIPLE RING
            </span>
          </div>

          {/* SVG Concentric Rings */}
          <div className="relative w-44 h-44 flex items-center justify-center my-2">
            <svg className="w-full h-full -rotate-90" viewBox="0 0 160 160">
              {/* Ring 1: Health (Pink) */}
              <circle cx="80" cy="80" r="70" stroke="#3B1828" strokeWidth="10" fill="none" />
              <circle
                cx="80"
                cy="80"
                r="70"
                stroke="#FA2C56"
                strokeWidth="10"
                fill="none"
                strokeDasharray="440"
                strokeDashoffset={440 - (440 * telemetry.score) / 100}
                strokeLinecap="round"
                className="transition-all duration-700"
              />
              {/* Ring 2: Bearing (Cyan) */}
              <circle cx="80" cy="80" r="54" stroke="#102844" strokeWidth="10" fill="none" />
              <circle
                cx="80"
                cy="80"
                r="54"
                stroke="#00F0FF"
                strokeWidth="10"
                fill="none"
                strokeDasharray="340"
                strokeDashoffset={340 - (340 * (telemetry.kurt > 4 ? 45 : 94)) / 100}
                strokeLinecap="round"
                className="transition-all duration-700"
              />
              {/* Ring 3: Balance (Yellow) */}
              <circle cx="80" cy="80" r="38" stroke="#3B3210" strokeWidth="10" fill="none" />
              <circle
                cx="80"
                cy="80"
                r="38"
                stroke="#F5C544"
                strokeWidth="10"
                fill="none"
                strokeDasharray="240"
                strokeDashoffset={240 - (240 * (telemetry.rms > 0.2 ? 35 : 92)) / 100}
                strokeLinecap="round"
                className="transition-all duration-700"
              />
            </svg>
            <div className="absolute flex flex-col items-center">
              <span className="text-2xl font-black font-mono text-white">{telemetry.score}%</span>
              <span className="text-[10px] font-bold text-neutral-400">HEALTH</span>
            </div>
          </div>

          <div className="w-full flex justify-around text-center pt-2 border-t border-white/5 text-[11px] font-bold">
            <span className="text-[#FA2C56]">HEALTH: {telemetry.score}%</span>
            <span className="text-[#00F0FF]">BEARING: {telemetry.kurt > 4 ? "FAULT" : "GOOD"}</span>
            <span className="text-[#F5C544]">BALANCE: {telemetry.rms.toFixed(2)}g</span>
          </div>
        </div>

        {/* Card 3: Fluid Wave Tank & Segmented VU Meter (Chart 7 & Chart 3) */}
        <div className="bg-[#12141A] rounded-[28px] p-6 border border-white/10 shadow-xl flex flex-col justify-between relative overflow-hidden">
          <div className="flex justify-between items-center mb-3">
            <span className="text-xs font-bold text-neutral-400 uppercase tracking-wider">
              Fluid Severity & VU Meter
            </span>
            <span className="px-2.5 py-0.5 rounded-full text-[10px] font-mono bg-blue-500/20 text-blue-400 font-bold">
              FLUID TRANSDUCER
            </span>
          </div>

          {/* Fluid Canvas */}
          <div className="relative w-full h-32 rounded-xl overflow-hidden bg-[#0A0D14] border border-white/5 mb-3">
            <canvas ref={canvasWaveRef} width={340} height={130} className="w-full h-full object-cover" />
            <div className="absolute inset-0 flex items-center justify-center pointer-events-none">
              <div className="bg-black/60 backdrop-blur-xs px-3 py-1 rounded-lg text-white font-mono font-bold text-sm">
                ISO VEL: {telemetry.iso.toFixed(2)} mm/s
              </div>
            </div>
          </div>

          {/* 16-Band Segmented LED Bar */}
          <div className="flex justify-between items-end gap-1.5 h-12 bg-white/5 p-2 rounded-xl border border-white/5">
            {Array.from({ length: 16 }).map((_, i) => {
              const active = i < Math.round(telemetry.rms * 60 + 3);
              const color = i > 12 ? "bg-rose-500" : i > 8 ? "bg-amber-500" : "bg-[#F5C544]";
              return (
                <div
                  key={i}
                  className={`flex-1 rounded-sm transition-all duration-150 ${
                    active ? color : "bg-neutral-800"
                  }`}
                  style={{ height: `${Math.max(20, (i + 1) * 6)}%` }}
                />
              );
            })}
          </div>
        </div>
      </div>
    </div>
  );
};
