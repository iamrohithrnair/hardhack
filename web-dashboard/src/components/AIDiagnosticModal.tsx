"use client";

import React, { useState } from "react";
import { X, Sparkles, AlertTriangle, ShieldCheck, CheckCircle2, ArrowRight } from "lucide-react";
import { TelemetryData } from "../hooks/useDeviceStream";

interface AIDiagnosticModalProps {
  isOpen: boolean;
  onClose: () => void;
  telemetry: TelemetryData;
}

export const AIDiagnosticModal: React.FC<AIDiagnosticModalProps> = ({
  isOpen,
  onClose,
  telemetry
}) => {
  const [loading, setLoading] = useState(false);
  const [report, setReport] = useState<any>(null);

  if (!isOpen) return null;

  const handleGenerateReport = async () => {
    setLoading(true);
    try {
      const res = await fetch("/api/ai/diagnose", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(telemetry)
      });
      const data = await res.json();
      if (data.success) {
        setReport(data.diagnosis);
      }
    } catch (e) {
      console.error("AI diagnosis error:", e);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/40 backdrop-blur-sm animate-fade-in">
      <div className="bg-white rounded-[32px] max-w-xl w-full p-8 border border-black/5 shadow-[0_24px_48px_rgba(0,0,0,0.12)] flex flex-col gap-6 relative max-h-[90vh] overflow-y-auto">
        {/* Close Button */}
        <button
          onClick={onClose}
          className="absolute top-6 right-6 w-8 h-8 rounded-full bg-neutral-100 hover:bg-neutral-200 flex items-center justify-center text-[#12141A] transition-all cursor-pointer"
        >
          <X className="w-4 h-4" />
        </button>

        {/* Header */}
        <div className="flex items-center gap-3">
          <div className="w-12 h-12 rounded-2xl bg-amber-100 flex items-center justify-center text-amber-600 shadow-xs">
            <Sparkles className="w-6 h-6" />
          </div>
          <div>
            <h2 className="text-xl font-extrabold text-[#12141A]">
              AI Machine Doctor Diagnostic
            </h2>
            <p className="text-xs text-[#6B7280]">
              Real-time FFT Harmonics & ISO 10816 Copilot
            </p>
          </div>
        </div>

        {/* Current Snapshot */}
        <div className="grid grid-cols-3 gap-3 p-3.5 rounded-2xl bg-[#F9FAFB] border border-black/5">
          <div>
            <span className="text-[10px] text-[#9CA3AF] font-bold">HEALTH</span>
            <div className="text-lg font-mono font-bold text-[#12141A]">{telemetry.score}%</div>
          </div>
          <div>
            <span className="text-[10px] text-[#9CA3AF] font-bold">1X RPM PEAK</span>
            <div className="text-lg font-mono font-bold text-[#0EA5E9]">{telemetry.f0.toFixed(1)} Hz</div>
          </div>
          <div>
            <span className="text-[10px] text-[#9CA3AF] font-bold">KURTOSIS</span>
            <div className="text-lg font-mono font-bold text-amber-600">{telemetry.kurt.toFixed(1)}</div>
          </div>
        </div>

        {/* Action Button */}
        {!report && (
          <button
            onClick={handleGenerateReport}
            disabled={loading}
            className="w-full py-3.5 rounded-2xl bg-[#1C1F26] hover:bg-black text-white font-bold text-sm flex items-center justify-center gap-2 shadow-md transition-all cursor-pointer disabled:opacity-50"
          >
            {loading ? (
              <span className="animate-pulse">Analyzing Micro-Vibrations...</span>
            ) : (
              <>
                <span>Generate Prescriptive Diagnostic Report</span>
                <ArrowRight className="w-4 h-4" />
              </>
            )}
          </button>
        )}

        {/* Report Output */}
        {report && (
          <div className="flex flex-col gap-4 animate-fade-in">
            {/* Fault Banner */}
            <div
              className={`p-4 rounded-2xl border flex items-start gap-3 ${
                report.healthScore < 50
                  ? "bg-rose-50 border-rose-200 text-rose-900"
                  : "bg-emerald-50 border-emerald-200 text-emerald-900"
              }`}
            >
              {report.healthScore < 50 ? (
                <AlertTriangle className="w-5 h-5 text-rose-600 shrink-0 mt-0.5" />
              ) : (
                <ShieldCheck className="w-5 h-5 text-emerald-600 shrink-0 mt-0.5" />
              )}
              <div className="flex flex-col">
                <span className="font-bold text-sm">{report.faultType}</span>
                <span className="text-xs opacity-90 mt-0.5">{report.rootCause}</span>
              </div>
            </div>

            {/* Lifetime RUL */}
            <div className="flex justify-between items-center p-3.5 rounded-xl bg-neutral-100 text-xs font-semibold">
              <span className="text-[#6B7280]">Estimated Remaining Life (RUL):</span>
              <span className="font-bold text-[#12141A]">{report.estimatedRUL}</span>
            </div>

            {/* Recommended Actions */}
            <div>
              <h4 className="text-xs font-bold text-[#12141A] uppercase tracking-wider mb-2">
                Prescriptive Maintenance Checklist
              </h4>
              <div className="flex flex-col gap-2">
                {report.recommendedActions.map((action: string, idx: number) => (
                  <div
                    key={idx}
                    className="flex items-start gap-2.5 p-2.5 rounded-xl bg-[#F9FAFB] border border-black/5 text-xs text-[#12141A]"
                  >
                    <CheckCircle2 className="w-4 h-4 text-emerald-600 shrink-0 mt-0.5" />
                    <span>{action}</span>
                  </div>
                ))}
              </div>
            </div>

            <button
              onClick={handleGenerateReport}
              disabled={loading}
              className="mt-2 py-2.5 rounded-xl bg-neutral-200 hover:bg-neutral-300 text-xs font-bold text-[#12141A] transition-all cursor-pointer"
            >
              Re-evaluate Live Telemetry
            </button>
          </div>
        )}
      </div>
    </div>
  );
};
