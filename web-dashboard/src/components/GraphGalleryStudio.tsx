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
  Cpu,
  CheckCircle2,
  AlertTriangle
} from "lucide-react";

interface GraphGalleryStudioProps {
  telemetry: TelemetryData;
}

export const GraphGalleryStudio: React.FC<GraphGalleryStudioProps> = ({ telemetry }) => {
  const [selectedStyle, setSelectedStyle] = useState<number>(0);
  const canvasECGRef = useRef<HTMLCanvasElement | null>(null);
  const canvasWaveRef = useRef<HTMLCanvasElement | null>(null);
  const canvasSplineRef = useRef<HTMLCanvasElement | null>(null);
  const phaseRef = useRef<number>(0);

  const isFault = telemetry.score < 50;

  // Real-time canvas animation loop
  useEffect(() => {
    let animId: number;

    const draw = () => {
      phaseRef.current += 0.08;

      // 1. Draw Neon ECG Pulse (Chart 8)
      if (canvasECGRef.current) {
        const cvs = canvasECGRef.current;
        const ctx = cvs.getContext("2d");
        if (ctx) {
          ctx.fillStyle = "#0C0E14";
          ctx.fillRect(0, 0, cvs.width, cvs.height);

          // Grid lines
          ctx.strokeStyle = "rgba(255, 255, 255, 0.04)";
          ctx.lineWidth = 1;
          for (let x = 0; x < cvs.width; x += 30) {
            ctx.beginPath();
            ctx.moveTo(x, 0);
            ctx.lineTo(x, cvs.height);
            ctx.stroke();
          }

          // ECG / Acoustic trace
          ctx.strokeStyle = isFault ? "#FF2A54" : "#FF2E7E";
          ctx.lineWidth = 3;
          ctx.shadowColor = isFault ? "#FF2A54" : "#FF2E7E";
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

      // 2. Draw Fluid Wave Tank (Chart 7)
      if (canvasWaveRef.current) {
        const cvs = canvasWaveRef.current;
        const ctx = cvs.getContext("2d");
        if (ctx) {
          ctx.fillStyle = "#0C0E14";
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
          ctx.fillStyle = "rgba(0, 240, 255, 0.75)";
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

      // 3. Draw Dual Spline Envelope Curve (Chart 9)
      if (canvasSplineRef.current) {
        const cvs = canvasSplineRef.current;
        const ctx = cvs.getContext("2d");
        if (ctx) {
          ctx.fillStyle = "#0C0E14";
          ctx.fillRect(0, 0, cvs.width, cvs.height);

          const midY = cvs.height / 2;
          const amp = Math.max(10, Math.min(50, telemetry.rms * 120));

          // Spline 1: Max Envelope (Red / Coral)
          ctx.strokeStyle = "#FF2A54";
          ctx.lineWidth = 2.5;
          ctx.shadowColor = "rgba(255, 42, 84, 0.6)";
          ctx.shadowBlur = 8;
          ctx.beginPath();
          for (let x = 0; x <= cvs.width; x += 4) {
            const y = midY - amp * (0.8 + 0.2 * Math.sin(x * 0.04 + phaseRef.current));
            if (x === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
          }
          ctx.stroke();

          // Spline 2: Min Envelope (Cyan)
          ctx.strokeStyle = "#00F0FF";
          ctx.lineWidth = 2.5;
          ctx.shadowColor = "rgba(0, 240, 255, 0.6)";
          ctx.shadowBlur = 8;
          ctx.beginPath();
          for (let x = 0; x <= cvs.width; x += 4) {
            const y = midY + amp * (0.75 + 0.25 * Math.sin(x * 0.05 - phaseRef.current));
            if (x === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
          }
          ctx.stroke();
          ctx.shadowBlur = 0;
        }
      }

      animId = requestAnimationFrame(draw);
    };

    draw();
    return () => cancelAnimationFrame(animId);
  }, [isFault, telemetry.rms]);

  const STYLES_LIST = [
    { id: 0, title: "1. Goal Pillars", icon: BarChart3, color: "text-amber-400" },
    { id: 1, title: "2. Dot Matrix", icon: Cpu, color: "text-emerald-400" },
    { id: 2, title: "3. Segmented VU", icon: Sliders, color: "text-rose-400" },
    { id: 3, title: "4. Triple Rings", icon: Disc3, color: "text-sky-400" },
    { id: 4, title: "5. Progress Arc", icon: Gauge, color: "text-cyan-400" },
    { id: 5, title: "6. Tachometer", icon: Flame, color: "text-orange-400" },
    { id: 6, title: "7. Fluid Waves", icon: Waves, color: "text-blue-400" },
    { id: 7, title: "8. ECG Pulse", icon: Activity, color: "text-pink-400" },
    { id: 8, title: "9. Dual Spline", icon: Radio, color: "text-purple-400" },
    { id: 9, title: "10. Bold Metric", icon: Sparkles, color: "text-emerald-400" },
  ];

  return (
    <div className="bg-[#12141A] text-white rounded-[32px] p-6 lg:p-8 border border-white/10 shadow-[0_24px_48px_rgba(0,0,0,0.25)] flex flex-col gap-6">
      {/* Header Row */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 border-b border-white/10 pb-5">
        <div>
          <div className="flex items-center gap-2.5">
            <span className="w-3 h-3 rounded-full bg-[#F5C544] shadow-[0_0_10px_rgba(245,197,68,0.8)]" />
            <h2 className="text-xl font-bold text-white tracking-tight">
              Sensor Chart Studio (10 Real-Time Diagnostic Styles)
            </h2>
          </div>
          <p className="text-xs text-neutral-400 font-medium mt-1">
            Inspired by Apple Watch luxury dark aesthetics. All graphs mirror live ESP32-S3 sensor streams and can be cycled on hardware via the BOOT switch.
          </p>
        </div>

        <div className="flex items-center gap-2 bg-[#1C1F26] p-1.5 rounded-2xl border border-white/5">
          <span className="text-[11px] font-bold text-[#F5C544] px-2 font-mono">
            PRESS ESP32 BOOT SWITCH TO CYCLE VIEWS
          </span>
        </div>
      </div>

      {/* 10-Chart Interactive Navigation Grid */}
      <div className="grid grid-cols-2 sm:grid-cols-5 gap-3">
        {STYLES_LIST.map((item) => {
          const Icon = item.icon;
          const isSel = selectedStyle === item.id;
          return (
            <button
              key={item.id}
              onClick={() => setSelectedStyle(item.id)}
              className={`p-3.5 rounded-2xl flex flex-col items-center gap-2 text-center transition-all cursor-pointer border ${
                isSel
                  ? "bg-[#1E2330] text-white border-[#F5C544] shadow-lg ring-1 ring-[#F5C544] scale-102"
                  : "bg-[#161B24] hover:bg-[#1C212C] text-neutral-400 border-white/5"
              }`}
            >
              <Icon className={`w-5 h-5 ${isSel ? "text-[#F5C544]" : item.color}`} />
              <span className="text-[11px] font-bold tracking-tight">{item.title}</span>
            </button>
          );
        })}
      </div>

      {/* Interactive Detail Card for Selected Style */}
      <div className="p-6 rounded-[28px] bg-[#161B24] border border-white/10 flex flex-col gap-4">
        {/* Style 0: Goal Pillars */}
        {selectedStyle === 0 && (
          <div className="flex flex-col gap-4 animate-fade-in">
            <div className="flex justify-between items-center">
              <span className="text-xs font-bold text-[#F5C544] uppercase tracking-wider font-mono">
                1. Harmonic Energy Goal Pillars (1X - 7X Fundamental Orders)
              </span>
              <span className="text-xs font-mono text-neutral-400">Target Line: 0.25g ISO Limit</span>
            </div>
            <div className="flex justify-around items-end h-44 bg-[#0C0E14] p-4 rounded-2xl border border-white/5 relative">
              {/* Threshold guideline */}
              <div className="absolute top-14 left-0 right-0 border-b border-dashed border-rose-500/60 pointer-events-none" />
              {[
                { order: "1X", val: Math.min(100, (telemetry.rms * 180)), color: "bg-[#F5C544]" },
                { order: "2X", val: Math.min(100, (telemetry.rms * 90) + 15), color: "bg-orange-500" },
                { order: "3X", val: Math.min(100, (telemetry.rms * 60) + 10), color: "bg-amber-500" },
                { order: "4X", val: Math.min(100, (telemetry.rms * 45) + 8), color: "bg-yellow-600" },
                { order: "5X", val: Math.min(100, (telemetry.rms * 30) + 6), color: "bg-emerald-500" },
                { order: "6X", val: Math.min(100, (telemetry.rms * 25) + 5), color: "bg-cyan-500" },
                { order: "7X", val: Math.min(100, (telemetry.rms * 20) + 4), color: "bg-sky-500" }
              ].map((p, idx) => (
                <div key={idx} className="flex flex-col items-center gap-2 h-full justify-end">
                  <span className="text-[10px] font-mono text-neutral-400">{p.val.toFixed(0)}%</span>
                  <div
                    className={`w-8 rounded-full ${p.color} transition-all duration-300 shadow-md`}
                    style={{ height: `${Math.max(12, p.val)}%` }}
                  />
                  <span className="text-xs font-bold text-white font-mono">{p.order}</span>
                </div>
              ))}
            </div>
          </div>
        )}

        {/* Style 1: Dot Matrix Heatmap */}
        {selectedStyle === 1 && (
          <div className="flex flex-col gap-4 animate-fade-in">
            <div className="flex justify-between items-center">
              <span className="text-xs font-bold text-emerald-400 uppercase tracking-wider font-mono">
                2. 24-Hour Spatio-Temporal Anomaly Dot Matrix Heatmap
              </span>
              <span className="text-xs font-mono text-neutral-400">96 Continuous Observation Slices</span>
            </div>
            <div className="grid grid-cols-12 sm:grid-cols-24 gap-1.5 p-4 bg-[#0C0E14] rounded-2xl border border-white/5">
              {Array.from({ length: 96 }).map((_, i) => {
                const isAnomaly = i % 19 === 0 && telemetry.score < 60;
                const isWarning = i % 7 === 0 && telemetry.rms > 0.15;
                const color = isAnomaly
                  ? "bg-rose-500 shadow-[0_0_8px_rgba(244,63,94,0.8)]"
                  : isWarning
                  ? "bg-amber-400"
                  : "bg-emerald-500/30";
                return (
                  <div
                    key={i}
                    className={`w-full aspect-square rounded-md transition-all ${color}`}
                    title={`Slice #${i + 1}`}
                  />
                );
              })}
            </div>
          </div>
        )}

        {/* Style 2: Segmented VU Equalizer */}
        {selectedStyle === 2 && (
          <div className="flex flex-col gap-4 animate-fade-in">
            <div className="flex justify-between items-center">
              <span className="text-xs font-bold text-rose-400 uppercase tracking-wider font-mono">
                3. Segmented Multi-Band Spectral LED VU Equalizer
              </span>
              <span className="text-xs font-mono text-neutral-400">Peak Hold: -3.2 dBFS</span>
            </div>
            <div className="flex justify-between items-end gap-1.5 h-36 bg-[#0C0E14] p-4 rounded-2xl border border-white/5">
              {Array.from({ length: 24 }).map((_, i) => {
                const active = i < Math.round((telemetry.rms * 70) + 4);
                const color = i > 18 ? "bg-rose-500" : i > 12 ? "bg-amber-500" : "bg-[#F5C544]";
                return (
                  <div
                    key={i}
                    className={`flex-1 rounded-sm transition-all duration-150 ${active ? color : "bg-neutral-800"}`}
                    style={{ height: `${Math.max(15, (i + 1) * 4.2)}%` }}
                  />
                );
              })}
            </div>
          </div>
        )}

        {/* Style 3: Triple Rings */}
        {selectedStyle === 3 && (
          <div className="flex flex-col sm:flex-row items-center justify-around gap-6 p-4 animate-fade-in">
            <div className="relative w-48 h-48 flex items-center justify-center">
              <svg className="w-full h-full -rotate-90" viewBox="0 0 160 160">
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
                <span className="text-3xl font-black font-mono text-white">{telemetry.score}%</span>
                <span className="text-[10px] font-bold text-neutral-400">HEALTH</span>
              </div>
            </div>

            <div className="flex flex-col gap-3 font-mono text-xs">
              <div className="flex items-center gap-2">
                <span className="w-3 h-3 rounded-full bg-[#FA2C56]" />
                <span>Overall Asset Health: <strong>{telemetry.score}%</strong></span>
              </div>
              <div className="flex items-center gap-2">
                <span className="w-3 h-3 rounded-full bg-[#00F0FF]" />
                <span>Bearing / Contact Integrity: <strong>{telemetry.kurt > 4 ? "Degraded" : "Nominal"}</strong></span>
              </div>
              <div className="flex items-center gap-2">
                <span className="w-3 h-3 rounded-full bg-[#F5C544]" />
                <span>Harmonic Balance: <strong>{telemetry.rms.toFixed(3)}g</strong></span>
              </div>
            </div>
          </div>
        )}

        {/* Style 4: Progress Arc */}
        {selectedStyle === 4 && (
          <div className="flex flex-col items-center gap-3 p-4 animate-fade-in">
            <div className="relative w-44 h-44 flex items-center justify-center">
              <svg className="w-full h-full -rotate-90" viewBox="0 0 120 120">
                <circle cx="60" cy="60" r="48" stroke="#1F2636" strokeWidth="10" fill="none" />
                <circle
                  cx="60"
                  cy="60"
                  r="48"
                  stroke="#00F0FF"
                  strokeWidth="10"
                  fill="none"
                  strokeDasharray="301"
                  strokeDashoffset={301 - (301 * telemetry.score) / 100}
                  strokeLinecap="round"
                  className="transition-all duration-700"
                />
              </svg>
              <div className="absolute flex flex-col items-center">
                <span className="text-3xl font-black font-mono text-white">{telemetry.score}%</span>
                <span className="text-[10px] font-bold text-cyan-400">ISO CLASS A</span>
              </div>
            </div>
            <span className="text-xs text-neutral-400 font-mono">Aero-Acoustic Diagnostic Compliance Index</span>
          </div>
        )}

        {/* Style 5: Tachometer */}
        {selectedStyle === 5 && (
          <div className="flex flex-col items-center gap-3 p-4 animate-fade-in">
            <div className="relative w-48 h-48 flex items-center justify-center">
              <svg className="w-full h-full" viewBox="0 0 160 160">
                <path d="M 25 120 A 65 65 0 1 1 135 120" stroke="#1F2636" strokeWidth="12" fill="none" strokeLinecap="round" />
                <path
                  d="M 25 120 A 65 65 0 1 1 135 120"
                  stroke="#F59E0B"
                  strokeWidth="12"
                  fill="none"
                  strokeDasharray="260"
                  strokeDashoffset={260 - (260 * Math.min(4500, telemetry.rpm)) / 4500}
                  strokeLinecap="round"
                  className="transition-all duration-500"
                />
              </svg>
              <div className="absolute flex flex-col items-center text-center">
                <span className="text-3xl font-black font-mono text-white">{(telemetry.rpm || 2910).toLocaleString()}</span>
                <span className="text-[10px] font-bold text-orange-400">ROTOR RPM</span>
              </div>
            </div>
            <span className="text-xs text-neutral-400 font-mono">0 to 4,500 RPM Rotational Speed Range</span>
          </div>
        )}

        {/* Style 6: Fluid Waves */}
        {selectedStyle === 6 && (
          <div className="flex flex-col gap-3 p-2 animate-fade-in">
            <div className="relative w-full h-36 rounded-2xl overflow-hidden bg-[#0C0E14] border border-white/5">
              <canvas ref={canvasWaveRef} width={800} height={150} className="w-full h-full object-cover" />
              <div className="absolute inset-0 flex items-center justify-center pointer-events-none">
                <div className="bg-black/70 backdrop-blur-xs px-4 py-1.5 rounded-xl text-white font-mono font-bold text-sm">
                  ENERGY DENSITY: {telemetry.iso.toFixed(2)} mm/s RMS
                </div>
              </div>
            </div>
            <span className="text-xs text-neutral-400 font-mono text-center">Dynamic Fluid Turbulence Simulation</span>
          </div>
        )}

        {/* Style 7: ECG Pulse */}
        {selectedStyle === 7 && (
          <div className="flex flex-col gap-3 p-2 animate-fade-in">
            <div className="relative w-full h-36 rounded-2xl overflow-hidden bg-[#0C0E14] border border-white/5">
              <canvas ref={canvasECGRef} width={800} height={150} className="w-full h-full object-cover" />
            </div>
            <div className="flex justify-between items-center text-xs font-mono px-2">
              <span className="text-pink-400">Acoustic Peak: {(telemetry.f0 || 48.5).toFixed(1)} Hz</span>
              <span className="text-white">Cardiac Stethoscope Rhythm Model</span>
            </div>
          </div>
        )}

        {/* Style 8: Dual Spline Curves */}
        {selectedStyle === 8 && (
          <div className="flex flex-col gap-3 p-2 animate-fade-in">
            <div className="relative w-full h-36 rounded-2xl overflow-hidden bg-[#0C0E14] border border-white/5">
              <canvas ref={canvasSplineRef} width={800} height={150} className="w-full h-full object-cover" />
            </div>
            <div className="flex justify-between items-center text-xs font-mono px-2">
              <span className="text-rose-400">▲ Max Envelope Trace (Peak +g)</span>
              <span className="text-cyan-400">▼ Min Envelope Trace (Peak -g)</span>
            </div>
          </div>
        )}

        {/* Style 9: Bold Diagnostics */}
        {selectedStyle === 9 && (
          <div className="flex flex-col items-center text-center gap-3 p-6 bg-[#0C0E14] rounded-2xl border border-white/5 animate-fade-in">
            <div className="px-4 py-1 rounded-full bg-emerald-500/20 text-emerald-400 font-mono text-xs font-bold">
              NOMINAL HARMONIC BALANCE
            </div>
            <h3 className="text-2xl font-black text-white tracking-tight">
              ISO 10816 CLASS A CERTIFIED
            </h3>
            <p className="text-xs text-neutral-400 max-w-lg leading-relaxed">
              Machine is operating with optimal rotational symmetry. Zero impulsive shock spikes detected (Kurtosis: {telemetry.kurt.toFixed(2)} &lt; 4.0).
            </p>
          </div>
        )}
      </div>

      {/* Top 3 Diagnostic Cards Showcase Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6 pt-2">
        {/* Card 1: Neon ECG Pulse Line (Chart 8 style) */}
        <div className="bg-[#161B24] rounded-[28px] p-6 border border-white/10 shadow-xl flex flex-col justify-between relative overflow-hidden">
          <div className="flex justify-between items-center mb-4">
            <span className="text-xs font-bold text-neutral-400 uppercase tracking-wider font-mono">
              Acoustic Stethoscope Pulse
            </span>
            <span className="px-2.5 py-0.5 rounded-full text-[10px] font-mono bg-pink-500/20 text-pink-400 font-bold">
              MIC TRANSDUCER
            </span>
          </div>

          <div className="relative w-full h-36 rounded-xl overflow-hidden bg-[#0C0E14] border border-white/5">
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
        <div className="bg-[#161B24] rounded-[28px] p-6 border border-white/10 shadow-xl flex flex-col items-center justify-between relative">
          <div className="w-full flex justify-between items-center mb-2">
            <span className="text-xs font-bold text-neutral-400 uppercase tracking-wider font-mono">
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

          <div className="w-full flex justify-around text-center pt-2 border-t border-white/5 text-[11px] font-bold font-mono">
            <span className="text-[#FA2C56]">HEALTH: {telemetry.score}%</span>
            <span className="text-[#00F0FF]">BEARING: {telemetry.kurt > 4 ? "FAULT" : "GOOD"}</span>
            <span className="text-[#F5C544]">BALANCE: {telemetry.rms.toFixed(2)}g</span>
          </div>
        </div>

        {/* Card 3: Fluid Wave Tank & Segmented VU Meter (Chart 7 & Chart 3) */}
        <div className="bg-[#161B24] rounded-[28px] p-6 border border-white/10 shadow-xl flex flex-col justify-between relative overflow-hidden">
          <div className="flex justify-between items-center mb-3">
            <span className="text-xs font-bold text-neutral-400 uppercase tracking-wider font-mono">
              Fluid Severity & VU Meter
            </span>
            <span className="px-2.5 py-0.5 rounded-full text-[10px] font-mono bg-blue-500/20 text-blue-400 font-bold">
              FLUID TRANSDUCER
            </span>
          </div>

          {/* Fluid Canvas */}
          <div className="relative w-full h-32 rounded-xl overflow-hidden bg-[#0C0E14] border border-white/5 mb-3">
            <canvas ref={canvasWaveRef} width={340} height={130} className="w-full h-full object-cover" />
            <div className="absolute inset-0 flex items-center justify-center pointer-events-none">
              <div className="bg-black/70 backdrop-blur-xs px-3 py-1 rounded-lg text-white font-mono font-bold text-sm">
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
