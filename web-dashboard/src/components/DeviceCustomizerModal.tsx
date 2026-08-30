"use client";

import React, { useState } from "react";
import { X, Plus, Check, Settings2, Sliders, Image as ImageIcon, Cpu } from "lucide-react";
import { MachineProfile, DEFAULT_MACHINES } from "../types/machine";

interface DeviceCustomizerModalProps {
  isOpen: boolean;
  onClose: () => void;
  currentMachine: MachineProfile;
  onSelectMachine: (machine: MachineProfile) => void;
}

export const DeviceCustomizerModal: React.FC<DeviceCustomizerModalProps> = ({
  isOpen,
  onClose,
  currentMachine,
  onSelectMachine
}) => {
  const [machines, setMachines] = useState<MachineProfile[]>(DEFAULT_MACHINES);
  const [isEditingCustom, setIsEditingCustom] = useState(false);

  // Form State for custom machine
  const [customName, setCustomName] = useState("Custom CNC Lathe");
  const [customCategory, setCustomCategory] = useState("Industrial Machining");
  const [customMotorType, setCustomMotorType] = useState("3-Phase AC Servo Motor");
  const [customRPM, setCustomRPM] = useState(3000);
  const [customIsoClass, setCustomIsoClass] = useState<"Class I" | "Class II" | "Class III" | "Class IV">("Class II");
  const [customImage, setCustomImage] = useState("https://images.unsplash.com/photo-1581092160607-ee22621dd758?w=600&auto=format&fit=crop&q=80");

  if (!isOpen) return null;

  const handleSaveCustom = (e: React.FormEvent) => {
    e.preventDefault();
    const newProfile: MachineProfile = {
      id: `custom-${Date.now()}`,
      name: customName,
      category: customCategory,
      motorType: customMotorType,
      nominalRPM: Number(customRPM),
      fundamentalHz: Number((customRPM / 60).toFixed(1)),
      isoClass: customIsoClass,
      warningRms: 0.30,
      criticalRms: 0.90,
      image: customImage || "https://images.unsplash.com/photo-1581092160607-ee22621dd758?w=600&auto=format&fit=crop&q=80",
      bearingType: "Custom High-Precision Roller",
      mountTorque: "25.0 Nm"
    };

    setMachines([newProfile, ...machines]);
    onSelectMachine(newProfile);
    setIsEditingCustom(false);
    onClose();
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/40 backdrop-blur-sm animate-fade-in">
      <div className="bg-white rounded-[32px] max-w-2xl w-full p-8 border border-black/5 shadow-[0_24px_48px_rgba(0,0,0,0.15)] flex flex-col gap-6 relative max-h-[90vh] overflow-y-auto">
        {/* Close Button */}
        <button
          onClick={onClose}
          className="absolute top-6 right-6 w-8 h-8 rounded-full bg-neutral-100 hover:bg-neutral-200 flex items-center justify-center text-[#12141A] transition-all cursor-pointer"
        >
          <X className="w-4 h-4" />
        </button>

        {/* Header */}
        <div className="flex items-center gap-3">
          <div className="w-12 h-12 rounded-2xl bg-[#F5C544]/20 flex items-center justify-center text-[#D4A322] shadow-xs">
            <Settings2 className="w-6 h-6" />
          </div>
          <div>
            <h2 className="text-xl font-extrabold text-[#12141A]">
              Target Machine Configuration
            </h2>
            <p className="text-xs text-[#6B7280]">
              Select or calibrate Mecha-Whisperer for any mechanical asset
            </p>
          </div>
        </div>

        {/* Top Toggle: Presets vs New Machine */}
        <div className="flex gap-2 p-1 rounded-full bg-neutral-100">
          <button
            onClick={() => setIsEditingCustom(false)}
            className={`flex-1 py-2 rounded-full text-xs font-bold transition-all cursor-pointer ${
              !isEditingCustom
                ? "bg-[#1C1F26] text-white shadow-xs"
                : "text-[#6B7280] hover:text-[#12141A]"
            }`}
          >
            Machine Profiles & Presets
          </button>
          <button
            onClick={() => setIsEditingCustom(true)}
            className={`flex-1 py-2 rounded-full text-xs font-bold transition-all cursor-pointer ${
              isEditingCustom
                ? "bg-[#1C1F26] text-white shadow-xs"
                : "text-[#6B7280] hover:text-[#12141A]"
            }`}
          >
            + Create Custom Device
          </button>
        </div>

        {/* Presets List View */}
        {!isEditingCustom && (
          <div className="grid grid-cols-1 sm:grid-cols-2 gap-3.5">
            {machines.map((m) => {
              const isSelected = m.id === currentMachine.id;
              return (
                <div
                  key={m.id}
                  onClick={() => {
                    onSelectMachine(m);
                    onClose();
                  }}
                  className={`p-4 rounded-2xl border transition-all cursor-pointer flex flex-col justify-between gap-3 relative ${
                    isSelected
                      ? "border-[#F5C544] bg-amber-50/50 shadow-sm ring-2 ring-[#F5C544]/40"
                      : "border-black/5 bg-[#F9FAFB] hover:border-neutral-300 hover:bg-white"
                  }`}
                >
                  <div className="flex items-start gap-3">
                    <img
                      src={m.image}
                      alt={m.name}
                      className="w-14 h-14 rounded-xl object-cover border border-black/5"
                    />
                    <div className="flex-1">
                      <div className="flex items-center justify-between">
                        <span className="text-[10px] font-bold text-[#D4A322] uppercase tracking-wider">
                          {m.category}
                        </span>
                        {isSelected && (
                          <span className="w-5 h-5 rounded-full bg-[#F5C544] text-[#12141A] flex items-center justify-center">
                            <Check className="w-3 h-3 stroke-[3]" />
                          </span>
                        )}
                      </div>
                      <h4 className="text-sm font-bold text-[#12141A] leading-snug mt-0.5">
                        {m.name}
                      </h4>
                      <p className="text-[11px] text-[#6B7280]">{m.motorType}</p>
                    </div>
                  </div>

                  <div className="grid grid-cols-3 gap-1 pt-2 border-t border-black/5 text-[10px] font-semibold text-[#6B7280]">
                    <div>
                      <span>RPM: </span>
                      <strong className="text-[#12141A]">{m.nominalRPM}</strong>
                    </div>
                    <div>
                      <span>1X Freq: </span>
                      <strong className="text-sky-600">{m.fundamentalHz}Hz</strong>
                    </div>
                    <div>
                      <span>ISO: </span>
                      <strong className="text-[#12141A]">{m.isoClass}</strong>
                    </div>
                  </div>
                </div>
              );
            })}
          </div>
        )}

        {/* Custom Machine Creation Form */}
        {isEditingCustom && (
          <form onSubmit={handleSaveCustom} className="flex flex-col gap-4">
            <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
              <div>
                <label className="block text-xs font-bold text-[#12141A] mb-1">
                  Machine / Asset Name
                </label>
                <input
                  type="text"
                  required
                  value={customName}
                  onChange={(e) => setCustomName(e.target.value)}
                  placeholder="e.g., CNC Lathe Spindle"
                  className="w-full px-3.5 py-2.5 rounded-xl bg-neutral-50 border border-neutral-200 text-xs font-semibold focus:outline-none focus:border-[#F5C544]"
                />
              </div>

              <div>
                <label className="block text-xs font-bold text-[#12141A] mb-1">
                  Equipment Category
                </label>
                <input
                  type="text"
                  required
                  value={customCategory}
                  onChange={(e) => setCustomCategory(e.target.value)}
                  placeholder="e.g., Machining / Pump / Fan"
                  className="w-full px-3.5 py-2.5 rounded-xl bg-neutral-50 border border-neutral-200 text-xs font-semibold focus:outline-none focus:border-[#F5C544]"
                />
              </div>

              <div>
                <label className="block text-xs font-bold text-[#12141A] mb-1">
                  Motor / Rotor Type
                </label>
                <input
                  type="text"
                  required
                  value={customMotorType}
                  onChange={(e) => setCustomMotorType(e.target.value)}
                  placeholder="e.g., AC Induction / Brushless"
                  className="w-full px-3.5 py-2.5 rounded-xl bg-neutral-50 border border-neutral-200 text-xs font-semibold focus:outline-none focus:border-[#F5C544]"
                />
              </div>

              <div>
                <label className="block text-xs font-bold text-[#12141A] mb-1">
                  Nominal Rotational Speed (RPM)
                </label>
                <input
                  type="number"
                  required
                  min={60}
                  max={60000}
                  value={customRPM}
                  onChange={(e) => setCustomRPM(Number(e.target.value))}
                  className="w-full px-3.5 py-2.5 rounded-xl bg-neutral-50 border border-neutral-200 text-xs font-semibold focus:outline-none focus:border-[#F5C544]"
                />
              </div>

              <div>
                <label className="block text-xs font-bold text-[#12141A] mb-1">
                  ISO 10816 Standard Classification
                </label>
                <select
                  value={customIsoClass}
                  onChange={(e: any) => setCustomIsoClass(e.target.value)}
                  className="w-full px-3.5 py-2.5 rounded-xl bg-neutral-50 border border-neutral-200 text-xs font-semibold focus:outline-none focus:border-[#F5C544]"
                >
                  <option value="Class I">Class I: Small Machines (&lt; 15 kW)</option>
                  <option value="Class II">Class II: Medium Machines (15 - 75 kW)</option>
                  <option value="Class III">Class III: Large Rigid Foundation</option>
                  <option value="Class IV">Class IV: Large Soft Foundation</option>
                </select>
              </div>

              <div>
                <label className="block text-xs font-bold text-[#12141A] mb-1">
                  Machine Photo URL (Optional)
                </label>
                <input
                  type="url"
                  value={customImage}
                  onChange={(e) => setCustomImage(e.target.value)}
                  placeholder="https://..."
                  className="w-full px-3.5 py-2.5 rounded-xl bg-neutral-50 border border-neutral-200 text-xs font-semibold focus:outline-none focus:border-[#F5C544]"
                />
              </div>
            </div>

            <button
              type="submit"
              className="mt-4 w-full py-3 rounded-xl bg-[#1C1F26] hover:bg-black text-white font-bold text-xs transition-all shadow-md cursor-pointer"
            >
              Save & Activate Device Profile
            </button>
          </form>
        )}
      </div>
    </div>
  );
};
