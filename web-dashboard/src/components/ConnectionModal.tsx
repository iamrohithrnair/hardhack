"use client";

import React, { useState } from "react";
import { X, Wifi, Bluetooth, Usb, Play, CheckCircle2, AlertCircle, Radio, ArrowRight, Smartphone } from "lucide-react";

interface ConnectionModalProps {
  isOpen: boolean;
  onClose: () => void;
  isConnected: boolean;
  connectionMode: "backend_ws" | "web_serial" | "bluetooth" | "wifi_ws" | "simulation";
  onConnectWebSerial: () => void;
  onConnectBluetooth: () => void;
  onConnectWiFi: (ip: string) => void;
  onConnectSimulator: () => void;
  onDisconnect: () => void;
}

export const ConnectionModal: React.FC<ConnectionModalProps> = ({
  isOpen,
  onClose,
  isConnected,
  connectionMode,
  onConnectWebSerial,
  onConnectBluetooth,
  onConnectWiFi,
  onConnectSimulator,
  onDisconnect
}) => {
  const [wifiIP, setWifiIP] = useState("192.168.4.1");
  const [activeTab, setActiveTab] = useState<"ble" | "wifi" | "usb" | "sim">("wifi");

  if (!isOpen) return null;

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/40 backdrop-blur-sm animate-fade-in">
      <div className="bg-white rounded-[32px] max-w-xl w-full p-8 border border-black/5 shadow-[0_24px_48px_rgba(0,0,0,0.15)] flex flex-col gap-6 relative max-h-[90vh] overflow-y-auto">
        {/* Close Button */}
        <button
          onClick={onClose}
          className="absolute top-6 right-6 w-8 h-8 rounded-full bg-neutral-100 hover:bg-neutral-200 flex items-center justify-center text-[#12141A] transition-all cursor-pointer"
        >
          <X className="w-4 h-4" />
        </button>

        {/* Header */}
        <div className="flex items-center gap-3">
          <div className="w-12 h-12 rounded-2xl bg-sky-100 flex items-center justify-center text-sky-600 shadow-xs">
            <Radio className="w-6 h-6" />
          </div>
          <div>
            <h2 className="text-xl font-extrabold text-[#12141A]">
              Pair & Connect Device
            </h2>
            <p className="text-xs text-[#6B7280]">
              Stream live vibration telemetry wirelessly via Bluetooth or Wi-Fi
            </p>
          </div>
        </div>

        {/* Current Active Status Pill */}
        {isConnected && (
          <div className="flex items-center justify-between p-3.5 rounded-2xl bg-emerald-50 border border-emerald-200 text-emerald-900 text-xs font-semibold">
            <div className="flex items-center gap-2">
              <span className="w-2.5 h-2.5 rounded-full bg-emerald-500 shadow-[0_0_8px_rgba(16,185,129,0.8)] animate-pulse" />
              <span>
                Connected via{" "}
                <strong className="uppercase">
                  {connectionMode.replace("_", " ")}
                </strong>
              </span>
            </div>
            <button
              onClick={() => {
                onDisconnect();
              }}
              className="px-3 py-1 rounded-full bg-rose-100 hover:bg-rose-200 text-rose-700 text-[11px] font-bold cursor-pointer transition-all"
            >
              Disconnect
            </button>
          </div>
        )}

        {/* Mode Selector Tabs */}
        <div className="grid grid-cols-4 gap-1.5 p-1 rounded-full bg-neutral-100">
          <button
            onClick={() => setActiveTab("wifi")}
            className={`py-2 rounded-full text-xs font-bold transition-all cursor-pointer flex items-center justify-center gap-1.5 ${
              activeTab === "wifi" ? "bg-[#1C1F26] text-white shadow-xs" : "text-[#6B7280]"
            }`}
          >
            <Wifi className="w-3.5 h-3.5" />
            <span>Wi-Fi</span>
          </button>

          <button
            onClick={() => setActiveTab("ble")}
            className={`py-2 rounded-full text-xs font-bold transition-all cursor-pointer flex items-center justify-center gap-1.5 ${
              activeTab === "ble" ? "bg-[#1C1F26] text-white shadow-xs" : "text-[#6B7280]"
            }`}
          >
            <Bluetooth className="w-3.5 h-3.5" />
            <span>BLE</span>
          </button>

          <button
            onClick={() => setActiveTab("usb")}
            className={`py-2 rounded-full text-xs font-bold transition-all cursor-pointer flex items-center justify-center gap-1.5 ${
              activeTab === "usb" ? "bg-[#1C1F26] text-white shadow-xs" : "text-[#6B7280]"
            }`}
          >
            <Usb className="w-3.5 h-3.5" />
            <span>USB</span>
          </button>

          <button
            onClick={() => setActiveTab("sim")}
            className={`py-2 rounded-full text-xs font-bold transition-all cursor-pointer flex items-center justify-center gap-1.5 ${
              activeTab === "sim" ? "bg-[#1C1F26] text-white shadow-xs" : "text-[#6B7280]"
            }`}
          >
            <Play className="w-3.5 h-3.5" />
            <span>Sim</span>
          </button>
        </div>

        {/* TAB 1: WIRELESS WI-FI SOFTAP */}
        {activeTab === "wifi" && (
          <div className="flex flex-col gap-4 animate-fade-in">
            <div className="p-4 rounded-2xl bg-[#F9FAFB] border border-black/5 flex flex-col gap-2.5 text-xs text-[#12141A]">
              <div className="flex items-center gap-2 font-bold text-sky-700">
                <Wifi className="w-4 h-4" />
                <span>Wi-Fi SoftAP Stream (No Router Required)</span>
              </div>
              <p className="text-[11px] text-[#6B7280] leading-relaxed">
                The ESP32-S3 broadcasts its own high-speed Wi-Fi hotspot. Connect your computer or phone to:
              </p>
              <div className="flex justify-between items-center bg-white p-2.5 rounded-xl border border-black/5 font-mono text-xs">
                <span>SSID: <strong>MECHA-WHISPERER</strong></span>
                <span className="text-[#10B981] font-bold">Open Network</span>
              </div>
            </div>

            <div>
              <label className="block text-xs font-bold text-[#12141A] mb-1">
                Device IP Address / Hostname
              </label>
              <input
                type="text"
                value={wifiIP}
                onChange={(e) => setWifiIP(e.target.value)}
                placeholder="192.168.4.1"
                className="w-full px-3.5 py-2.5 rounded-xl bg-neutral-50 border border-neutral-200 text-xs font-semibold focus:outline-none focus:border-[#F5C544]"
              />
            </div>

            <button
              onClick={() => {
                onConnectWiFi(wifiIP);
                onClose();
              }}
              className="w-full py-3 rounded-2xl bg-sky-600 hover:bg-sky-700 text-white font-bold text-xs flex items-center justify-center gap-2 shadow-md transition-all cursor-pointer"
            >
              <span>Connect via Wi-Fi (ws://{wifiIP}/ws)</span>
              <ArrowRight className="w-4 h-4" />
            </button>
          </div>
        )}

        {/* TAB 2: WIRELESS BLUETOOTH (BLE) */}
        {activeTab === "ble" && (
          <div className="flex flex-col gap-4 animate-fade-in">
            <div className="p-4 rounded-2xl bg-[#F9FAFB] border border-black/5 flex flex-col gap-2.5 text-xs text-[#12141A]">
              <div className="flex items-center gap-2 font-bold text-amber-700">
                <Bluetooth className="w-4 h-4" />
                <span>Web Bluetooth API (1-Click Wireless Pair)</span>
              </div>
              <p className="text-[11px] text-[#6B7280] leading-relaxed">
                Connects directly to the ESP32-S3 over Bluetooth Low Energy (BLE 5.0). Works natively in Google Chrome and Microsoft Edge on Mac, Windows, Linux, and Android.
              </p>
            </div>

            <button
              onClick={() => {
                onConnectBluetooth();
                onClose();
              }}
              className="w-full py-3.5 rounded-2xl bg-[#1C1F26] hover:bg-black text-white font-bold text-xs flex items-center justify-center gap-2 shadow-md transition-all cursor-pointer"
            >
              <Bluetooth className="w-4 h-4 text-[#F5C544]" />
              <span>Scan & Pair Bluetooth Stethoscope</span>
            </button>
          </div>
        )}

        {/* TAB 3: USB-C CABLE SERIAL */}
        {activeTab === "usb" && (
          <div className="flex flex-col gap-4 animate-fade-in">
            <div className="p-4 rounded-2xl bg-[#F9FAFB] border border-black/5 flex flex-col gap-2.5 text-xs text-[#12141A]">
              <div className="flex items-center gap-2 font-bold text-neutral-800">
                <Usb className="w-4 h-4" />
                <span>Wired USB-C Connection</span>
              </div>
              <p className="text-[11px] text-[#6B7280] leading-relaxed">
                Plug the board into your computer with a USB-C cable for ultra-low latency hardware serial streaming.
              </p>
            </div>

            <button
              onClick={() => {
                onConnectWebSerial();
                onClose();
              }}
              className="w-full py-3 rounded-2xl bg-[#1C1F26] hover:bg-black text-white font-bold text-xs flex items-center justify-center gap-2 shadow-md transition-all cursor-pointer"
            >
              <Usb className="w-4 h-4" />
              <span>Pair USB-C Port (/dev/cu.usbmodem)</span>
            </button>
          </div>
        )}

        {/* TAB 4: SIMULATOR */}
        {activeTab === "sim" && (
          <div className="flex flex-col gap-4 animate-fade-in">
            <div className="p-4 rounded-2xl bg-amber-50 border border-amber-200 text-xs text-amber-900">
              <p className="leading-relaxed font-semibold">
                Running in Interactive Simulation Mode generates realistic rotational harmonic waveforms and allows testing of fault anomalies without physical hardware.
              </p>
            </div>

            <button
              onClick={() => {
                onConnectSimulator();
                onClose();
              }}
              className="w-full py-3 rounded-2xl bg-[#F5C544] hover:bg-amber-400 text-[#12141A] font-bold text-xs flex items-center justify-center gap-2 shadow-md transition-all cursor-pointer"
            >
              <Play className="w-4 h-4" />
              <span>Activate Simulation Mode</span>
            </button>
          </div>
        )}
      </div>
    </div>
  );
};
