"use client";

import React, { useState } from "react";
import { useDeviceStream } from "../hooks/useDeviceStream";
import { Header } from "../components/Header";
import { HeroSection } from "../components/HeroSection";
import { EquipmentCard } from "../components/EquipmentCard";
import { VibrationSeverityCard } from "../components/VibrationSeverityCard";
import { RadialExamCard } from "../components/RadialExamCard";
import { DemoChecklistCard } from "../components/DemoChecklistCard";
import { OscilloscopeTimelineCard } from "../components/OscilloscopeTimelineCard";
import { AIDiagnosticModal } from "../components/AIDiagnosticModal";

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
  const [isAIModalOpen, setIsAIModalOpen] = useState(false);

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
          activeTab={activeTab}
          setActiveTab={setActiveTab}
        />

        {/* Hero Section */}
        <HeroSection telemetry={telemetry} />

        {/* 4-Column Main Cards Grid */}
        <main className="grid grid-cols-1 md:grid-cols-2 xl:grid-cols-[280px_1.1fr_1.1fr_1.3fr] gap-5">
          {/* Card 1: Equipment Profile & Hardware Specs */}
          <EquipmentCard telemetry={telemetry} />

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

        {/* AI Doctor Modal */}
        <AIDiagnosticModal
          isOpen={isAIModalOpen}
          onClose={() => setIsAIModalOpen(false)}
          telemetry={telemetry}
        />
      </div>
    </div>
  );
}
