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
import { AIDiagnosticModal } from "../components/AIDiagnosticModal";
import { DeviceCustomizerModal } from "../components/DeviceCustomizerModal";

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
    isExamRunning,
    examSeconds,
    soundEnabled,
    setIsExamRunning,
    setSoundEnabled,
    connectWebSerial,
    calibrate,
    setFaultMode
  } = useDeviceStream();

  const [activeTab, setActiveTab] = useState("dashboard");
  const [currentMachine, setCurrentMachine] = useState<MachineProfile>(DEFAULT_MACHINES[0]);
  const [isAIModalOpen, setIsAIModalOpen] = useState(false);
  const [isDeviceModalOpen, setIsDeviceModalOpen] = useState(false);

  return (
    <div className="flex justify-center w-full min-h-screen px-4 sm:px-8 py-6">
      <div className="w-full max-w-[1380px] flex flex-col gap-6">
        {/* Top Navigation */}
        <Header
          isConnected={isConnected}
          connectionMode={connectionMode}
          soundEnabled={soundEnabled}
          onToggleSound={() => setSoundEnabled(!soundEnabled)}
          onConnect={connectWebSerial}
          onOpenAIModal={() => setIsAIModalOpen(true)}
          onOpenDeviceModal={() => setIsDeviceModalOpen(true)}
          activeTab={activeTab}
          setActiveTab={setActiveTab}
          currentMachine={currentMachine}
        />

        {/* Hero Section */}
        <HeroSection telemetry={telemetry} machine={currentMachine} />

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

              {/* Card 2: Vibration Severity & Session Bars */}
              <VibrationSeverityCard
                telemetry={telemetry}
                onOpenReport={() => setIsAIModalOpen(true)}
              />

              {/* Card 3: Radial Exam Gauge & Stopwatch */}
              <RadialExamCard
                telemetry={telemetry}
                examSeconds={examSeconds}
                isExamRunning={isExamRunning}
                onTogglePlay={() => setIsExamRunning(!isExamRunning)}
                onCalibrate={calibrate}
                onToggleDemo={() => {
                  if (telemetry.state === 1) {
                    setFaultMode("unbalance");
                  } else {
                    setFaultMode("healthy");
                  }
                }}
              />

              {/* Card 4: 1-Min Pitch Exam Checklist */}
              <DemoChecklistCard
                telemetry={telemetry}
                onSelectMode={(mode) => setFaultMode(mode)}
              />
            </main>

            {/* Bottom Timeline & Real-Time Oscilloscope Card */}
            <OscilloscopeTimelineCard telemetry={telemetry} />
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
        />

        {/* Target Machine Customizer & Switcher Modal */}
        <DeviceCustomizerModal
          isOpen={isDeviceModalOpen}
          onClose={() => setIsDeviceModalOpen(false)}
          currentMachine={currentMachine}
          onSelectMachine={(m) => setCurrentMachine(m)}
        />
      </div>
    </div>
  );
}
