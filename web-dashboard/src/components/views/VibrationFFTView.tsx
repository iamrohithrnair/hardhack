"use client";

import React, { useEffect, useRef, useState } from "react";
import { Activity, Zap, BarChart2, Radio, Layers, AlertTriangle, ShieldCheck } from "lucide-react";
import { TelemetryData } from "../../hooks/useDeviceStream";
import { MachineProfile } from "../../types/machine";

interface VibrationFFTViewProps {
  telemetry: TelemetryData;
  machine: MachineProfile;
}

export const VibrationFFTView: React.FC<VibrationFFTViewProps> = ({
  telemetry,
  machine
}) => {
  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const waterfallCanvasRef = useRef<HTMLCanvasElement | null>(null);

  const [scaleMode, setScaleMode] = useState<"linear" | "log_db">("linear");
  const [selectedHarmonic, setSelectedHarmonic] = useState<number | null>(null);

  const f0 = machine.fundamentalHz || 48.5;
  const isFault = telemetry.score < 50;

  // Real-Time 32-Band FFT and Waterfall Animation
  useEffect(() => {
    let animationFrameId: number;
    const BARS = 32;
    const fftBuffer = new Array(BARS).fill(0.05);
    const peakBuffer = new Array(BARS).fill(0.05);
    const waterfallHistory: number[][] = [];
    const MAX_WATERFALL = 80;

    const render = () => {
      const canvas = canvasRef.current;
      const wfCanvas = waterfallCanvasRef.current;
      if (!canvas) return;

      const ctx = canvas.getContext("2d");
      if (!ctx) return;

      const w = canvas.width;
      const h = canvas.height;

      // Clear main FFT canvas
      ctx.fillStyle = "#0B0E14";
      ctx.fillRect(0, 0, w, h);

      // Draw Grid Lines & Frequencies
      ctx.strokeStyle = "rgba(255, 255, 255, 0.06)";
      ctx.lineWidth = 1;
      for (let y = 0; y <= 4; y++) {
        const yPos = (y / 4) * (h - 30) + 10;
        ctx.beginPath();
        ctx.moveTo(0, yPos);
        ctx.lineTo(w, yPos);
        ctx.stroke();
      }

      // Generate Spectral Energy
      const currentFFT = new Array(BARS);
      for (let i = 0; i < BARS; i++) {
        let val = 0.03 + (Math.random() - 0.5) * 0.015;
        const binFreq = (i + 1) * (125 / BARS);

        // Fundamental 1X Peak
        if (Math.abs(binFreq - f0) < 4) {
          val = isFault ? 0.95 : 0.65;
        }
        // 2X Harmonic
        else if (Math.abs(binFreq - f0 * 2) < 4) {
          val = isFault ? 0.60 : 0.18;
        }
        // 3X Harmonic
        else if (Math.abs(binFreq - f0 * 3) < 4) {
          val = isFault ? 0.40 : 0.08;
        }
        // Bearing High-frequency noise if bearing state
        else if (telemetry.state === 4 && binFreq > 70) {
          val = 0.55 + (Math.random() - 0.5) * 0.2;
        }

        fftBuffer[i] = fftBuffer[i] * 0.75 + val * 0.25;
        if (fftBuffer[i] > peakBuffer[i]) peakBuffer[i] = fftBuffer[i];
        else peakBuffer[i] = Math.max(0.02, peakBuffer[i] - 0.008);

        currentFFT[i] = fftBuffer[i];
      }

      // Update Waterfall History
      waterfallHistory.unshift([...currentFFT]);
      if (waterfallHistory.length > MAX_WATERFALL) waterfallHistory.pop();

      // Render 32 FFT Bars
      const barSpacing = 4;
      const barW = (w - (BARS - 1) * barSpacing) / BARS;

      for (let i = 0; i < BARS; i++) {
        const bx = i * (barW + barSpacing);
        let barH = fftBuffer[i] * (h - 40);
        if (scaleMode === "log_db") {
          barH = (Math.log10(fftBuffer[i] * 9 + 1)) * (h - 40);
        }
        const by = h - 30 - barH;

        // Gradient for Bar
        const grad = ctx.createLinearGradient(0, by, 0, h - 30);
        if (isFault) {
          grad.addColorStop(0, "#F43F5E");
          grad.addColorStop(1, "rgba(244, 63, 94, 0.2)");
        } else {
          grad.addColorStop(0, "#F5C544");
          grad.addColorStop(1, "rgba(14, 165, 233, 0.15)");
        }

        ctx.fillStyle = grad;
        ctx.fillRect(bx, by, barW, barH);

        // Peak Hold Top Line
        const py = h - 30 - peakBuffer[i] * (h - 40);
        ctx.fillStyle = isFault ? "#FDA4AF" : "#FDE68A";
        ctx.fillRect(bx, py, barW, 2);

        // Frequency Label
        if (i % 4 === 0) {
          ctx.fillStyle = "#6B7280";
          ctx.font = "9px 'JetBrains Mono', monospace";
          ctx.fillText(`${Math.round((i + 1) * (125 / BARS))}Hz`, bx, h - 10);
        }
      }

      // Render Harmonic Marker Lines (1X, 2X, 3X)
      const harmonics = [
        { mult: 1, label: "1X (RPM)", color: "#0EA5E9" },
        { mult: 2, label: "2X (Align)", color: "#F5C544" },
        { mult: 3, label: "3X (Blade)", color: "#A855F7" }
      ];

      harmonics.forEach((hMarker) => {
        const targetFreq = f0 * hMarker.mult;
        const binIndex = (targetFreq / 125) * BARS - 1;
        if (binIndex >= 0 && binIndex < BARS) {
          const markerX = binIndex * (barW + barSpacing) + barW / 2;

          ctx.strokeStyle = hMarker.color;
          ctx.setLineDash([4, 4]);
          ctx.beginPath();
          ctx.moveTo(markerX, 10);
          ctx.lineTo(markerX, h - 30);
          ctx.stroke();
          ctx.setLineDash([]);

          // Marker Flag
          ctx.fillStyle = hMarker.color;
          ctx.fillRect(markerX - 24, 12, 48, 16);
          ctx.fillStyle = "#FFFFFF";
          ctx.font = "bold 9px sans-serif";
          ctx.fillText(hMarker.label, markerX - 20, 24);
        }
      });

      // Render 2D Waterfall Canvas
      if (wfCanvas) {
        const wfCtx = wfCanvas.getContext("2d");
        if (wfCtx) {
          const wfw = wfCanvas.width;
          const wfh = wfCanvas.height;
          wfCtx.fillStyle = "#0B0E14";
          wfCtx.fillRect(0, 0, wfw, wfh);

          const rowH = wfh / MAX_WATERFALL;
          const colW = wfw / BARS;

          for (let r = 0; r < waterfallHistory.length; r++) {
            const rowData = waterfallHistory[r];
            const ry = r * rowH;

            for (let c = 0; c < BARS; c++) {
              const energy = rowData[c];
              const cx = c * colW;

              // Color heatmap
              if (energy > 0.7) wfCtx.fillStyle = isFault ? "#F43F5E" : "#F5C544";
              else if (energy > 0.4) wfCtx.fillStyle = "#0EA5E9";
              else if (energy > 0.15) wfCtx.fillStyle = "#1E293B";
              else wfCtx.fillStyle = "#0F172A";

              wfCtx.fillRect(cx, ry, colW - 1, rowH);
            }
          }
        }
      }

      animationFrameId = requestAnimationFrame(render);
    };

    render();
    return () => cancelAnimationFrame(animationFrameId);
  }, [f0, isFault, scaleMode, telemetry.state]);

  return (
    <div className="flex flex-col gap-6 animate-fade-in">
      {/* Top Banner Stats */}
      <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
        <div className="p-5 rounded-[24px] bg-white border border-black/5 shadow-xs flex items-center gap-4">
          <div className="w-12 h-12 rounded-2xl bg-sky-50 flex items-center justify-center text-sky-600">
            <Radio className="w-6 h-6" />
          </div>
          <div>
            <span className="text-xs text-[#6B7280] font-semibold">1X Fundamental</span>
            <div className="text-2xl font-extrabold font-mono text-[#12141A]">
              {f0.toFixed(1)} <span className="text-xs font-sans text-[#9CA3AF]">Hz</span>
            </div>
          </div>
        </div>

        <div className="p-5 rounded-[24px] bg-white border border-black/5 shadow-xs flex items-center gap-4">
          <div className="w-12 h-12 rounded-2xl bg-amber-50 flex items-center justify-center text-[#F5C544]">
            <Zap className="w-6 h-6" />
          </div>
          <div>
            <span className="text-xs text-[#6B7280] font-semibold">1X Harmonic Energy</span>
            <div className="text-2xl font-extrabold font-mono text-[#12141A]">
              {isFault ? "88.4%" : "42.1%"}
            </div>
          </div>
        </div>

        <div className="p-5 rounded-[24px] bg-white border border-black/5 shadow-xs flex items-center gap-4">
          <div className="w-12 h-12 rounded-2xl bg-purple-50 flex items-center justify-center text-purple-600">
            <Layers className="w-6 h-6" />
          </div>
          <div>
            <span className="text-xs text-[#6B7280] font-semibold">Total Harmonic Dist.</span>
            <div className="text-2xl font-extrabold font-mono text-[#12141A]">
              {isFault ? "28.6 dB" : "6.2 dB"}
            </div>
          </div>
        </div>

        <div className="p-5 rounded-[24px] bg-white border border-black/5 shadow-xs flex items-center gap-4">
          <div className="w-12 h-12 rounded-2xl bg-rose-50 flex items-center justify-center text-rose-600">
            <AlertTriangle className="w-6 h-6" />
          </div>
          <div>
            <span className="text-xs text-[#6B7280] font-semibold">Spectral Diagnosis</span>
            <div className="text-sm font-bold text-[#12141A] leading-tight">
              {isFault ? "Mass Unbalance Dominant" : "Normal Resonance"}
            </div>
          </div>
        </div>
      </div>

      {/* Main FFT Canvas Card */}
      <div className="p-6 rounded-[28px] bg-white border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col gap-4">
        <div className="flex justify-between items-center">
          <div className="flex items-center gap-2.5">
            <BarChart2 className="w-5 h-5 text-[#F5C544]" />
            <h3 className="text-base font-bold text-[#12141A]">
              Real-Time 32-Band FFT Frequency Spectrum ({machine.name})
            </h3>
          </div>

          <div className="flex items-center gap-2">
            <button
              onClick={() => setScaleMode("linear")}
              className={`px-3 py-1.5 rounded-full text-xs font-bold transition-all cursor-pointer ${
                scaleMode === "linear"
                  ? "bg-[#1C1F26] text-white"
                  : "bg-neutral-100 text-[#6B7280] hover:text-[#12141A]"
              }`}
            >
              Linear
            </button>
            <button
              onClick={() => setScaleMode("log_db")}
              className={`px-3 py-1.5 rounded-full text-xs font-bold transition-all cursor-pointer ${
                scaleMode === "log_db"
                  ? "bg-[#1C1F26] text-white"
                  : "bg-neutral-100 text-[#6B7280] hover:text-[#12141A]"
              }`}
            >
              dB (Log)
            </button>
          </div>
        </div>

        {/* Primary Spectrum Viewport */}
        <div className="w-full h-80 rounded-2xl overflow-hidden bg-[#0B0E14] shadow-inner">
          <canvas ref={canvasRef} width={1200} height={320} className="w-full h-full block" />
        </div>
      </div>

      {/* Waterfall Heatmap Card */}
      <div className="p-6 rounded-[28px] bg-white border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col gap-4">
        <div className="flex justify-between items-center">
          <h3 className="text-sm font-bold text-[#12141A]">
            Spectral Waterfall Heatmap (Time vs Frequency Evolution)
          </h3>
          <span className="text-xs text-[#9CA3AF] font-semibold">Top = Newest · Bottom = Past</span>
        </div>

        <div className="w-full h-44 rounded-2xl overflow-hidden bg-[#0B0E14] shadow-inner">
          <canvas ref={waterfallCanvasRef} width={1200} height={176} className="w-full h-full block" />
        </div>
      </div>
    </div>
  );
};
