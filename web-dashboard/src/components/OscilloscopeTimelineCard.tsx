"use client";

import React, { useEffect, useRef } from "react";
import { TelemetryData } from "../hooks/useDeviceStream";

interface OscilloscopeTimelineCardProps {
  telemetry: TelemetryData;
}

export const OscilloscopeTimelineCard: React.FC<OscilloscopeTimelineCardProps> = ({
  telemetry
}) => {
  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const waveHistoryRef = useRef<number[]>(new Array(200).fill(0));
  const fftHistoryRef = useRef<number[]>(new Array(28).fill(0.05));
  const fftPeaksRef = useRef<number[]>(new Array(28).fill(0.05));
  const phaseRef = useRef<number>(0);

  const isFault = telemetry.score < 50;

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

      // Dynamic angular frequency and amplitude directly driven by hardware telemetry
      const freqMultiplier = Math.max(0.05, (telemetry.f0 || 48.5) / 320.0);
      phaseRef.current += freqMultiplier;

      // Scale amplitude dynamically with RMS
      const rmsAmp = Math.max(0.12, Math.min(1.2, telemetry.rms * 6.0));
      const noise = (Math.random() - 0.5) * (0.04 + telemetry.kurt * 0.01);
      
      let sample = 0;
      if (telemetry.state === 1) {
        sample = rmsAmp * Math.sin(phaseRef.current) + noise;
      } else if (telemetry.state === 3) {
        // Rotor unbalance (heavy 1X dominant component + wobble)
        const mod = 1.0 + 0.35 * Math.sin(phaseRef.current * 0.25);
        sample = rmsAmp * 1.5 * Math.sin(phaseRef.current) * mod + 0.4 * Math.sin(phaseRef.current * 2) + noise * 2;
      } else {
        // Bearing damage (high kurtosis impulse spikes)
        const spike = Math.random() < 0.1 ? (Math.random() - 0.5) * 2.5 : 0;
        sample = rmsAmp * Math.sin(phaseRef.current) + spike + noise * 3;
      }

      waveHistoryRef.current.shift();
      waveHistoryRef.current.push(sample);

      // 28-Band Dynamic Spectral Bars in Background
      const BARS = 28;
      const barWidth = (w - (BARS - 1) * 5) / BARS;

      for (let i = 0; i < BARS; i++) {
        let target = 0.03 + (telemetry.rms * 0.4);
        if (i === 5) target = Math.min(0.95, 0.4 + telemetry.rms * 2.5); // 1X fundamental
        if (i === 10 && telemetry.state !== 1) target = Math.min(0.85, 0.25 + telemetry.rms * 2.0); // 2X harmonic
        if (i === 15 && telemetry.state === 4) target = Math.min(0.75, 0.3 + telemetry.kurt * 0.08); // Bearing resonance

        fftHistoryRef.current[i] = fftHistoryRef.current[i] * 0.85 + target * 0.15;
        if (fftHistoryRef.current[i] > fftPeaksRef.current[i]) {
          fftPeaksRef.current[i] = fftHistoryRef.current[i];
        } else {
          fftPeaksRef.current[i] = Math.max(0, fftPeaksRef.current[i] - 0.015);
        }

        const bx = i * (barWidth + 5);
        const bh = fftHistoryRef.current[i] * (h * 0.75);
        const by = h - bh;

        ctx.fillStyle = isFault ? "rgba(244, 63, 94, 0.25)" : "rgba(245, 197, 68, 0.20)";
        ctx.fillRect(bx, by, barWidth, bh);

        // Peak line
        const py = h - fftPeaksRef.current[i] * (h * 0.75);
        ctx.fillStyle = isFault ? "#F43F5E" : "#F5C544";
        ctx.fillRect(bx, py, barWidth, 2);
      }

      // Draw Main Micro-Vibration Trace
      const traceColor = isFault ? "#F43F5E" : "#0EA5E9";
      ctx.strokeStyle = traceColor;
      ctx.lineWidth = 2.5;
      ctx.shadowColor = traceColor;
      ctx.shadowBlur = 10;
      ctx.beginPath();

      const sliceWidth = w / (waveHistoryRef.current.length - 1);
      for (let i = 0; i < waveHistoryRef.current.length; i++) {
        const val = waveHistoryRef.current[i];
        const y = midY - val * (h * 0.38);
        const x = i * sliceWidth;

        if (i === 0) {
          ctx.moveTo(x, y);
        } else {
          ctx.lineTo(x, y);
        }
      }
      ctx.stroke();
      ctx.shadowBlur = 0; // Reset shadow

      animationFrameId = requestAnimationFrame(render);
    };

    render();

    return () => {
      cancelAnimationFrame(animationFrameId);
    };
  }, [isFault, telemetry.f0, telemetry.kurt, telemetry.rms, telemetry.state]);

  return (
    <div className="bg-[#12141A] rounded-[32px] p-6 lg:p-7 border border-white/10 shadow-2xl flex flex-col gap-5 relative overflow-hidden">
      {/* Top Header Row */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4">
        <div className="flex items-center gap-3">
          <div className="w-3 h-3 rounded-full bg-[#0EA5E9] shadow-[0_0_10px_rgba(14,165,233,0.8)] animate-pulse" />
          <h3 className="text-base font-bold text-white tracking-tight">
            Live Micro-Vibration Oscilloscope & FFT Transducer
          </h3>
          <span className="px-2.5 py-0.5 rounded-full text-[11px] font-mono bg-white/10 text-sky-400 font-bold">
            250 Hz Continuous
          </span>
        </div>

        {/* Live Metrics Badges */}
        <div className="flex items-center gap-2">
          <div className="px-3 py-1 rounded-xl bg-white/5 border border-white/10 text-xs font-mono text-neutral-300">
            F0: <strong className="text-white">{(telemetry.f0 || 48.5).toFixed(1)} Hz</strong>
          </div>
          <div className="px-3 py-1 rounded-xl bg-white/5 border border-white/10 text-xs font-mono text-neutral-300">
            RPM: <strong className="text-[#F5C544]">{telemetry.rpm}</strong>
          </div>
          <div className="px-3 py-1 rounded-xl bg-white/5 border border-white/10 text-xs font-mono text-neutral-300">
            Kurtosis: <strong className={telemetry.kurt > 4 ? "text-rose-400" : "text-emerald-400"}>{telemetry.kurt.toFixed(2)}</strong>
          </div>
        </div>
      </div>

      {/* Canvas Scope */}
      <div className="relative w-full h-48 sm:h-56 rounded-2xl overflow-hidden border border-white/5 bg-[#0D1017]">
        <canvas
          ref={canvasRef}
          width={1000}
          height={260}
          className="w-full h-full object-cover"
        />

        {/* Bottom Left Status Overlay */}
        <div className="absolute bottom-3 left-4 flex items-center gap-2 pointer-events-none">
          <span className="w-2 h-2 rounded-full bg-emerald-400 animate-ping" />
          <span className="text-[11px] font-mono font-bold text-neutral-400 uppercase tracking-wider">
            {telemetry.source === "hardware" ? "HARDWARE TELEMETRY LIVE" : "SIMULATION ENGINE LIVE"}
          </span>
        </div>

        {/* Bottom Right Scale Overlay */}
        <div className="absolute bottom-3 right-4 flex items-center gap-3 text-[10px] font-mono text-neutral-500 pointer-events-none">
          <span>RMS: {telemetry.rms.toFixed(3)}g</span>
          <span>ISO: {telemetry.iso.toFixed(2)} mm/s</span>
          <span>GAIN: 8X</span>
        </div>
      </div>
    </div>
  );
};
