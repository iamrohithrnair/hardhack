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
  const fftHistoryRef = useRef<number[]>(new Array(24).fill(0.05));
  const fftPeaksRef = useRef<number[]>(new Array(24).fill(0.05));
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

      // Generate physics wave sample based on live telemetry state
      phaseRef.current += 0.14;
      const noise = (Math.random() - 0.5) * 0.04;
      let sample = 0;

      if (telemetry.state === 1) {
        sample = 0.28 * Math.sin(phaseRef.current) + noise;
      } else {
        const mod = 1.0 + 0.3 * Math.sin(phaseRef.current * 0.2);
        sample = 0.85 * Math.sin(phaseRef.current) * mod + 0.3 * Math.sin(phaseRef.current * 2) + noise * 4;
      }

      waveHistoryRef.current.shift();
      waveHistoryRef.current.push(sample);

      // 24-Band FFT Bars in Background
      const BARS = 24;
      const barWidth = (w - (BARS - 1) * 6) / BARS;

      for (let i = 0; i < BARS; i++) {
        let target = 0.04;
        if (telemetry.state === 1) {
          if (i === 4) target = 0.75;
          if (i === 8) target = 0.15;
        } else {
          if (i === 4) target = 0.95;
          if (i === 8) target = 0.65;
          if (i === 12) target = 0.45;
        }

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

      // Draw Main Micro-Vibration Trace
      const traceColor = isFault ? "#F43F5E" : "#0EA5E9";
      ctx.strokeStyle = traceColor;
      ctx.lineWidth = 2.5;
      ctx.shadowColor = traceColor;
      ctx.shadowBlur = 8;
      ctx.beginPath();

      const points = waveHistoryRef.current;
      for (let i = 0; i < points.length; i++) {
        const x = (i / points.length) * w;
        const y = midY - points[i] * (h * 0.4);
        if (i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      }
      ctx.stroke();
      ctx.shadowBlur = 0;

      animationFrameId = requestAnimationFrame(render);
    };

    render();
    return () => cancelAnimationFrame(animationFrameId);
  }, [telemetry.state, isFault]);

  return (
    <section className="bg-white rounded-[28px] p-6 border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col gap-4">
      {/* Header with Month Navigation & Legend */}
      <div className="flex flex-col sm:flex-row justify-between items-start sm:items-center gap-3">
        <div className="flex items-center gap-3.5">
          <span className="text-xs text-[#9CA3AF] font-semibold hover:text-[#12141A] cursor-pointer">
            August
          </span>
          <span className="text-sm font-bold text-[#12141A]">September 2026</span>
          <span className="text-xs text-[#9CA3AF] font-semibold hover:text-[#12141A] cursor-pointer">
            October
          </span>
        </div>

        <div className="flex items-center gap-4 text-[11px] font-semibold text-[#6B7280]">
          <div className="flex items-center gap-1.5">
            <span className="w-2 h-2 rounded-full bg-sky-500" />
            <span>Micro-Vibration Trace</span>
          </div>
          <div className="flex items-center gap-1.5">
            <span className="w-2 h-2 rounded-full bg-[#F5C544]" />
            <span>24-Band FFT Spectrum</span>
          </div>
          {isFault && (
            <div className="flex items-center gap-1.5 text-rose-600">
              <span className="w-2 h-2 rounded-full bg-rose-500 animate-ping" />
              <span>Critical Anomaly</span>
            </div>
          )}
        </div>
      </div>

      {/* Canvas Viewport with Floating Event Pills */}
      <div className="w-full h-36 bg-[#0D1017] rounded-2xl overflow-hidden relative shadow-inner">
        <canvas
          ref={canvasRef}
          width={1200}
          height={144}
          className="w-full h-full block"
        />

        {/* Floating Timeline Event Pills */}
        <div className="absolute top-3 left-[18%] bg-[#1C1F26]/90 border border-white/15 border-l-4 border-l-sky-400 backdrop-blur-md px-3.5 py-1.5 rounded-full text-white shadow-md pointer-events-none flex flex-col">
          <span className="text-[11px] font-bold">Baseline Lock</span>
          <span className="text-[9px] text-[#9CA3AF]">F0: 48.5 Hz · Smooth Sine Wave</span>
        </div>

        {isFault && (
          <div className="absolute top-3 left-[62%] bg-rose-950/90 border border-rose-500/30 border-l-4 border-l-rose-500 backdrop-blur-md px-3.5 py-1.5 rounded-full text-white shadow-lg pointer-events-none flex flex-col animate-bounce">
            <span className="text-[11px] font-bold text-rose-300">Rotor Imbalance Spike</span>
            <span className="text-[9px] text-rose-200">+18 dB 1X Harmonics · Health 18%</span>
          </div>
        )}
      </div>

      {/* Timeline Axis Labels */}
      <div className="flex justify-between text-[11px] text-[#9CA3AF] font-semibold px-2">
        <span>00:00s</span>
        <span>00:15s (Exam Start)</span>
        <span>00:30s (Nominal Lock)</span>
        <span>00:45s (Fault Induced)</span>
        <span>01:00s (Diagnosis Complete)</span>
      </div>
    </section>
  );
};
