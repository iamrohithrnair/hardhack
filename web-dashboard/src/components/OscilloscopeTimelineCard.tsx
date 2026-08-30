"use client";

import React, { useEffect, useRef, useState } from "react";
import { TelemetryData } from "../hooks/useDeviceStream";
import { MachineProfile } from "../types/machine";
import { Activity, Waves, BarChart3, Zap, ShieldAlert, Sparkles, Sliders } from "lucide-react";

interface OscilloscopeTimelineCardProps {
  telemetry: TelemetryData;
  machine: MachineProfile;
}

export const OscilloscopeTimelineCard: React.FC<OscilloscopeTimelineCardProps> = ({
  telemetry,
  machine
}) => {
  const [activeView, setActiveView] = useState<"sine" | "fft" | "kurtosis">("sine");

  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const waveHistoryXRef = useRef<number[]>(new Array(220).fill(0));
  const waveHistoryYRef = useRef<number[]>(new Array(220).fill(0));
  const waveHistoryZRef = useRef<number[]>(new Array(220).fill(0));
  const kurtosisHistoryRef = useRef<number[]>(new Array(120).fill(2.9));
  const fftHistoryRef = useRef<number[]>(new Array(24).fill(0.05));
  const phaseRef = useRef<number>(0);

  const isCritical = telemetry.score < 30;

  // Custom asset parameters
  const targetHz = machine.fundamentalHz || 48.5;
  const warnRms = machine.warningRms || 0.25;
  const critRms = machine.criticalRms || 0.85;
  const kurtThresh = machine.kurtosisThreshold || 4.0;
  const targetType = machine.detectionTarget || "vibration";

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

      // Dark Obsidian Canvas Background
      ctx.fillStyle = "#0C0E14";
      ctx.fillRect(0, 0, w, h);

      // Subtle Grid Lines
      ctx.strokeStyle = "rgba(255, 255, 255, 0.04)";
      ctx.lineWidth = 1;
      for (let x = 0; x < w; x += 40) {
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, h);
        ctx.stroke();
      }
      for (let y = 0; y < h; y += 40) {
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(w, y);
        ctx.stroke();
      }

      // Dynamic angular frequency and phase proportional to asset fundamental frequency
      const displayHz = telemetry.f0 > 1.0 ? telemetry.f0 : targetHz;
      const freqMultiplier = Math.max(0.04, Math.min(0.4, displayHz / 250.0));
      phaseRef.current += freqMultiplier;

      // Amplitude scaled relative to asset's custom warning threshold
      const normalizedRms = telemetry.rms / Math.max(0.05, warnRms);
      const rmsAmp = Math.max(0.12, Math.min(1.4, normalizedRms * 0.8));
      const noise = (Math.random() - 0.5) * (0.03 + (telemetry.kurt / kurtThresh) * 0.02);

      // 3-Axis Multi-Chromatic Signals
      let sampleX = 0;
      let sampleY = 0;
      let sampleZ = 0;

      if (telemetry.state === 1) {
        // Nominal Harmonic
        sampleX = rmsAmp * Math.sin(phaseRef.current) + noise;
        sampleY = rmsAmp * 0.75 * Math.sin(phaseRef.current + 1.2) + noise * 0.8;
        sampleZ = rmsAmp * 0.85 * Math.sin(phaseRef.current + 2.4) + noise * 0.9;
      } else if (telemetry.state === 3) {
        // Unbalance / Resonance
        const mod = 1.0 + 0.4 * Math.sin(phaseRef.current * 0.25);
        sampleX = rmsAmp * 1.8 * Math.sin(phaseRef.current) * mod + 0.4 * Math.sin(phaseRef.current * 2) + noise * 2;
        sampleY = rmsAmp * 1.2 * Math.sin(phaseRef.current + 1.5) + noise * 1.5;
        sampleZ = rmsAmp * 1.4 * Math.sin(phaseRef.current + 3.0) + noise * 1.5;
      } else if (telemetry.state === 4) {
        // Impact Shock
        const isImpact = Math.random() < 0.12;
        const impactSpike = isImpact ? (Math.random() > 0.5 ? 2.0 : -2.0) * rmsAmp * 2.5 : 0;
        sampleX = rmsAmp * Math.sin(phaseRef.current) + impactSpike + noise * 3;
        sampleY = rmsAmp * 0.7 * Math.sin(phaseRef.current + 1.2) + impactSpike * 0.7 + noise * 2;
        sampleZ = rmsAmp * 0.9 * Math.sin(phaseRef.current + 2.4) + impactSpike * 0.9 + noise * 2;
      } else {
        sampleX = rmsAmp * Math.sin(phaseRef.current) + noise;
        sampleY = rmsAmp * 0.8 * Math.sin(phaseRef.current + 1.2) + noise;
        sampleZ = rmsAmp * 0.9 * Math.sin(phaseRef.current + 2.4) + noise;
      }

      // Shift wave histories
      waveHistoryXRef.current.push(sampleX);
      waveHistoryXRef.current.shift();

      waveHistoryYRef.current.push(sampleY);
      waveHistoryYRef.current.shift();

      waveHistoryZRef.current.push(sampleZ);
      waveHistoryZRef.current.shift();

      // Shift Kurtosis history
      kurtosisHistoryRef.current.push(telemetry.kurt);
      kurtosisHistoryRef.current.shift();

      // -------------------------------------------------------------
      // RENDER 1: MULTI-CHROMATIC SINE WAVES
      // -------------------------------------------------------------
      if (activeView === "sine") {
        ctx.strokeStyle = "rgba(255, 255, 255, 0.1)";
        ctx.lineWidth = 1;
        ctx.setLineDash([4, 4]);
        ctx.beginPath();
        ctx.moveTo(0, midY);
        ctx.lineTo(w, midY);
        ctx.stroke();
        ctx.setLineDash([]);

        // Upper & Lower Warning Threshold Guides
        const warnYTop = midY - (h * 0.32);
        const warnYBot = midY + (h * 0.32);
        ctx.strokeStyle = "rgba(245, 197, 68, 0.25)";
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(0, warnYTop);
        ctx.lineTo(w, warnYTop);
        ctx.moveTo(0, warnYBot);
        ctx.lineTo(w, warnYBot);
        ctx.stroke();

        const renderWaveTrace = (
          history: number[],
          strokeColor: string,
          shadowColor: string,
          lineWidth: number
        ) => {
          ctx.save();
          ctx.strokeStyle = strokeColor;
          ctx.shadowColor = shadowColor;
          ctx.shadowBlur = 12;
          ctx.lineWidth = lineWidth;
          ctx.beginPath();

          const step = w / (history.length - 1);
          for (let i = 0; i < history.length; i++) {
            const x = i * step;
            const y = midY - history[i] * (h * 0.32);
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
          }
          ctx.stroke();
          ctx.restore();
        };

        // Trace 1: Hot Red/Pink (Z-Axis)
        renderWaveTrace(waveHistoryZRef.current, "#FF2A54", "rgba(255, 42, 84, 0.6)", 2.5);
        // Trace 2: Lime Green (Y-Axis)
        renderWaveTrace(waveHistoryYRef.current, "#00FF66", "rgba(0, 255, 102, 0.6)", 2.5);
        // Trace 3: Electric Cyan (X-Axis - Dominant Lead)
        renderWaveTrace(waveHistoryXRef.current, "#00F0FF", "rgba(0, 240, 255, 0.8)", 3.5);

        // Leading Cursor Glow
        const lastVal = waveHistoryXRef.current[waveHistoryXRef.current.length - 1];
        const cursorY = midY - lastVal * (h * 0.32);
        ctx.fillStyle = "#00F0FF";
        ctx.shadowColor = "#00F0FF";
        ctx.shadowBlur = 16;
        ctx.beginPath();
        ctx.arc(w - 4, cursorY, 5, 0, Math.PI * 2);
        ctx.fill();
      }

      // -------------------------------------------------------------
      // RENDER 2: 24-BAND FFT SPECTRUM
      // -------------------------------------------------------------
      else if (activeView === "fft") {
        const barCount = 24;
        const barW = (w - (barCount + 1) * 8) / barCount;

        for (let i = 0; i < barCount; i++) {
          const x = 8 + i * (barW + 8);
          
          let target = (telemetry.visualSpectrum[i % telemetry.visualSpectrum.length] || 0.1);
          if (telemetry.state === 3 && i === 4) target = 0.95;

          fftHistoryRef.current[i] = fftHistoryRef.current[i] * 0.8 + target * 0.2;
          const barH = fftHistoryRef.current[i] * (h * 0.78);
          const y = h - barH - 24;

          const grad = ctx.createLinearGradient(0, h, 0, y);
          if (i > 18) {
            grad.addColorStop(0, "#FF2A54");
            grad.addColorStop(1, "#FF6B8B");
          } else if (i > 12) {
            grad.addColorStop(0, "#F59E0B");
            grad.addColorStop(1, "#FBBF24");
          } else if (i > 6) {
            grad.addColorStop(0, "#F5C544");
            grad.addColorStop(1, "#FEF08A");
          } else {
            grad.addColorStop(0, "#00F0FF");
            grad.addColorStop(1, "#67E8F9");
          }

          ctx.fillStyle = grad;
          ctx.beginPath();
          ctx.roundRect(x, y, barW, barH, [4, 4, 0, 0]);
          ctx.fill();

          ctx.fillStyle = "#FFFFFF";
          ctx.fillRect(x, y - 4, barW, 2);
        }
      }

      // -------------------------------------------------------------
      // RENDER 3: KURTOSIS IMPACT TIMELINE
      // -------------------------------------------------------------
      else if (activeView === "kurtosis") {
        const maxK = Math.max(8.0, kurtThresh * 1.5);
        const threshY = h - (kurtThresh / maxK) * h;
        ctx.strokeStyle = "#FF2A54";
        ctx.lineWidth = 1.5;
        ctx.setLineDash([6, 6]);
        ctx.beginPath();
        ctx.moveTo(0, threshY);
        ctx.lineTo(w, threshY);
        ctx.stroke();
        ctx.setLineDash([]);

        ctx.fillStyle = "#FF2A54";
        ctx.font = "10px monospace";
        ctx.fillText(`ANOMALY THRESHOLD (κ = ${kurtThresh.toFixed(1)})`, 10, threshY - 6);

        ctx.save();
        ctx.strokeStyle = telemetry.kurt > kurtThresh ? "#FF2A54" : "#00FF66";
        ctx.shadowColor = telemetry.kurt > kurtThresh ? "rgba(255, 42, 84, 0.8)" : "rgba(0, 255, 102, 0.8)";
        ctx.shadowBlur = 10;
        ctx.lineWidth = 3;
        ctx.beginPath();

        const step = w / (kurtosisHistoryRef.current.length - 1);
        for (let i = 0; i < kurtosisHistoryRef.current.length; i++) {
          const val = kurtosisHistoryRef.current[i];
          const x = i * step;
          const y = h - (val / maxK) * (h * 0.85) - 20;
          if (i === 0) ctx.moveTo(x, y);
          else ctx.lineTo(x, y);
        }
        ctx.stroke();
        ctx.restore();
      }

      animationFrameId = requestAnimationFrame(render);
    };

    render();

    return () => cancelAnimationFrame(animationFrameId);
  }, [activeView, critRms, kurtThresh, targetHz, targetType, telemetry.f0, telemetry.kurt, telemetry.rms, telemetry.score, telemetry.state, telemetry.visualSpectrum, warnRms]);

  return (
    <div className="bg-[#12141A] text-white p-6 rounded-[32px] border border-white/10 shadow-[0_20px_40px_rgba(0,0,0,0.25)] flex flex-col gap-5 relative overflow-hidden">
      {/* Top Header & View Toggles */}
      <div className="flex flex-col sm:flex-row justify-between items-start sm:items-center gap-4">
        <div className="flex items-center gap-3">
          <div className="w-11 h-11 rounded-2xl bg-sky-500/10 border border-sky-500/20 flex items-center justify-center text-sky-400">
            <Waves className="w-5 h-5" />
          </div>
          <div>
            <div className="flex items-center gap-2">
              <h3 className="text-base font-extrabold text-white flex items-center gap-2">
                {machine.name} Live Stream
                <span className="w-2 h-2 rounded-full bg-emerald-400 animate-ping" />
              </h3>
              <span className="px-2 py-0.5 rounded-full text-[10px] font-mono bg-amber-500/20 text-[#F5C544] font-bold uppercase">
                {machine.detectionTarget || "VIBRATION"}
              </span>
            </div>
            <p className="text-xs text-neutral-400 font-mono mt-0.5">
              250 Hz High-Speed Telemetry · Target Fundamental: {targetHz} Hz · Baseline: {warnRms}g RMS
            </p>
          </div>
        </div>

        {/* View Switcher Pills */}
        <div className="flex items-center gap-1.5 p-1 rounded-2xl bg-[#1C1F26] border border-white/5">
          <button
            onClick={() => setActiveView("sine")}
            className={`px-3.5 py-1.5 rounded-xl text-xs font-bold transition-all cursor-pointer flex items-center gap-1.5 ${
              activeView === "sine"
                ? "bg-[#F5C544] text-black shadow-md font-extrabold"
                : "text-neutral-400 hover:text-white"
            }`}
          >
            <Waves className="w-3.5 h-3.5" />
            1. Tri-Axial Waves
          </button>
          <button
            onClick={() => setActiveView("fft")}
            className={`px-3.5 py-1.5 rounded-xl text-xs font-bold transition-all cursor-pointer flex items-center gap-1.5 ${
              activeView === "fft"
                ? "bg-[#00F0FF] text-black shadow-md font-extrabold"
                : "text-neutral-400 hover:text-white"
            }`}
          >
            <BarChart3 className="w-3.5 h-3.5" />
            2. FFT Spectrum
          </button>
          <button
            onClick={() => setActiveView("kurtosis")}
            className={`px-3.5 py-1.5 rounded-xl text-xs font-bold transition-all cursor-pointer flex items-center gap-1.5 ${
              activeView === "kurtosis"
                ? "bg-[#00FF66] text-black shadow-md font-extrabold"
                : "text-neutral-400 hover:text-white"
            }`}
          >
            <Zap className="w-3.5 h-3.5" />
            3. Kurtosis Shock
          </button>
        </div>
      </div>

      {/* Real-Time Canvas Graph Container */}
      <div className="relative w-full h-64 rounded-2xl overflow-hidden border border-white/5 bg-[#0C0E14]">
        <canvas
          ref={canvasRef}
          width={1200}
          height={320}
          className="w-full h-full object-cover"
        />

        {/* Legend Overlay */}
        <div className="absolute top-3 right-3 flex items-center gap-3 bg-black/60 backdrop-blur-md px-3 py-1.5 rounded-xl border border-white/10 text-[11px] font-mono">
          {activeView === "sine" && (
            <>
              <span className="flex items-center gap-1 text-[#00F0FF]">
                <span className="w-2 h-2 rounded-full bg-[#00F0FF]" /> X-Axis (Lead)
              </span>
              <span className="flex items-center gap-1 text-[#00FF66]">
                <span className="w-2 h-2 rounded-full bg-[#00FF66]" /> Y-Axis
              </span>
              <span className="flex items-center gap-1 text-[#FF2A54]">
                <span className="w-2 h-2 rounded-full bg-[#FF2A54]" /> Z-Axis
              </span>
            </>
          )}
          {activeView === "fft" && (
            <span className="text-[#F5C544]">
              Fundamental F0: {(telemetry.f0 || targetHz).toFixed(1)} Hz · Harmonics of {machine.name}
            </span>
          )}
          {activeView === "kurtosis" && (
            <span className={telemetry.kurt > kurtThresh ? "text-rose-400 font-bold" : "text-emerald-400 font-bold"}>
              Current Kurtosis: {telemetry.kurt.toFixed(2)} (Threshold: {kurtThresh.toFixed(1)})
            </span>
          )}
        </div>
      </div>

      {/* Bottom Summary Bar */}
      <div className="grid grid-cols-2 sm:grid-cols-4 gap-3 text-xs font-mono">
        <div className="bg-[#161B24] p-3 rounded-2xl border border-white/5 flex flex-col">
          <span className="text-neutral-400 text-[10px]">RMS ACCELERATION</span>
          <strong className="text-lg font-bold text-[#F5C544]">
            {telemetry.rms.toFixed(3)}g{" "}
            <span className="text-[10px] text-neutral-400 font-normal">/ {warnRms}g lim</span>
          </strong>
        </div>
        <div className="bg-[#161B24] p-3 rounded-2xl border border-white/5 flex flex-col">
          <span className="text-neutral-400 text-[10px]">FUNDAMENTAL FREQ</span>
          <strong className="text-lg font-bold text-[#00F0FF]">
            {(telemetry.f0 || targetHz).toFixed(1)} Hz
          </strong>
        </div>
        <div className="bg-[#161B24] p-3 rounded-2xl border border-white/5 flex flex-col">
          <span className="text-neutral-400 text-[10px]">KURTOSIS FACTOR</span>
          <strong className={`text-lg font-bold ${telemetry.kurt > kurtThresh ? "text-rose-400" : "text-emerald-400"}`}>
            {telemetry.kurt.toFixed(2)}{" "}
            <span className="text-[10px] text-neutral-400 font-normal">/ &lt;{kurtThresh}</span>
          </strong>
        </div>
        <div className="bg-[#161B24] p-3 rounded-2xl border border-white/5 flex flex-col">
          <span className="text-neutral-400 text-[10px]">ISO VIBRATION VELOCITY</span>
          <strong className="text-lg font-bold text-white">
            {telemetry.iso.toFixed(2)} mm/s ({machine.isoClass})
          </strong>
        </div>
      </div>
    </div>
  );
};
