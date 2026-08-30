"use client";

import React from "react";
import { Volume2, VolumeX, Sparkles, Usb, Wifi, Bluetooth, Settings2 } from "lucide-react";
import { MachineProfile } from "../types/machine";
import { ConnectionMode } from "../hooks/useDeviceStream";

interface HeaderProps {
  isConnected: boolean;
  connectionMode: ConnectionMode;
  soundEnabled: boolean;
  onToggleSound: () => void;
  onOpenConnectionModal: () => void;
  onOpenAIModal: () => void;
  onOpenDeviceModal: () => void;
  activeTab: string;
  setActiveTab: (tab: string) => void;
  currentMachine: MachineProfile;
}

export const Header: React.FC<HeaderProps> = ({
  isConnected,
  connectionMode,
  soundEnabled,
  onToggleSound,
  onOpenConnectionModal,
  onOpenAIModal,
  onOpenDeviceModal,
  activeTab,
  setActiveTab,
  currentMachine
}) => {
  const getConnectionLabel = () => {
    if (!isConnected) return "Pair Device";
    if (connectionMode === "bluetooth") return "BLE Wireless";
    if (connectionMode === "wifi_ws") return "Wi-Fi (192.168.4.1)";
    if (connectionMode === "backend_ws") return "ESP32 Bridge";
    if (connectionMode === "web_serial") return "USB Serial";
    return "Simulation Mode";
  };

  const renderConnectionIcon = () => {
    if (connectionMode === "bluetooth") return <Bluetooth className="w-3.5 h-3.5 text-amber-500" />;
    if (connectionMode === "wifi_ws") return <Wifi className="w-3.5 h-3.5 text-sky-500" />;
    return <Usb className="w-3.5 h-3.5 text-neutral-400" />;
  };

  return (
    <header className="flex flex-col md:flex-row items-center justify-between py-2 gap-4">
      {/* Brand Capsule & Device Selector */}
      <div className="flex items-center gap-2.5">
        <div className="flex items-center gap-2.5 px-4 py-2 rounded-full border border-black/5 bg-white/70 backdrop-blur-md shadow-xs">
          <div className="w-2.5 h-2.5 rounded-full bg-[#F5C544] shadow-[0_0_8px_rgba(245,197,68,0.7)] animate-pulse" />
          <span className="font-bold text-[15px] tracking-tight text-[#12141A]">MechaWhisperer</span>
        </div>

        {/* Machine Quick Switcher */}
        <button
          onClick={onOpenDeviceModal}
          className="flex items-center gap-1.5 px-3.5 py-1.5 rounded-full border border-black/5 bg-white/80 hover:bg-white text-[#12141A] text-xs font-bold transition-all cursor-pointer shadow-xs"
        >
          <Settings2 className="w-3.5 h-3.5 text-[#D4A322]" />
          <span>{currentMachine.name}</span>
          <span className="text-[10px] text-[#6B7280] font-normal">({currentMachine.nominalRPM} RPM)</span>
        </button>
      </div>

      {/* Navigation Pills */}
      <nav className="flex items-center bg-white/80 p-1 rounded-full border border-black/5 shadow-xs overflow-x-auto max-w-full">
        {[
          { id: "dashboard", label: "Dashboard" },
          { id: "vibration", label: "Vibration FFT" },
          { id: "stethoscope", label: "Stethoscope" },
          { id: "iso", label: "ISO 10816" },
          { id: "history", label: "History" }
        ].map((tab) => (
          <button
            key={tab.id}
            onClick={() => setActiveTab(tab.id)}
            className={`px-4 py-1.5 rounded-full text-xs font-semibold transition-all duration-200 cursor-pointer whitespace-nowrap ${
              activeTab === tab.id
                ? "bg-[#1C1F26] text-white shadow-sm"
                : "text-[#6B7280] hover:text-[#12141A]"
            }`}
          >
            {tab.label}
          </button>
        ))}
      </nav>

      {/* Actions */}
      <div className="flex items-center gap-2.5">
        {/* AI Doctor Copilot Button */}
        <button
          onClick={onOpenAIModal}
          className="flex items-center gap-2 px-3.5 py-1.5 rounded-full border border-amber-300 bg-amber-50 hover:bg-amber-100 text-amber-900 text-xs font-bold transition-all cursor-pointer shadow-xs"
        >
          <Sparkles className="w-3.5 h-3.5 text-[#F5C544]" />
          <span>AI Doctor</span>
        </button>

        {/* Audio Stethoscope Toggle */}
        <button
          onClick={onToggleSound}
          className="w-10 h-10 rounded-full border border-black/5 bg-white/80 flex items-center justify-center text-[#6B7280] hover:text-[#12141A] hover:bg-white transition-all cursor-pointer"
          title={soundEnabled ? "Mute Mechanical Audio" : "Unmute Mechanical Audio"}
        >
          {soundEnabled ? <Volume2 className="w-4 h-4 text-[#F5C544]" /> : <VolumeX className="w-4 h-4" />}
        </button>

        {/* Connect Wireless / USB Button */}
        <button
          onClick={onOpenConnectionModal}
          className={`flex items-center gap-2 px-4 py-2 rounded-full border text-xs font-semibold transition-all cursor-pointer shadow-xs ${
            isConnected
              ? "bg-emerald-50 border-emerald-300 text-emerald-700"
              : "bg-white border-black/5 text-[#12141A] hover:border-[#F5C544]"
          }`}
        >
          <span
            className={`w-2 h-2 rounded-full ${
              isConnected
                ? "bg-emerald-500 shadow-[0_0_8px_rgba(16,185,129,0.8)]"
                : "bg-neutral-300"
            }`}
          />
          <span>{getConnectionLabel()}</span>
          {renderConnectionIcon()}
        </button>

        {/* User Profile */}
        <div className="w-10 h-10 rounded-full overflow-hidden border-2 border-white shadow-xs">
          <img
            src="https://images.unsplash.com/photo-1534528741775-53994a69daeb?w=100&auto=format&fit=crop&q=80"
            alt="Inspector"
            className="w-full h-full object-cover"
          />
        </div>
      </div>
    </header>
  );
};
