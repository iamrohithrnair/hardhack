"use client";

import React, { useState } from "react";
import { ShieldCheck, AlertTriangle, AlertCircle, FileText, CheckCircle2, Award } from "lucide-react";
import { TelemetryData } from "../../hooks/useDeviceStream";
import { MachineProfile } from "../../types/machine";

interface ISOStandardViewProps {
  telemetry: TelemetryData;
  machine: MachineProfile;
}

export const ISOStandardView: React.FC<ISOStandardViewProps> = ({
  telemetry,
  machine
}) => {
  const [selectedClass, setSelectedClass] = useState<string>(machine.isoClass || "Class I");

  const isoVal = telemetry.iso;

  // Determine Zone
  let zone = "Zone A";
  let zoneLabel = "Zone A: Good / New Condition";
  let zoneColor = "text-emerald-700 bg-emerald-50 border-emerald-300";
  let zoneDesc = "Vibration of newly commissioned machines. Vibration is optimal with no maintenance required.";

  if (isoVal > 4.50) {
    zone = "Zone D";
    zoneLabel = "Zone D: Unacceptable / Dangerous";
    zoneColor = "text-rose-700 bg-rose-50 border-rose-300";
    zoneDesc = "Vibration severity is of sufficient magnitude to cause damage to the machine. Immediate shutdown recommended.";
  } else if (isoVal > 2.80) {
    zone = "Zone C";
    zoneLabel = "Zone C: Unsatisfactory for Long-Term Operation";
    zoneColor = "text-amber-700 bg-amber-50 border-amber-300";
    zoneDesc = "Machine may be operated for a limited period until an unscheduled maintenance stoppage can be arranged.";
  } else if (isoVal > 1.12) {
    zone = "Zone B";
    zoneLabel = "Zone B: Acceptable / Unrestricted Operation";
    zoneColor = "text-sky-700 bg-sky-50 border-sky-300";
    zoneDesc = "Machines within this zone are normally considered acceptable for long-term continuous operation.";
  }

  // Calculate thermometer indicator position percentage (0 to 10 mm/s scale)
  const markerPercent = Math.min(100, Math.max(2, (isoVal / 7.1) * 100));

  return (
    <div className="flex flex-col gap-6 animate-fade-in">
      {/* Header Banner */}
      <div className="p-6 rounded-[28px] bg-white border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col sm:flex-row justify-between items-start sm:items-center gap-4">
        <div className="flex items-center gap-3.5">
          <div className="w-12 h-12 rounded-2xl bg-emerald-50 flex items-center justify-center text-emerald-600">
            <Award className="w-6 h-6" />
          </div>
          <div>
            <h2 className="text-xl font-extrabold text-[#12141A]">
              ISO 10816 Industrial Vibration Evaluation Matrix
            </h2>
            <p className="text-xs text-[#6B7280]">
              Evaluation of machine vibration by measurements on non-rotating parts ({machine.name})
            </p>
          </div>
        </div>

        {/* Current ISO Badge */}
        <div className={`px-4 py-2 rounded-full border text-xs font-bold ${zoneColor}`}>
          {zoneLabel}
        </div>
      </div>

      {/* ISO Scale Thermometer Card */}
      <div className="p-6 rounded-[28px] bg-white border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col gap-4">
        <div className="flex justify-between items-center">
          <span className="text-sm font-bold text-[#12141A]">
            Current Operating Severity: <strong className="font-mono text-base">{isoVal.toFixed(2)} mm/s RMS</strong>
          </span>
          <span className="text-xs text-[#6B7280] font-semibold">Standard: ISO 10816-3 (10Hz – 1000Hz)</span>
        </div>

        {/* Visual Severity Scale */}
        <div className="relative pt-6 pb-2">
          {/* Active Operating Pointer */}
          <div
            className="absolute top-0 -translate-x-1/2 flex flex-col items-center transition-all duration-500 z-10"
            style={{ left: `${markerPercent}%` }}
          >
            <span className="bg-[#1C1F26] text-white text-[10px] font-bold px-2 py-0.5 rounded shadow-md whitespace-nowrap">
              Live: {isoVal.toFixed(2)} mm/s
            </span>
            <div className="w-0 h-0 border-l-4 border-l-transparent border-r-4 border-r-transparent border-t-4 border-t-[#1C1F26]" />
          </div>

          {/* Color Gradient Track */}
          <div className="h-6 rounded-full overflow-hidden flex shadow-inner border border-black/5">
            <div className="bg-emerald-400 flex-1 flex items-center justify-center text-[9px] font-bold text-emerald-950">
              Zone A (&lt;1.12)
            </div>
            <div className="bg-sky-400 flex-1 flex items-center justify-center text-[9px] font-bold text-sky-950">
              Zone B (1.12-2.8)
            </div>
            <div className="bg-amber-400 flex-1 flex items-center justify-center text-[9px] font-bold text-amber-950">
              Zone C (2.8-4.5)
            </div>
            <div className="bg-rose-500 flex-1 flex items-center justify-center text-[9px] font-bold text-white">
              Zone D (&gt;4.5)
            </div>
          </div>
        </div>
      </div>

      {/* ISO Evaluation Matrix Table Card */}
      <div className="p-6 rounded-[28px] bg-white border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col gap-4">
        <h3 className="text-base font-bold text-[#12141A]">
          ISO 10816-3 Classification Matrix
        </h3>

        <div className="overflow-x-auto">
          <table className="w-full text-left text-xs border-collapse">
            <thead>
              <tr className="border-b border-black/5 text-[#6B7280] font-bold">
                <th className="py-3 px-4">Severity (mm/s RMS)</th>
                <th className="py-3 px-4">Class I (Small &lt;15kW)</th>
                <th className="py-3 px-4">Class II (Medium 15-75kW)</th>
                <th className="py-3 px-4">Class III (Large Rigid)</th>
                <th className="py-3 px-4">Class IV (Large Soft)</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-black/5 font-semibold">
              <tr className={isoVal < 0.71 ? "bg-emerald-50/80 font-bold" : ""}>
                <td className="py-2.5 px-4 font-mono">0.28 – 0.71</td>
                <td className="py-2.5 px-4 text-emerald-700">Zone A (Good)</td>
                <td className="py-2.5 px-4 text-emerald-700">Zone A (Good)</td>
                <td className="py-2.5 px-4 text-emerald-700">Zone A (Good)</td>
                <td className="py-2.5 px-4 text-emerald-700">Zone A (Good)</td>
              </tr>
              <tr className={isoVal >= 0.71 && isoVal < 1.12 ? "bg-emerald-50/80 font-bold" : ""}>
                <td className="py-2.5 px-4 font-mono">0.71 – 1.12</td>
                <td className="py-2.5 px-4 text-emerald-700">Zone A (Good)</td>
                <td className="py-2.5 px-4 text-emerald-700">Zone A (Good)</td>
                <td className="py-2.5 px-4 text-emerald-700">Zone A (Good)</td>
                <td className="py-2.5 px-4 text-emerald-700">Zone A (Good)</td>
              </tr>
              <tr className={isoVal >= 1.12 && isoVal < 2.80 ? "bg-sky-50/80 font-bold" : ""}>
                <td className="py-2.5 px-4 font-mono">1.12 – 2.80</td>
                <td className="py-2.5 px-4 text-sky-700">Zone B (Acceptable)</td>
                <td className="py-2.5 px-4 text-sky-700">Zone B (Acceptable)</td>
                <td className="py-2.5 px-4 text-sky-700">Zone B (Acceptable)</td>
                <td className="py-2.5 px-4 text-sky-700">Zone B (Acceptable)</td>
              </tr>
              <tr className={isoVal >= 2.80 && isoVal < 4.50 ? "bg-amber-50/80 font-bold" : ""}>
                <td className="py-2.5 px-4 font-mono">2.80 – 4.50</td>
                <td className="py-2.5 px-4 text-amber-700">Zone C (Warning)</td>
                <td className="py-2.5 px-4 text-amber-700">Zone C (Warning)</td>
                <td className="py-2.5 px-4 text-sky-700">Zone B (Acceptable)</td>
                <td className="py-2.5 px-4 text-sky-700">Zone B (Acceptable)</td>
              </tr>
              <tr className={isoVal >= 4.50 ? "bg-rose-50/80 font-bold" : ""}>
                <td className="py-2.5 px-4 font-mono">&gt; 4.50</td>
                <td className="py-2.5 px-4 text-rose-700 font-bold">Zone D (Critical)</td>
                <td className="py-2.5 px-4 text-rose-700 font-bold">Zone D (Critical)</td>
                <td className="py-2.5 px-4 text-rose-700 font-bold">Zone D (Critical)</td>
                <td className="py-2.5 px-4 text-amber-700">Zone C (Warning)</td>
              </tr>
            </tbody>
          </table>
        </div>

        {/* Mitigation Instructions */}
        <div className={`p-4 rounded-2xl border flex items-start gap-3 mt-2 ${zoneColor}`}>
          <AlertCircle className="w-5 h-5 shrink-0 mt-0.5" />
          <div className="flex flex-col">
            <span className="font-bold text-sm">ISO Evaluation Protocol</span>
            <p className="text-xs mt-0.5 opacity-90 leading-relaxed">{zoneDesc}</p>
          </div>
        </div>
      </div>
    </div>
  );
};
