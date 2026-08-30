"use client";

import { useEffect, useRef, useState, useCallback } from "react";

export interface TelemetryData {
  rpm: number;
  f0: number;
  rms: number;
  kurt: number;
  iso: number;
  score: number;
  state: number; // 1: Healthy, 3: Imbalance, 4: Bearing
  source?: "hardware" | "simulated";
}

export type ConnectionMode = "backend_ws" | "web_serial" | "bluetooth" | "wifi_ws" | "simulation";

export function useDeviceStream() {
  const [telemetry, setTelemetry] = useState<TelemetryData>({
    rpm: 2910,
    f0: 48.5,
    rms: 0.082,
    kurt: 2.94,
    iso: 0.16,
    score: 98,
    state: 1,
    source: "hardware"
  });

  const [isConnected, setIsConnected] = useState<boolean>(false);
  const [connectionMode, setConnectionMode] = useState<ConnectionMode>("simulation");
  const [isExamRunning, setIsExamRunning] = useState<boolean>(true);
  const [examSeconds, setExamSeconds] = useState<number>(45);
  const [soundEnabled, setSoundEnabled] = useState<boolean>(true);
  const [wifiTargetIP, setWifiTargetIP] = useState<string>("192.168.4.1");

  const wsRef = useRef<WebSocket | null>(null);
  const bleDeviceRef = useRef<any>(null);
  const bleCharRef = useRef<any>(null);
  const serialPortRef = useRef<any>(null);
  const readerRef = useRef<any>(null);
  const audioCtxRef = useRef<AudioContext | null>(null);
  const motorOscRef = useRef<OscillatorNode | null>(null);
  const motorGainRef = useRef<GainNode | null>(null);

  // Audio Stethoscope Engine
  const initAudio = useCallback(() => {
    if (typeof window === "undefined") return;
    if (!audioCtxRef.current) {
      const AudioCtx = window.AudioContext || (window as any).webkitAudioContext;
      const ctx = new AudioCtx();
      audioCtxRef.current = ctx;

      const osc = ctx.createOscillator();
      osc.type = "sawtooth";
      osc.frequency.setValueAtTime(telemetry.f0, ctx.currentTime);

      const filter = ctx.createBiquadFilter();
      filter.type = "lowpass";
      filter.frequency.setValueAtTime(180, ctx.currentTime);

      const gain = ctx.createGain();
      gain.gain.setValueAtTime(soundEnabled ? 0.03 : 0.0, ctx.currentTime);

      osc.connect(filter);
      filter.connect(gain);
      gain.connect(ctx.destination);
      osc.start();

      motorOscRef.current = osc;
      motorGainRef.current = gain;
    }
  }, [soundEnabled, telemetry.f0]);

  const updateAudioPitch = useCallback((freq: number, state: number) => {
    if (!audioCtxRef.current || !motorOscRef.current || !motorGainRef.current) return;
    if (!soundEnabled) {
      motorGainRef.current.gain.setTargetAtTime(0, audioCtxRef.current.currentTime, 0.05);
      return;
    }

    motorOscRef.current.frequency.setTargetAtTime(freq, audioCtxRef.current.currentTime, 0.05);
    const vol = state === 3 ? 0.12 : state === 4 ? 0.08 : 0.03;
    motorGainRef.current.gain.setTargetAtTime(vol, audioCtxRef.current.currentTime, 0.05);
  }, [soundEnabled]);

  // 1. Connect via Wi-Fi SoftAP (WebSocket + Fast HTTP Proxy Fallback)
  const connectWiFi = useCallback((ip: string = "192.168.4.1") => {
    initAudio();
    setWifiTargetIP(ip);
    setConnectionMode("wifi_ws");
    setIsConnected(true);

    if (wsRef.current) {
      wsRef.current.close();
    }

    try {
      const wsUrl = `ws://${ip}/ws`;
      const ws = new WebSocket(wsUrl);

      ws.onopen = () => {
        setIsConnected(true);
      };

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          setTelemetry((prev) => {
            const updated = { ...prev, ...data, source: "hardware" };
            updateAudioPitch(updated.f0, updated.state);
            return updated;
          });
        } catch (e) {}
      };

      ws.onclose = () => {
        // Will rely on HTTP polling below
      };

      wsRef.current = ws;
    } catch (err) {}
  }, [initAudio, updateAudioPitch]);

  // 2. Connect via Python WebSocket Backend (ws://localhost:8765/ws)
  const connectWebSocket = useCallback(() => {
    try {
      const ws = new WebSocket("ws://localhost:8765/ws");

      ws.onopen = () => {
        setIsConnected(true);
        setConnectionMode("backend_ws");
      };

      ws.onmessage = (event) => {
        try {
          const payload = JSON.parse(event.data);
          if (payload.type === "telemetry" && payload.data) {
            setTelemetry((prev) => {
              const updated = { ...prev, ...payload.data };
              updateAudioPitch(updated.f0, updated.state);
              return updated;
            });
          }
        } catch (e) {}
      };

      ws.onclose = () => {
        // If Python backend closes, auto-switch to Wi-Fi SoftAP
        connectWiFi(wifiTargetIP);
      };

      wsRef.current = ws;
    } catch (err) {
      connectWiFi(wifiTargetIP);
    }
  }, [connectWiFi, updateAudioPitch, wifiTargetIP]);

  // Continuous Fast Poll for Wi-Fi Telemetry (every 100ms via Server Proxy & direct fetch)
  useEffect(() => {
    if (connectionMode !== "wifi_ws") return;

    const pollInterval = setInterval(async () => {
      // 1. Try Next.js server-side proxy
      try {
        const proxyRes = await fetch(`/api/device/telemetry?ip=${wifiTargetIP}`, { cache: "no-store" });
        if (proxyRes.ok) {
          const data = await proxyRes.json();
          if (data && typeof data.score === "number") {
            setTelemetry((prev) => {
              const updated = { ...prev, ...data, source: "hardware" };
              updateAudioPitch(updated.f0, updated.state);
              return updated;
            });
            setIsConnected(true);
            return;
          }
        }
      } catch (e) {}

      // 2. Try direct browser fetch
      try {
        const directRes = await fetch(`http://${wifiTargetIP}/api/telemetry`, {
          cache: "no-store",
          signal: AbortSignal.timeout(800)
        });
        if (directRes.ok) {
          const data = await directRes.json();
          if (data && typeof data.score === "number") {
            setTelemetry((prev) => {
              const updated = { ...prev, ...data, source: "hardware" };
              updateAudioPitch(updated.f0, updated.state);
              return updated;
            });
            setIsConnected(true);
          }
        }
      } catch (e) {}
    }, 100);

    return () => clearInterval(pollInterval);
  }, [connectionMode, updateAudioPitch, wifiTargetIP]);

  // 3. Connect via Web Bluetooth API (BLE 5.0 Wireless)
  const connectBluetooth = useCallback(async () => {
    initAudio();

    if (!("bluetooth" in navigator)) {
      alert("Web Bluetooth API is not supported in this browser. Please use Google Chrome or Microsoft Edge.");
      return;
    }

    try {
      const device = await (navigator as any).bluetooth.requestDevice({
        acceptAllDevices: true,
        optionalServices: ["4fafc201-1fb5-459e-8fcc-c5c9c331914b", "generic_access"]
      });

      const server = await device.gatt.connect();
      bleDeviceRef.current = device;
      setIsConnected(true);
      setConnectionMode("bluetooth");

      try {
        const service = await server.getPrimaryService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
        const characteristic = await service.getCharacteristic("beb5483e-36e1-4688-b7f5-ea07361b26a8");
        bleCharRef.current = characteristic;

        await characteristic.startNotifications();
        characteristic.addEventListener("characteristicvaluechanged", (event: any) => {
          const decoder = new TextDecoder("utf-8");
          const jsonStr = decoder.decode(event.target.value);
          try {
            const data = JSON.parse(jsonStr);
            setTelemetry((prev) => {
              const updated = { ...prev, ...data, source: "hardware" };
              updateAudioPitch(updated.f0, updated.state);
              return updated;
            });
          } catch (e) {}
        });
      } catch (svcErr) {
        console.warn("BLE Characteristic subscription:", svcErr);
      }
    } catch (err) {
      console.warn("Bluetooth pairing error:", err);
    }
  }, [initAudio, updateAudioPitch]);

  // 4. Connect via Web Serial API
  const connectWebSerial = useCallback(async () => {
    initAudio();

    if (!("serial" in navigator)) {
      alert("Web Serial API is not supported in this browser.");
      return;
    }

    try {
      const port = await (navigator as any).serial.requestPort();
      await port.open({ baudRate: 115200 });

      serialPortRef.current = port;
      setIsConnected(true);
      setConnectionMode("web_serial");

      const textDecoder = new (window as any).TextDecoderStream();
      port.readable.pipeTo(textDecoder.writable);
      const reader = textDecoder.readable.getReader();
      readerRef.current = reader;

      let buffer = "";
      while (true) {
        const { value, done } = await reader.read();
        if (done) break;
        buffer += value;
        const lines = buffer.split("\n");
        buffer = lines.pop() || "";

        for (const line of lines) {
          const trimmed = line.trim();
          if (trimmed.startsWith("{") && trimmed.endsWith("}")) {
            try {
              const data = JSON.parse(trimmed);
              setTelemetry((prev) => {
                const updated = { ...prev, ...data, source: "hardware" };
                updateAudioPitch(updated.f0, updated.state);
                return updated;
              });
            } catch (e) {}
          }
        }
      }
    } catch (err) {
      console.warn("Serial connection closed:", err);
      setIsConnected(false);
    }
  }, [initAudio, updateAudioPitch]);

  // 5. Connect Simulator
  const connectSimulator = useCallback(() => {
    initAudio();
    setIsConnected(true);
    setConnectionMode("simulation");
  }, [initAudio]);

  // Disconnect
  const disconnect = useCallback(() => {
    if (wsRef.current) {
      wsRef.current.close();
      wsRef.current = null;
    }
    if (bleDeviceRef.current && bleDeviceRef.current.gatt.connected) {
      bleDeviceRef.current.gatt.disconnect();
      bleDeviceRef.current = null;
    }
    if (readerRef.current) {
      readerRef.current.cancel();
      readerRef.current = null;
    }
    if (serialPortRef.current) {
      serialPortRef.current.close();
      serialPortRef.current = null;
    }
    setIsConnected(false);
    setConnectionMode("simulation");
  }, []);

  // Controls
  const calibrate = useCallback(async () => {
    setTelemetry((prev) => ({ ...prev, score: 100, state: 1, iso: 0.16 }));
    try {
      await fetch("http://localhost:8765/api/control", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ command: "CALIBRATE" })
      });
    } catch (e) {}
  }, []);

  const setFaultMode = useCallback(async (mode: "healthy" | "unbalance" | "bearing") => {
    initAudio();
    const st = mode === "unbalance" ? 3 : mode === "bearing" ? 4 : 1;
    const score = mode === "unbalance" ? 18 : mode === "bearing" ? 32 : 98;
    const rms = mode === "unbalance" ? 1.48 : mode === "bearing" ? 0.48 : 0.082;
    const kurt = mode === "unbalance" ? 3.6 : mode === "bearing" ? 8.6 : 2.94;
    const iso = mode === "unbalance" ? 6.42 : mode === "bearing" ? 2.95 : 0.16;

    setTelemetry((prev) => ({
      ...prev,
      state: st,
      score,
      rms,
      kurt,
      iso
    }));

    updateAudioPitch(telemetry.f0, st);

    try {
      await fetch("http://localhost:8765/api/control", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ command: "SET_FAULT", param: mode })
      });
    } catch (e) {}
  }, [initAudio, telemetry.f0, updateAudioPitch]);

  // Try auto-connecting to Python WebSocket backend or Wi-Fi SoftAP on mount
  useEffect(() => {
    connectWebSocket();
    return () => {
      if (wsRef.current) wsRef.current.close();
    };
  }, [connectWebSocket]);

  // Exam Timer
  useEffect(() => {
    if (!isExamRunning) return;
    const interval = setInterval(() => {
      setExamSeconds((s) => s + 1);
    }, 1000);
    return () => clearInterval(interval);
  }, [isExamRunning]);

  return {
    telemetry,
    isConnected,
    connectionMode,
    isExamRunning,
    examSeconds,
    soundEnabled,
    wifiTargetIP,
    setIsExamRunning,
    setSoundEnabled,
    connectWebSerial,
    connectBluetooth,
    connectWiFi,
    connectSimulator,
    connectWebSocket,
    disconnect,
    calibrate,
    setFaultMode,
    initAudio
  };
}
