"use client";

import React, { useState } from "react";
import { X, Plus, Check, Settings2, Sparkles, Cpu, Bot, Waves, Radio, Activity } from "lucide-react";
import { MachineProfile, DEFAULT_MACHINES, PRESET_DEVICE_TEMPLATES } from "../types/machine";

interface DeviceCustomizerModalProps {
  isOpen: boolean;
  onClose: () => void;
  currentMachine: MachineProfile;
  onSelectMachine: (machine: MachineProfile) => void;
  onOpenAIProfiler?: () => void;
}

export const DeviceCustomizerModal: React.FC<DeviceCustomizerModalProps> = ({
  isOpen,
  onClose,
  currentMachine,
  onSelectMachine,
  onOpenAIProfiler
}) => {
  const [machines, setMachines] = useState<MachineProfile[]>(() => {
    // Merge default and preset templates
    const presetsAsProfiles: MachineProfile[] = PRESET_DEVICE_TEMPLATES.map((p, idx) => ({
      id: p.id || `preset-${idx}`,
      name: p.name || "Custom Asset",
      category: p.category || "General Asset",
      motorType: p.motorType || "Dynamic Transducer",
      nominalRPM: p.nominalRPM || 0,
      fundamentalHz: p.fundamentalHz || 10.0,
      isoClass: p.isoClass || "Class I",
      warningRms: p.warningRms || 0.25,
      criticalRms: p.criticalRms || 0.85,
      image: p.image || "https://images.unsplash.com/photo-1581092160607-ee22621dd758?w=600&auto=format&fit=crop&q=80",
      bearingType: p.bearingType || "Industrial Precision",
      mountTorque: p.mountTorque || "15 Nm",
      detectionTarget: p.detectionTarget || "vibration",
      physicsSummary: p.physicsSummary,
      anomalyRules: p.anomalyRules
    }));

    return [...DEFAULT_MACHINES, ...presetsAsProfiles];
  });

  if (!isOpen) return null;

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/40 backdrop-blur-sm animate-fade-in">
      <div className="bg-white rounded-[32px] max-w-3xl w-full p-8 border border-black/5 shadow-[0_24px_48px_rgba(0,0,0,0.15)] flex flex-col gap-6 relative max-h-[90vh] overflow-y-auto">
        {/* Close Button */}
        <button
          onClick={onClose}
          className="absolute top-6 right-6 w-8 h-8 rounded-full bg-neutral-100 hover:bg-neutral-200 flex items-center justify-center text-[#12141A] transition-all cursor-pointer"
        >
          <X className="w-4 h-4" />
        </button>

        {/* Header Row */}
        <div className="flex items-center justify-between gap-4">
          <div className="flex items-center gap-3">
            <div className="w-12 h-12 rounded-2xl bg-[#F5C544]/20 flex items-center justify-center text-[#D4A322] shadow-xs">
              <Settings2 className="w-6 h-6" />
            </div>
            <div>
              <h2 className="text-xl font-extrabold text-[#12141A]">
                Asset & Anomaly Detector Library
              </h2>
              <p className="text-xs text-[#6B7280]">
                Configure Mecha-Whisperer for any machine, seismic event, or biomechanical motion
              </p>
            </div>
          </div>

          {/* AI Profiler Trigger Button */}
          {onOpenAIProfiler && (
            <button
              onClick={() => {
                onClose();
                onOpenAIProfiler();
              }}
              className="px-4 py-2.5 rounded-2xl bg-gradient-to-r from-amber-500 to-yellow-500 hover:from-amber-400 hover:to-yellow-400 text-black font-extrabold text-xs shadow-md transition-all cursor-pointer flex items-center gap-2"
            >
              <Sparkles className="w-4 h-4" />
              + AI Profile Custom Device
            </button>
          )}
        </div>

        {/* Machine Profiles Grid */}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          {machines.map((m) => {
            const isSelected = currentMachine.id === m.id;
            return (
              <div
                key={m.id}
                onClick={() => {
                  onSelectMachine(m);
                  onClose();
                }}
                className={`p-4 rounded-2xl border transition-all cursor-pointer flex flex-col justify-between gap-3 ${
                  isSelected
                    ? "border-[#F5C544] bg-[#FFFBEB] ring-2 ring-[#F5C544]/50 shadow-md"
                    : "border-neutral-200 bg-white hover:border-neutral-300 hover:shadow-xs"
                }`}
              >
                <div className="flex items-start justify-between gap-3">
                  <div className="flex items-center gap-3">
                    <img
                      src={m.image}
                      alt={m.name}
                      className="w-12 h-12 rounded-xl object-cover border border-black/5"
                    />
                    <div>
                      <h4 className="text-sm font-bold text-[#12141A] leading-snug">{m.name}</h4>
                      <span className="text-[11px] font-semibold text-[#6B7280]">{m.category}</span>
                    </div>
                  </div>

                  {isSelected && (
                    <span className="w-6 h-6 rounded-full bg-[#F5C544] text-black flex items-center justify-center shrink-0">
                      <Check className="w-3.5 h-3.5 stroke-[3]" />
                    </span>
                  )}
                </div>

                {m.physicsSummary && (
                  <p className="text-[11px] text-[#4B5563] line-clamp-2 italic">
                    "{m.physicsSummary}"
                  </p>
                )}

                <div className="grid grid-cols-3 gap-2 pt-2 border-t border-black/5 text-[10px] font-mono text-[#6B7280]">
                  <div>
                    <span>TARGET:</span>
                    <strong className="block text-[#12141A] font-bold uppercase">{m.detectionTarget || "vibration"}</strong>
                  </div>
                  <div>
                    <span>FREQ:</span>
                    <strong className="block text-[#12141A] font-bold">{m.fundamentalHz} Hz</strong>
                  </div>
                  <div>
                    <span>ISO CLASS:</span>
                    <strong className="block text-[#12141A] font-bold">{m.isoClass}</strong>
                  </div>
                </div>
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
};
