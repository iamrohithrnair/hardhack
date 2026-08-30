"use client";

import React, { useState } from "react";
import { X, Sparkles, AlertTriangle, ShieldCheck, CheckCircle2, ArrowRight, Bot } from "lucide-react";
import { TelemetryData } from "../hooks/useDeviceStream";
import { MachineProfile } from "../types/machine";

interface AIDiagnosticModalProps {
  isOpen: boolean;
  onClose: () => void;
  telemetry: TelemetryData;
  machine?: MachineProfile;
}

export const AIDiagnosticModal: React.FC<AIDiagnosticModalProps> = ({
  isOpen,
  onClose,
  telemetry,
  machine
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
        body: JSON.stringify({ ...telemetry, machine })
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
              AI Doctor & Anomaly Diagnostic
            </h2>
            <p className="text-xs text-[#6B7280]">
              Target: <strong className="text-[#12141A]">{machine?.name || "Asset"}</strong> ({machine?.category || "Industrial"})
            </p>
          </div>
        </div>

        {/* Action button if no report yet */}
        {!report && !loading && (
          <div className="flex flex-col items-center justify-center gap-4 py-8 text-center bg-neutral-50 rounded-2xl p-6 border border-neutral-100">
            <div className="w-14 h-14 rounded-full bg-[#1C1F26] text-[#F5C544] flex items-center justify-center shadow-md">
              <Bot className="w-7 h-7" />
            </div>
            <div>
              <h3 className="text-base font-bold text-[#12141A]">Ready for Multimodal Physical Audit</h3>
              <p className="text-xs text-[#6B7280] max-w-md mt-1">
                Gemini 3.7 & GPT-5.6 will analyze real-time spectral FFT harmonics, RMS energy ({telemetry.rms.toFixed(3)}g), and Kurtosis factor ({telemetry.kurt.toFixed(2)}) for <strong>{machine?.name}</strong>.
              </p>
            </div>
            <button
              onClick={handleGenerateReport}
              className="px-6 py-3 rounded-full bg-[#1C1F26] hover:bg-black text-white font-bold text-sm shadow-md transition-all cursor-pointer flex items-center gap-2"
            >
              <Sparkles className="w-4 h-4 text-[#F5C544]" />
              Run Full AI Diagnosis
            </button>
          </div>
        )}

        {/* Loading Spinner */}
        {loading && (
          <div className="flex flex-col items-center justify-center gap-3 py-12 text-center">
            <div className="w-10 h-10 border-3 border-amber-500 border-t-transparent rounded-full animate-spin" />
            <span className="text-xs font-bold text-[#12141A]">
              Synthesizing acoustic spectral signatures & physical harmonics...
            </span>
          </div>
        )}

        {/* Report Display */}
        {report && (
          <div className="flex flex-col gap-4 animate-fade-in">
            {/* Fault Title Card */}
            <div className="p-4 rounded-2xl bg-[#1C1F26] text-white flex flex-col gap-2 shadow-sm">
              <div className="flex justify-between items-center text-xs">
                <span className="font-bold text-amber-400 uppercase tracking-wider">
                  Diagnosis Result
                </span>
                <span className="px-2 py-0.5 rounded-full bg-white/10 text-white font-mono text-[10px]">
                  {report.isoClass}
                </span>
              </div>
              <h3 className="text-lg font-black tracking-tight">{report.faultType}</h3>
              <div className="text-xs font-bold text-neutral-300">
                Urgency: <span className="text-rose-400">{report.urgency}</span> · RUL: {report.estimatedRUL}
              </div>
            </div>

            {/* Root Cause */}
            <div className="p-4 rounded-2xl bg-neutral-50 border border-neutral-100 flex flex-col gap-1">
              <h4 className="text-xs font-bold text-[#12141A] uppercase tracking-wider">Root Cause Analysis</h4>
              <p className="text-xs text-[#4B5563] leading-relaxed">{report.rootCause}</p>
            </div>

            {/* Recommended Action Checklist */}
            <div className="flex flex-col gap-2">
              <h4 className="text-xs font-bold text-[#12141A] uppercase tracking-wider">Prescribed Action Steps</h4>
              <div className="flex flex-col gap-2">
                {report.recommendedActions.map((action: string, idx: number) => (
                  <div key={idx} className="flex items-start gap-2.5 p-3 rounded-xl bg-neutral-50 border border-neutral-100 text-xs">
                    <CheckCircle2 className="w-4 h-4 text-emerald-500 shrink-0 mt-0.5" />
                    <span className="text-[#374151] font-medium">{action}</span>
                  </div>
                ))}
              </div>
            </div>

            <button
              onClick={handleGenerateReport}
              className="mt-2 py-2.5 rounded-full border border-neutral-200 hover:bg-neutral-100 text-[#12141A] font-bold text-xs transition-all cursor-pointer"
            >
              Re-Analyze Live Stream
            </button>
          </div>
        )}
      </div>
    </div>
  );
};
