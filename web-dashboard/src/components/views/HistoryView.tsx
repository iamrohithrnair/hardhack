"use client";

import React, { useState } from "react";
import { Download, History, CheckCircle2, AlertTriangle, ShieldCheck, Clock } from "lucide-react";
import { TelemetryData } from "../../hooks/useDeviceStream";
import { MachineProfile } from "../../types/machine";

interface HistoryViewProps {
  telemetry: TelemetryData;
  machine: MachineProfile;
}

export const HistoryView: React.FC<HistoryViewProps> = ({
  telemetry,
  machine
}) => {
  const [records] = useState([
    {
      id: "REC-1049",
      time: "Just now (Live)",
      rpm: telemetry.rpm,
      rms: telemetry.rms,
      iso: telemetry.iso,
      kurt: telemetry.kurt,
      status: telemetry.score >= 70 ? "Healthy" : "Anomaly Detected",
      severity: telemetry.score >= 70 ? "Zone A" : "Zone D"
    },
    {
      id: "REC-1048",
      time: "12:58 PM",
      rpm: 2910,
      rms: 0.082,
      iso: 0.16,
      kurt: 2.94,
      status: "Baseline Calibrated",
      severity: "Zone A"
    },
    {
      id: "REC-1047",
      time: "12:50 PM",
      rpm: 2915,
      rms: 0.088,
      iso: 0.18,
      kurt: 2.98,
      status: "Routine Exam",
      severity: "Zone A"
    },
    {
      id: "REC-1046",
      time: "Yesterday, 4:15 PM",
      rpm: 2908,
      rms: 0.081,
      iso: 0.15,
      kurt: 2.92,
      status: "Pre-Shift Check",
      severity: "Zone A"
    }
  ]);

  const handleExportCSV = () => {
    const csvContent = "data:text/csv;charset=utf-8," 
      + "RecordID,Time,RPM,RMS(g),ISO(mm/s),Kurtosis,Status,Severity\n"
      + records.map(r => `${r.id},${r.time},${r.rpm},${r.rms},${r.iso},${r.kurt},${r.status},${r.severity}`).join("\n");
    const encodedUri = encodeURI(csvContent);
    const link = document.createElement("a");
    link.setAttribute("href", encodedUri);
    link.setAttribute("download", `mecha_whisperer_${machine.id}_history.csv`);
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  };

  return (
    <div className="flex flex-col gap-6 animate-fade-in">
      {/* Header */}
      <div className="p-6 rounded-[28px] bg-white border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col sm:flex-row justify-between items-start sm:items-center gap-4">
        <div className="flex items-center gap-3.5">
          <div className="w-12 h-12 rounded-2xl bg-sky-50 flex items-center justify-center text-sky-600">
            <History className="w-6 h-6" />
          </div>
          <div>
            <h2 className="text-xl font-extrabold text-[#12141A]">
              Inspection History & Telemetry Log
            </h2>
            <p className="text-xs text-[#6B7280]">
              Historical records and baseline trends for {machine.name}
            </p>
          </div>
        </div>

        <button
          onClick={handleExportCSV}
          className="flex items-center gap-2 px-4 py-2 rounded-full bg-[#1C1F26] hover:bg-black text-white text-xs font-bold transition-all cursor-pointer shadow-xs"
        >
          <Download className="w-4 h-4" />
          <span>Export CSV Telemetry</span>
        </button>
      </div>

      {/* Records Table */}
      <div className="p-6 rounded-[28px] bg-white border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col gap-4">
        <h3 className="text-base font-bold text-[#12141A]">Historical Diagnostic Exam Records</h3>

        <div className="overflow-x-auto">
          <table className="w-full text-left text-xs border-collapse">
            <thead>
              <tr className="border-b border-black/5 text-[#6B7280] font-bold">
                <th className="py-3 px-4">Record ID</th>
                <th className="py-3 px-4">Timestamp</th>
                <th className="py-3 px-4">Speed (RPM)</th>
                <th className="py-3 px-4">RMS Accel</th>
                <th className="py-3 px-4">ISO Velocity</th>
                <th className="py-3 px-4">Kurtosis</th>
                <th className="py-3 px-4">Diagnostic Status</th>
                <th className="py-3 px-4">ISO Zone</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-black/5 font-semibold">
              {records.map((r, idx) => (
                <tr key={idx} className={idx === 0 ? "bg-amber-50/40" : ""}>
                  <td className="py-3 px-4 font-mono text-[#12141A] font-bold">{r.id}</td>
                  <td className="py-3 px-4 text-[#6B7280]">{r.time}</td>
                  <td className="py-3 px-4 font-mono">{r.rpm}</td>
                  <td className="py-3 px-4 font-mono">{r.rms} g</td>
                  <td className="py-3 px-4 font-mono">{r.iso} mm/s</td>
                  <td className="py-3 px-4 font-mono">{r.kurt}</td>
                  <td className="py-3 px-4">
                    <span
                      className={`px-2.5 py-1 rounded-full text-[10px] font-bold ${
                        r.status.includes("Anomaly")
                          ? "bg-rose-100 text-rose-800"
                          : "bg-emerald-100 text-emerald-800"
                      }`}
                    >
                      {r.status}
                    </span>
                  </td>
                  <td className="py-3 px-4">
                    <span
                      className={`px-2 py-0.5 rounded text-[10px] font-bold ${
                        r.severity === "Zone A"
                          ? "bg-emerald-50 text-emerald-700"
                          : "bg-rose-50 text-rose-700"
                      }`}
                    >
                      {r.severity}
                    </span>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
};
