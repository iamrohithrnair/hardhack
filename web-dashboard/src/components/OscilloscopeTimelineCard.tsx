"use client";

import React, { useEffect, useRef, useState } from "react";
import { TelemetryData } from "../hooks/useDeviceStream";
import { MachineProfile } from "../types/machine";
import { Activity, Waves, BarChart3, Zap, ShieldAlert, Sparkles } from "lucide-react";

interface OscilloscopeTimelineCardProps {
  telemetry: TelemetryData;
  machine: MachineProfile;
}

export const OscilloscopeTimelineCard: React.FC<OscilloscopeTimelineCardProps> = ({
  telemetry,
  machine
}) => {
  const [activeMode, setActiveMode] = useState<"classic" | "multiaxis" | "kurtosis">("classic");

  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const waveHistoryRef = useRef<number[]>(new Array(240).fill(0));
  const waveHistoryYRef = useRef<number[]>(new Array(240).fill(0));
  const waveHistoryZRef = useRef<number[]>(new Array(240).fill(0));
  const fftHistoryRef = useRef<number[]>(new Array(28).fill(0.05));
  const fftPeaksRef = useRef<number[]>(new Array(28).fill(0.05));
  const phaseRef = useRef<number>(0);

  const isFault = telemetry.score < 50;

  // Custom asset parameters
  const targetHz = machine.fundamentalHz || 48.5;
  const warnRms = machine.warningRms || 0.25;
  const critRms = machine.criticalRms || 0.85;
  const kurtThresh = machine.kurtosisThreshold || 4.0;

  useEffect(() => {
    let animationFrameId: number;

    const render = () => {
      const canvas = canvasRef.current;
      if (!canvas) return;
      const ctx = canvas.getContext("2d");
      if (!ctx) return;

      const w = canvas.width;
      const h = canvas.height;
      const midY = h / 2;

      // Dark obsidian canvas background
      ctx.fillStyle = "#0D1017";
      ctx.fillRect(0, 0, w, h);

      // Subtle Center Line
      ctx.strokeStyle = "#1E2330";
      ctx.lineWidth = 1;
      ctx.beginPath();
      ctx.moveTo(0, midY);
      ctx.lineTo(w, midY);
      ctx.stroke();

      // Dynamic Angular Phase Speed driven by fundamental frequency
      const displayHz = telemetry.f0 > 1.0 ? telemetry.f0 : targetHz;
      const freqMultiplier = Math.max(0.08, Math.min(0.25, displayHz / 280.0));
      phaseRef.current += freqMultiplier;

      // Scale dynamic amplitude relative to custom warning limit
      const normRms = telemetry.rms / Math.max(0.05, warnRms);
      const rmsAmp = Math.max(0.18, Math.min(1.2, normRms * 0.45));
      const noise = (Math.random() - 0.5) * (0.02 + telemetry.kurt * 0.008);

      let sampleX = 0;
      let sampleY = 0;
      let sampleZ = 0;

      if (telemetry.state === 1) {
        // Nominal: ultra smooth harmonic sine wave
        sampleX = rmsAmp * Math.sin(phaseRef.current) + noise;
        sampleY = rmsAmp * 0.7 * Math.sin(phaseRef.current + 1.2) + noise * 0.8;
        sampleZ = rmsAmp * 0.8 * Math.sin(phaseRef.current + 2.4) + noise * 0.8;
      } else if (telemetry.state === 3) {
        // Rotor unbalance / resonance: high 1X harmonic modulation
        const mod = 1.0 + 0.35 * Math.sin(phaseRef.current * 0.25);
        sampleX = rmsAmp * 1.6 * Math.sin(phaseRef.current) * mod + 0.35 * Math.sin(phaseRef.current * 2) + noise * 2;
        sampleY = rmsAmp * 1.1 * Math.sin(phaseRef.current + 1.4) + noise * 1.5;
        sampleZ = rmsAmp * 1.3 * Math.sin(phaseRef.current + 2.8) + noise * 1.5;
      } else if (telemetry.state === 4) {
        // Bearing spalling / impulsive shock: sudden micro-peaks
        const isImpact = Math.random() < 0.14;
        const impactSpike = isImpact ? (Math.random() > 0.5 ? 1.8 : -1.8) * rmsAmp * 2.2 : 0;
        sampleX = rmsAmp * Math.sin(phaseRef.current) + impactSpike + noise * 3;
        sampleY = rmsAmp * 0.7 * Math.sin(phaseRef.current + 1.2) + impactSpike * 0.7 + noise * 2;
        sampleZ = rmsAmp * 0.85 * Math.sin(phaseRef.current + 2.4) + impactSpike * 0.85 + noise * 2;
      } else {
        sampleX = rmsAmp * Math.sin(phaseRef.current) + noise;
        sampleY = rmsAmp * 0.75 * Math.sin(phaseRef.current + 1.2) + noise;
        sampleZ = rmsAmp * 0.85 * Math.sin(phaseRef.current + 2.4) + noise;
      }

      waveHistoryRef.current.shift();
      waveHistoryRef.current.push(sampleX);

      waveHistoryYRef.current.shift();
      waveHistoryYRef.current.push(sampleY);

      waveHistoryZRef.current.shift();
      waveHistoryZRef.current.push(sampleZ);

      // -------------------------------------------------------------
      // 1. 24-Band FFT Bars in Background
      // -------------------------------------------------------------
      const BARS = 28;
      const barWidth = (w - (BARS - 1) * 6) / BARS;

      for (let i = 0; i < BARS; i++) {
        let target = (telemetry.visualSpectrum[i % telemetry.visualSpectrum.length] || 0.05);
        if (telemetry.state === 3 && i === 4) target = 0.95;

        fftHistoryRef.current[i] = fftHistoryRef.current[i] * 0.8 + target * 0.2;
        if (fftHistoryRef.current[i] > fftPeaksRef.current[i]) {
          fftPeaksRef.current[i] = fftHistoryRef.current[i];
        } else {
          fftPeaksRef.current[i] = Math.max(0, fftPeaksRef.current[i] - 0.012);
        }

        const bx = i * (barWidth + 6);
        const bh = fftHistoryRef.current[i] * (h * 0.7);
        const by = h - bh;

        ctx.fillStyle = isFault ? "rgba(244, 63, 94, 0.22)" : "rgba(245, 197, 68, 0.18)";
        ctx.fillRect(bx, by, barWidth, bh);

        // Peak line
        const py = h - fftPeaksRef.current[i] * (h * 0.7);
        ctx.fillStyle = isFault ? "#F43F5E" : "#F5C544";
        ctx.fillRect(bx, py, barWidth, 2);
      }

      // -------------------------------------------------------------
      // 2. Draw Smooth Glowing Sine Waves
      // -------------------------------------------------------------
      if (activeMode === "multiaxis") {
        // Multi-Axial Waves (Red, Green, Cyan)
        const renderTrace = (pts: number[], color: string, shadow: string, width: number) => {
          ctx.save();
          ctx.strokeStyle = color;
          ctx.shadowColor = shadow;
          ctx.shadowBlur = 10;
          ctx.lineWidth = width;
          ctx.beginPath();
          for (let i = 0; i < pts.length; i++) {
            const x = (i / (pts.length - 1)) * w;
            const y = midY - pts[i] * (h * 0.38);
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
          }
          ctx.stroke();
          ctx.restore();
        };

        renderTrace(waveHistoryZRef.current, "#FF2A54", "rgba(255, 42, 84, 0.6)", 2.0);
        renderTrace(waveHistoryYRef.current, "#00FF66", "rgba(0, 255, 102, 0.6)", 2.0);
        renderTrace(waveHistoryRef.current, "#00F0FF", "rgba(0, 240, 255, 0.9)", 3.0);
      } else {
        // Classic Ultra-Smooth Single Sine Wave (Sky Cyan or Amber/Rose on Fault)
        const traceColor = isFault ? "#F43F5E" : "#0EA5E9";
        ctx.save();
        ctx.strokeStyle = traceColor;
        ctx.lineWidth = 3.0;
        ctx.shadowColor = traceColor;
        ctx.shadowBlur = 12;
        ctx.beginPath();

        const points = waveHistoryRef.current;
        for (let i = 0; i < points.length; i++) {
          const x = (i / (points.length - 1)) * w;
          const y = midY - points[i] * (h * 0.38);
          if (i === 0) ctx.moveTo(x, y);
          else ctx.lineTo(x, y);
        }
        ctx.stroke();

        // Glowing Leading Edge Dot
        const lastVal = points[points.length - 1];
        const lastY = midY - lastVal * (h * 0.38);
        ctx.fillStyle = traceColor;
        ctx.beginPath();
        ctx.arc(w - 4, lastY, 5, 0, Math.PI * 2);
        ctx.fill();
        ctx.restore();
      }

      animationFrameId = requestAnimationFrame(render);
    };

    render();
    return () => cancelAnimationFrame(animationFrameId);
  }, [activeMode, isFault, targetHz, telemetry.f0, telemetry.kurt, telemetry.rms, telemetry.score, telemetry.state, telemetry.visualSpectrum, warnRms]);

  return (
    <section className="bg-white rounded-[28px] p-6 border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col gap-4">
      {/* Header with Timeline Date & Mode Switcher */}
      <div className="flex flex-col sm:flex-row justify-between items-start sm:items-center gap-3">
        <div className="flex items-center gap-3.5">
          <div className="flex items-center gap-2">
            <span className="w-2.5 h-2.5 rounded-full bg-sky-500 animate-pulse" />
            <h3 className="text-base font-extrabold text-[#12141A]">
              Physical Micro-Vibration Oscilloscope & FFT
            </h3>
          </div>
          <span className="px-2.5 py-0.5 rounded-full bg-black/[0.04] border border-black/5 text-[11px] font-mono font-bold text-[#6B7280] uppercase">
            {machine.name} ({targetHz} Hz)
          </span>
        </div>

        {/* Legend & Mode Toggles */}
        <div className="flex items-center gap-3 text-[11px] font-semibold text-[#6B7280]">
          <button
            onClick={() => setActiveMode("classic")}
            className={`px-3 py-1 rounded-full transition-all cursor-pointer ${
              activeMode === "classic"
                ? "bg-[#1C1F26] text-white shadow-xs"
                : "bg-neutral-100 hover:bg-neutral-200 text-[#4B5563]"
            }`}
          >
            Smooth Sine Wave
          </button>
          <button
            onClick={() => setActiveMode("multiaxis")}
            className={`px-3 py-1 rounded-full transition-all cursor-pointer ${
              activeMode === "multiaxis"
                ? "bg-[#1C1F26] text-white shadow-xs"
                : "bg-neutral-100 hover:bg-neutral-200 text-[#4B5563]"
            }`}
          >
            3-Axis Neon
          </button>

          <div className="hidden sm:flex items-center gap-1.5 ml-2">
            <span className="w-2 h-2 rounded-full bg-[#F5C544]" />
            <span>28-Band FFT</span>
          </div>

          {isFault && (
            <div className="flex items-center gap-1.5 text-rose-600">
              <span className="w-2 h-2 rounded-full bg-rose-500 animate-ping" />
              <span>Anomaly Spike</span>
            </div>
          )}
        </div>
      </div>

      {/* Canvas Viewport with Floating Event Glass Pills */}
      <div className="w-full h-44 bg-[#0D1017] rounded-2xl overflow-hidden relative shadow-inner">
        <canvas
          ref={canvasRef}
          width={1200}
          height={176}
          className="w-full h-full block"
        />

        {/* Floating Timeline Event Pills */}
        <div className="absolute top-3 left-[14%] bg-[#1C1F26]/90 border border-white/15 border-l-4 border-l-sky-400 backdrop-blur-md px-3.5 py-1.5 rounded-full text-white shadow-md pointer-events-none flex flex-col">
          <span className="text-[11px] font-bold">Baseline Harmonic Lock</span>
          <span className="text-[9px] text-[#9CA3AF] font-mono">
            {targetHz} Hz Fundamental · {(telemetry.rms || 0.082).toFixed(3)}g RMS
          </span>
        </div>

        {isFault ? (
          <div className="absolute top-3 left-[62%] bg-rose-950/90 border border-rose-500/30 border-l-4 border-l-rose-500 backdrop-blur-md px-3.5 py-1.5 rounded-full text-white shadow-lg pointer-events-none flex flex-col animate-bounce">
            <span className="text-[11px] font-bold text-rose-300">Vibration Excursion Spike</span>
            <span className="text-[9px] text-rose-200 font-mono">
              +18 dB 1X Harmonic · Health {telemetry.score}%
            </span>
          </div>
        ) : (
          <div className="absolute top-3 left-[68%] bg-emerald-950/80 border border-emerald-500/30 border-l-4 border-l-emerald-400 backdrop-blur-md px-3.5 py-1.5 rounded-full text-white shadow-md pointer-events-none flex flex-col">
            <span className="text-[11px] font-bold text-emerald-300">Gaussian Symmetry</span>
            <span className="text-[9px] text-emerald-200 font-mono">
              Kurtosis: {telemetry.kurt.toFixed(2)} &lt; {kurtThresh}
            </span>
          </div>
        )}

        {/* Bottom Right Live Scale Overlay */}
        <div className="absolute bottom-2.5 right-4 flex items-center gap-3 text-[10px] font-mono text-neutral-400 bg-black/60 backdrop-blur-xs px-2.5 py-1 rounded-lg border border-white/10 pointer-events-none">
          <span>RMS: <strong className="text-[#F5C544]">{telemetry.rms.toFixed(3)}g</strong></span>
          <span>ISO: <strong className="text-white">{telemetry.iso.toFixed(2)} mm/s</strong></span>
          <span>FREQ: <strong className="text-sky-400">{(telemetry.f0 || targetHz).toFixed(1)} Hz</strong></span>
        </div>
      </div>

      {/* Timeline Axis Labels */}
      <div className="flex justify-between text-[11px] text-[#9CA3AF] font-semibold px-2 font-mono">
        <span>00:00s</span>
        <span>00:15s (Calibration Lock)</span>
        <span>00:30s (Spectral Analysis)</span>
        <span>00:45s (Dynamic Tracking)</span>
        <span>01:00s (Health Assessment)</span>
      </div>
    </section>
  );
};
