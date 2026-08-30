"use client";

import React from "react";
import { Volume2, VolumeX, Sparkles, Usb, Cpu } from "lucide-react";

interface HeaderProps {
  isConnected: boolean;
  connectionMode: "backend_ws" | "web_serial" | "simulation";
  soundEnabled: boolean;
  onToggleSound: () => void;
  onConnect: () => void;
  onOpenAIModal: () => void;
  activeTab: string;
  setActiveTab: (tab: string) => void;
}

export const Header: React.FC<HeaderProps> = ({
  isConnected,
  connectionMode,
  soundEnabled,
  onToggleSound,
  onConnect,
  onOpenAIModal,
  activeTab,
  setActiveTab
}) => {
  return (
    <header className="flex items-center justify-between py-2">
      {/* Brand Capsule */}
      <div className="flex items-center gap-2.5 px-4 py-2 rounded-full border border-black/5 bg-white/70 backdrop-blur-md shadow-xs">
        <div className="w-2.5 h-2.5 rounded-full bg-[#F5C544] shadow-[0_0_8px_rgba(245,197,68,0.7)] animate-pulse" />
        <span className="font-bold text-[15px] tracking-tight text-[#12141A]">MechaWhisperer</span>
      </div>

      {/* Navigation Pills */}
      <nav className="flex items-center bg-white/80 p-1 rounded-full border border-black/5 shadow-xs">
        {[
          { id: "dashboard", label: "Dashboard" },
          { id: "vibration", label: "Vibration FFT" },
          { id: "stethoscope", label: "Stethoscope" },
          { id: "iso", label: "ISO 10816" }
        ].map((tab) => (
          <button
            key={tab.id}
            onClick={() => setActiveTab(tab.id)}
            className={`px-4 py-1.5 rounded-full text-xs font-semibold transition-all duration-200 cursor-pointer ${
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
      <div className="flex items-center gap-3">
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

        {/* Connect USB / WebSocket Button */}
        <button
          onClick={onConnect}
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
          <span>
            {isConnected
              ? connectionMode === "backend_ws"
                ? "ESP32-S3 (Bridge Live)"
                : "ESP32-S3 (Web Serial)"
              : "Pair ESP32-S3"}
          </span>
          <Usb className="w-3.5 h-3.5 text-neutral-400" />
        </button>

        {/* User Profile Thumbnail */}
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
