"use client";

import React, { useState } from "react";
import { useDeviceStream } from "../hooks/useDeviceStream";
import { DEFAULT_MACHINES, MachineProfile } from "../types/machine";
import { Header } from "../components/Header";
import { HeroSection } from "../components/HeroSection";
import { EquipmentCard } from "../components/EquipmentCard";
import { VibrationSeverityCard } from "../components/VibrationSeverityCard";
import { RadialExamCard } from "../components/RadialExamCard";
import { DemoChecklistCard } from "../components/DemoChecklistCard";
import { OscilloscopeTimelineCard } from "../components/OscilloscopeTimelineCard";
import { GraphGalleryStudio } from "../components/GraphGalleryStudio";
import { AIDiagnosticModal } from "../components/AIDiagnosticModal";
import { DeviceCustomizerModal } from "../components/DeviceCustomizerModal";
import { CustomDeviceProfilerModal } from "../components/CustomDeviceProfilerModal";
import { ConnectionModal } from "../components/ConnectionModal";

// Tab Views
import { VibrationFFTView } from "../components/views/VibrationFFTView";
import { StethoscopeView } from "../components/views/StethoscopeView";
import { ISOStandardView } from "../components/views/ISOStandardView";
import { HistoryView } from "../components/views/HistoryView";

export default function DashboardPage() {
  const {
    telemetry,
    isConnected,
    connectionMode,
    connectionError,
    isExamRunning,
    examSeconds,
    soundEnabled,
    setIsExamRunning,
    setSoundEnabled,
    connectWebSerial,
    connectBluetooth,
    connectWiFi,
    connectSimulator,
    disconnect,
    calibrate,
    setFaultMode
  } = useDeviceStream();

  const [activeTab, setActiveTab] = useState("dashboard");
  const [currentMachine, setCurrentMachine] = useState<MachineProfile>(DEFAULT_MACHINES[0]);
  const [isAIModalOpen, setIsAIModalOpen] = useState(false);
  const [isDeviceModalOpen, setIsDeviceModalOpen] = useState(false);
  const [isCustomProfilerOpen, setIsCustomProfilerOpen] = useState(false);
  const [isConnectionModalOpen, setIsConnectionModalOpen] = useState(false);

  return (
    <div className="flex justify-center w-full min-h-screen px-4 sm:px-8 py-6">
      <div className="w-full max-w-[1380px] flex flex-col gap-6">
        {/* Top Navigation */}
        <Header
          isConnected={isConnected}
          connectionMode={connectionMode}
          soundEnabled={soundEnabled}
          onToggleSound={() => setSoundEnabled(!soundEnabled)}
          onOpenConnectionModal={() => setIsConnectionModalOpen(true)}
          onOpenAIModal={() => setIsAIModalOpen(true)}
          onOpenDeviceModal={() => setIsDeviceModalOpen(true)}
          activeTab={activeTab}
          setActiveTab={setActiveTab}
          currentMachine={currentMachine}
        />

        {/* Hero Section */}
        <HeroSection
          telemetry={telemetry}
          machine={currentMachine}
          onOpenCustomizer={() => setIsDeviceModalOpen(true)}
          onOpenProfiler={() => setIsCustomProfilerOpen(true)}
        />

        {/* Wireless Quick-Connect Floating Banner if Disconnected from USB */}
        {!isConnected && (
          <div className="flex flex-col sm:flex-row items-center justify-between gap-3 px-5 py-3 rounded-2xl bg-amber-500/10 border border-amber-500/20 backdrop-blur-md animate-fade-in">
            <div className="flex items-center gap-3">
              <span className="w-2.5 h-2.5 rounded-full bg-amber-500 animate-ping" />
              <p className="text-xs font-semibold text-[#12141A]">
                <strong>Wireless Mode Ready:</strong> Connect laptop Wi-Fi to <span className="font-mono bg-white/80 px-2 py-0.5 rounded-md border border-amber-300 font-bold">MECHA-WHISPERER</span> or pair via Bluetooth.
              </p>
            </div>
            <div className="flex items-center gap-2">
              <button
                onClick={() => connectWiFi("192.168.4.1")}
                className="px-3.5 py-1.5 rounded-full bg-sky-600 hover:bg-sky-700 text-white text-xs font-bold shadow-xs transition-all cursor-pointer"
              >
                Connect Wi-Fi Stream
              </button>
              <button
                onClick={connectBluetooth}
                className="px-3.5 py-1.5 rounded-full bg-[#1C1F26] hover:bg-black text-white text-xs font-bold shadow-xs transition-all cursor-pointer"
              >
                Pair Bluetooth
              </button>
            </div>
          </div>
        )}

        {/* TAB 1: MAIN DASHBOARD OVERVIEW */}
        {activeTab === "dashboard" && (
          <div className="flex flex-col gap-6 animate-fade-in">
            {/* 4-Column Main Cards Grid */}
            <main className="grid grid-cols-1 md:grid-cols-2 xl:grid-cols-[280px_1.1fr_1.1fr_1.3fr] gap-5">
              {/* Card 1: Equipment Profile & Hardware Specs */}
              <EquipmentCard
                telemetry={telemetry}
                machine={currentMachine}
                onOpenCustomizer={() => setIsDeviceModalOpen(true)}
              />

              {/* Card 2: Micro-Vibration Severity RMS */}
              <VibrationSeverityCard
                telemetry={telemetry}
                onOpenReport={() => setActiveTab("vibration")}
              />

              {/* Card 3: Radial Diagnostic Health Score */}
              <RadialExamCard
                telemetry={telemetry}
                isExamRunning={isExamRunning}
                examSeconds={examSeconds}
                onTogglePlay={() => setIsExamRunning(!isExamRunning)}
                onCalibrate={calibrate}
                onToggleDemo={() => setFaultMode(telemetry.state === 1 ? "unbalance" : "healthy")}
              />

              {/* Card 4: Interactive Demo & Machine Fault Simulator */}
              <DemoChecklistCard
                telemetry={telemetry}
                onSelectMode={(mode) => setFaultMode(mode)}
              />
            </main>

            {/* Bottom Timeline & Real-Time Oscilloscope Card */}
            <OscilloscopeTimelineCard telemetry={telemetry} machine={currentMachine} />

            {/* Sensor Chart Studio (10 Real-Time Graph Styles) */}
            <GraphGalleryStudio telemetry={telemetry} machine={currentMachine} />
          </div>
        )}

        {/* TAB 2: DEDICATED VIBRATION FFT STUDIO */}
        {activeTab === "vibration" && (
          <VibrationFFTView telemetry={telemetry} machine={currentMachine} />
        )}

        {/* TAB 3: ACOUSTIC STETHOSCOPE STUDIO */}
        {activeTab === "stethoscope" && (
          <StethoscopeView
            telemetry={telemetry}
            machine={currentMachine}
            soundEnabled={soundEnabled}
            onToggleSound={() => setSoundEnabled(!soundEnabled)}
          />
        )}

        {/* TAB 4: ISO 10816 INDUSTRIAL EVALUATION MATRIX */}
        {activeTab === "iso" && (
          <ISOStandardView telemetry={telemetry} machine={currentMachine} />
        )}

        {/* TAB 5: INSPECTION HISTORY & EXPORT */}
        {activeTab === "history" && (
          <HistoryView telemetry={telemetry} machine={currentMachine} />
        )}

        {/* AI Doctor Modal */}
        <AIDiagnosticModal
          isOpen={isAIModalOpen}
          onClose={() => setIsAIModalOpen(false)}
          telemetry={telemetry}
          machine={currentMachine}
        />

        {/* Target Machine Customizer & Switcher Modal */}
        <DeviceCustomizerModal
          isOpen={isDeviceModalOpen}
          onClose={() => setIsDeviceModalOpen(false)}
          currentMachine={currentMachine}
          onSelectMachine={(m) => setCurrentMachine(m)}
          onOpenAIProfiler={() => setIsCustomProfilerOpen(true)}
        />

        {/* Custom Device & Event AI Profiler Modal */}
        <CustomDeviceProfilerModal
          isOpen={isCustomProfilerOpen}
          onClose={() => setIsCustomProfilerOpen(false)}
          telemetry={telemetry}
          onSaveProfile={(profile) => setCurrentMachine(profile)}
        />

        {/* Wireless Connection Selector Modal (BLE / Wi-Fi SoftAP / USB) */}
        <ConnectionModal
          isOpen={isConnectionModalOpen}
          onClose={() => setIsConnectionModalOpen(false)}
          connectionMode={connectionMode}
          isConnected={isConnected}
          connectionError={connectionError}
          onConnectWebSerial={connectWebSerial}
          onConnectBluetooth={connectBluetooth}
          onConnectWiFi={(ip) => connectWiFi(ip)}
          onConnectSimulator={connectSimulator}
          onDisconnect={disconnect}
        />
      </div>
    </div>
  );
}
