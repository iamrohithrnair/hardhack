"use client";

import React, { useEffect, useRef, useState } from "react";
import { Volume2, VolumeX, Mic, Disc, Play, Pause, Sliders, Waves, Activity, Headphones } from "lucide-react";
import { TelemetryData } from "../../hooks/useDeviceStream";
import { MachineProfile } from "../../types/machine";

interface StethoscopeViewProps {
  telemetry: TelemetryData;
  machine: MachineProfile;
  soundEnabled: boolean;
  onToggleSound: () => void;
}

export const StethoscopeView: React.FC<StethoscopeViewProps> = ({
  telemetry,
  machine,
  soundEnabled,
  onToggleSound
}) => {
  const [filterMode, setFilterMode] = useState<"all" | "low" | "band" | "high">("all");
  const [volume, setVolume] = useState<number>(75);
  const [isRecording, setIsRecording] = useState<boolean>(false);
  const [recordTime, setRecordTime] = useState<number>(0);

  const canvasRef = useRef<HTMLCanvasElement | null>(null);

  // Audio Oscilloscope Wave Animation
  useEffect(() => {
    let animationFrameId: number;
    let phase = 0;

    const render = () => {
      const canvas = canvasRef.current;
      if (!canvas) return;
      const ctx = canvas.getContext("2d");
      if (!ctx) return;

      const w = canvas.width;
      const h = canvas.height;
      const midY = h / 2;

      ctx.fillStyle = "#0B0E14";
      ctx.fillRect(0, 0, w, h);

      // Draw Center Baseline
      ctx.strokeStyle = "rgba(255, 255, 255, 0.08)";
      ctx.lineWidth = 1;
      ctx.beginPath();
      ctx.moveTo(0, midY);
      ctx.lineTo(w, midY);
      ctx.stroke();

      // Mechanical wave simulation based on filter mode
      phase += 0.15;
      ctx.beginPath();
      ctx.lineWidth = 3;

      let strokeGrad = ctx.createLinearGradient(0, 0, w, 0);
      if (filterMode === "low") strokeGrad.addColorStop(0, "#0EA5E9");
      else if (filterMode === "band") strokeGrad.addColorStop(0, "#F5C544");
      else if (filterMode === "high") strokeGrad.addColorStop(0, "#F43F5E");
      else strokeGrad.addColorStop(0, "#10B981");

      ctx.strokeStyle = strokeGrad;
      ctx.shadowColor = (filterMode === "high" || telemetry.score < 50) ? "#F43F5E" : "#F5C544";
      ctx.shadowBlur = 12;

      for (let x = 0; x < w; x++) {
        const t = x / w;
        let y = 0;

        if (filterMode === "low") {
          // Low-pass deep drone
          y = Math.sin(phase + t * 12) * (h * 0.35);
        } else if (filterMode === "band") {
          // Blade pass harmonic
          y = (Math.sin(phase * 2 + t * 30) + 0.5 * Math.sin(phase * 4 + t * 60)) * (h * 0.28);
        } else if (filterMode === "high") {
          // High frequency impacts
          y = (Math.sin(phase * 8 + t * 90) * (Math.random() > 0.85 ? 1.8 : 0.4)) * (h * 0.25);
        } else {
          // All-pass full composite sound
          y = (Math.sin(phase + t * 12) * 0.5 + Math.sin(phase * 3 + t * 36) * 0.3 + (Math.random() - 0.5) * 0.2) * (h * 0.35);
        }

        if (x === 0) ctx.moveTo(x, midY + y);
        else ctx.lineTo(x, midY + y);
      }

      ctx.stroke();
      ctx.shadowBlur = 0;

      animationFrameId = requestAnimationFrame(render);
    };

    render();
    return () => cancelAnimationFrame(animationFrameId);
  }, [filterMode, telemetry.score]);

  // Recording Timer
  useEffect(() => {
    if (!isRecording) return;
    const interval = setInterval(() => {
      setRecordTime((t) => t + 1);
    }, 1000);
    return () => clearInterval(interval);
  }, [isRecording]);

  return (
    <div className="flex flex-col gap-6 animate-fade-in">
      {/* Header Banner */}
      <div className="p-6 rounded-[28px] bg-white border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col sm:flex-row justify-between items-start sm:items-center gap-4">
        <div className="flex items-center gap-3.5">
          <div className="w-12 h-12 rounded-2xl bg-amber-100 flex items-center justify-center text-amber-700">
            <Headphones className="w-6 h-6" />
          </div>
          <div>
            <h2 className="text-xl font-extrabold text-[#12141A]">
              Acoustic Stethoscope Diagnostic Lab
            </h2>
            <p className="text-xs text-[#6B7280]">
              Listening to {machine.name} micro-vibrations & acoustic transducer stream
            </p>
          </div>
        </div>

        <div className="flex items-center gap-3">
          <button
            onClick={() => {
              if (isRecording) {
                setIsRecording(false);
                setRecordTime(0);
              } else {
                setIsRecording(true);
              }
            }}
            className={`flex items-center gap-2 px-4 py-2 rounded-full text-xs font-bold transition-all cursor-pointer shadow-xs ${
              isRecording
                ? "bg-rose-500 text-white animate-pulse"
                : "bg-neutral-100 hover:bg-neutral-200 text-[#12141A]"
            }`}
          >
            <Disc className="w-4 h-4" />
            <span>{isRecording ? `Recording (${recordTime}s)` : "Record Sample"}</span>
          </button>

          <button
            onClick={onToggleSound}
            className={`flex items-center gap-2 px-4 py-2 rounded-full text-xs font-bold transition-all cursor-pointer shadow-xs ${
              soundEnabled
                ? "bg-[#1C1F26] text-white"
                : "bg-neutral-100 text-[#6B7280]"
            }`}
          >
            {soundEnabled ? <Volume2 className="w-4 h-4 text-[#F5C544]" /> : <VolumeX className="w-4 h-4" />}
            <span>{soundEnabled ? "Audio Live" : "Audio Muted"}</span>
          </button>
        </div>
      </div>

      {/* Main Waveform Transducer Canvas */}
      <div className="p-6 rounded-[28px] bg-white border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col gap-4">
        <div className="flex justify-between items-center">
          <div className="flex items-center gap-2">
            <Waves className="w-5 h-5 text-[#F5C544]" />
            <h3 className="text-base font-bold text-[#12141A]">
              Live Stethoscope Acoustic Oscilloscope
            </h3>
          </div>
          <span className="text-xs font-mono font-bold text-[#0EA5E9]">
            Transducer: QMI8658 / ES8311 Codec
          </span>
        </div>

        <div className="w-full h-72 rounded-2xl overflow-hidden bg-[#0B0E14] shadow-inner">
          <canvas ref={canvasRef} width={1200} height={288} className="w-full h-full block" />
        </div>
      </div>

      {/* Acoustic Filter Selector Cards */}
      <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
        {[
          {
            id: "all",
            name: "1. All-Pass Mechanical",
            range: "10 Hz – 1,000 Hz",
            desc: "Full acoustic spectrum including low hum and high friction hiss.",
            color: "border-emerald-400 bg-emerald-50/40 text-emerald-900"
          },
          {
            id: "low",
            name: "2. Subharmonic & Unbalance",
            range: "10 Hz – 100 Hz",
            desc: "Isolates deep rotational rumble, 1X mass eccentricity, and loose chassis mounting.",
            color: "border-sky-400 bg-sky-50/40 text-sky-900"
          },
          {
            id: "band",
            name: "3. Blade-Pass & Gear Mesh",
            range: "100 Hz – 600 Hz",
            desc: "Isolates fan blade buffeting tones, belt slap, and gear tooth impact frequencies.",
            color: "border-amber-400 bg-amber-50/40 text-amber-900"
          },
          {
            id: "high",
            name: "4. Bearing Pitting (Kurtosis)",
            range: "600 Hz – 2,000 Hz",
            desc: "Isolates high-frequency micro-spalling clicks and dry raceway squeals.",
            color: "border-rose-400 bg-rose-50/40 text-rose-900"
          }
        ].map((f) => {
          const isSelected = filterMode === f.id;
          return (
            <div
              key={f.id}
              onClick={() => setFilterMode(f.id as any)}
              className={`p-5 rounded-[24px] border transition-all cursor-pointer flex flex-col justify-between gap-3 ${
                isSelected
                  ? `${f.color} shadow-sm ring-2 ring-amber-400/40`
                  : "border-black/5 bg-white hover:border-neutral-300"
              }`}
            >
              <div>
                <span className="text-[10px] font-bold uppercase tracking-wider text-[#6B7280]">
                  {f.range}
                </span>
                <h4 className="text-sm font-bold text-[#12141A] mt-0.5">{f.name}</h4>
                <p className="text-xs text-[#6B7280] mt-1 leading-relaxed">{f.desc}</p>
              </div>

              <div className="pt-2 border-t border-black/5 flex items-center justify-between">
                <span className="text-[11px] font-bold text-[#12141A]">
                  {isSelected ? "Active Listening Mode" : "Click to Listen"}
                </span>
                {isSelected && (
                  <span className="w-2 h-2 rounded-full bg-emerald-500 shadow-[0_0_6px_rgba(16,185,129,0.8)]" />
                )}
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
};
