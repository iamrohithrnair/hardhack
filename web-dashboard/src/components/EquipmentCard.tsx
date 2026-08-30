"use client";

import React, { useState } from "react";
import { ChevronDown, ChevronUp, Cpu, Activity, ShieldCheck, Settings2 } from "lucide-react";
import { TelemetryData } from "../hooks/useDeviceStream";
import { MachineProfile } from "../types/machine";

interface EquipmentCardProps {
  telemetry: TelemetryData;
  machine: MachineProfile;
  onOpenCustomizer: () => void;
}

export const EquipmentCard: React.FC<EquipmentCardProps> = ({
  telemetry,
  machine,
  onOpenCustomizer
}) => {
  const [isHardwareOpen, setIsHardwareOpen] = useState(true);

  const isHealthy = telemetry.score >= 70;
  const isCritical = telemetry.score < 30;

  return (
    <div className="bg-white rounded-[28px] p-6 border border-black/5 shadow-[0_12px_32px_rgba(0,0,0,0.03)] flex flex-col relative justify-between">
      <div>
        {/* Machine Photo Container */}
        <div className="w-full h-44 rounded-2xl overflow-hidden relative mb-4 shadow-inner group">
          <img
            src={machine.image}
            alt={machine.name}
            className="w-full h-full object-cover group-hover:scale-105 transition-transform duration-300"
          />
          {/* Floating Status Pill */}
          <div
            className={`absolute bottom-3 left-3 px-3 py-1.5 rounded-full text-[11px] font-semibold flex items-center gap-1.5 backdrop-blur-md text-white transition-all ${
              isCritical
                ? "bg-rose-600/90 shadow-[0_0_12px_rgba(244,63,94,0.6)]"
                : isHealthy
                ? "bg-[#1C1F26]/85 shadow-sm"
                : "bg-amber-600/90"
            }`}
          >
            <span
              className={`w-1.5 h-1.5 rounded-full ${
                isCritical
                  ? "bg-white animate-ping"
                  : isHealthy
                  ? "bg-emerald-400 shadow-[0_0_6px_rgba(52,211,153,0.8)]"
                  : "bg-amber-300"
              }`}
            />
            <span>
              {isCritical
                ? "Critical Imbalance"
                : isHealthy
                ? "Healthy Operation"
                : "Vibration Warning"}
            </span>
          </div>

          {/* 1000Hz IMU Tag */}
          <div className="absolute bottom-3 right-3 bg-white/90 backdrop-blur-sm px-2.5 py-1 rounded-full text-[10px] font-bold text-[#12141A] shadow-xs">
            1000 Hz IMU
          </div>
        </div>

        {/* Equipment Details & Quick Switcher */}
        <div className="flex justify-between items-start mb-4">
          <div>
            <h3 className="text-base font-bold text-[#12141A] leading-tight">{machine.name}</h3>
            <p className="text-xs text-[#6B7280]">{machine.motorType}</p>
          </div>
          <button
            onClick={onOpenCustomizer}
            className="p-2 rounded-xl bg-neutral-100 hover:bg-[#12141A] hover:text-white text-[#6B7280] transition-all cursor-pointer shadow-xs"
            title="Configure / Switch Machine"
          >
            <Settings2 className="w-4 h-4" />
          </button>
        </div>

        {/* Accordion Specs */}
        <div className="flex flex-col gap-2">
          <div className="rounded-xl bg-[#F9FAFB] p-2.5 text-xs font-semibold flex justify-between items-center">
            <span className="text-[#6B7280]">Vibration Velocity</span>
            <span className="font-mono text-[#12141A] font-bold">{telemetry.iso.toFixed(2)} mm/s</span>
          </div>

          <div className="rounded-xl bg-[#F9FAFB] p-2.5 text-xs font-semibold">
            <button
              onClick={() => setIsHardwareOpen(!isHardwareOpen)}
              className="w-full flex justify-between items-center cursor-pointer"
            >
              <span>Paired Hardware</span>
              <div className="flex items-center gap-1.5">
                <span className="bg-[#1C1F26] text-white px-2 py-0.5 rounded-full text-[10px]">
                  AMOLED 1.8"
                </span>
                {isHardwareOpen ? <ChevronUp className="w-3.5 h-3.5" /> : <ChevronDown className="w-3.5 h-3.5" />}
              </div>
            </button>

            {isHardwareOpen && (
              <div className="mt-2 pt-2 border-t border-black/5 flex items-center gap-2.5">
                <div className="w-8 h-8 rounded-lg bg-white flex items-center justify-center shadow-xs text-sky-500">
                  <Cpu className="w-4 h-4" />
                </div>
                <div className="flex flex-col">
                  <span className="text-[11px] font-bold text-[#12141A]">Waveshare ESP32-S3</span>
                  <span className="text-[10px] text-[#6B7280]">QMI8658 + ES8311 Codec</span>
                </div>
              </div>
            )}
          </div>

          <div className="rounded-xl bg-[#F9FAFB] p-2.5 text-xs font-semibold flex justify-between items-center">
            <span className="text-[#6B7280]">Bearing Assembly</span>
            <span className="text-emerald-600 font-bold">
              {machine.bearingType || "Dual Sealed Precision"}
            </span>
          </div>
        </div>
      </div>
    </div>
  );
};
