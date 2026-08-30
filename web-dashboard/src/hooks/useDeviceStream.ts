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
  /** 24 normalised FFT bar magnitudes, matching BARS_COUNT in dsp_engine.h. */
  visualSpectrum: number[];
  source?: "hardware" | "simulated";
}

export type ConnectionMode = "backend_ws" | "web_serial" | "bluetooth" | "wifi_ws" | "simulation";

/** Must match ble_manager.c on the ESP32-S3. */
const BLE_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const BLE_TELEMETRY_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
const BLE_CONTROL_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a9";
const BLE_DEVICE_NAME = "MECHA-WHISPERER";

/** A WebSocket frame older than this means the link is stale and HTTP polling should take over. */
const WS_STALE_MS = 1500;

/** Matches BARS_COUNT in dsp_engine.h. */
const SPECTRUM_BARS = 24;
const DEFAULT_SPECTRUM = Array.from({ length: SPECTRUM_BARS }, (_, i) => 0.35 - i * 0.01);

export function useDeviceStream() {
  const [telemetry, setTelemetry] = useState<TelemetryData>({
    rpm: 2910,
    f0: 48.5,
    rms: 0.082,
    kurt: 2.94,
    iso: 0.16,
    score: 98,
    state: 1,
    visualSpectrum: DEFAULT_SPECTRUM,
    source: "hardware"
  });

  const [isConnected, setIsConnected] = useState<boolean>(false);
  const [connectionMode, setConnectionMode] = useState<ConnectionMode>("simulation");
  const [connectionError, setConnectionError] = useState<string | null>(null);
  const [isExamRunning, setIsExamRunning] = useState<boolean>(true);
  const [examSeconds, setExamSeconds] = useState<number>(45);
  const [soundEnabled, setSoundEnabled] = useState<boolean>(true);
  const [wifiTargetIP, setWifiTargetIP] = useState<string>("192.168.4.1");

  const wsRef = useRef<WebSocket | null>(null);
  const bleDeviceRef = useRef<any>(null);
  const bleTelemetryCharRef = useRef<any>(null);
  const bleControlCharRef = useRef<any>(null);
  const bleBufferRef = useRef<string>("");
  const serialPortRef = useRef<any>(null);
  const readerRef = useRef<any>(null);
  const readableClosedRef = useRef<Promise<void> | null>(null);
  const serialWriterRef = useRef<any>(null);
  const writableClosedRef = useRef<Promise<void> | null>(null);
  const audioCtxRef = useRef<AudioContext | null>(null);
  const motorOscRef = useRef<OscillatorNode | null>(null);
  const motorGainRef = useRef<GainNode | null>(null);

  // Guards against the auto-reconnect logic fighting an intentional disconnect().
  const manualDisconnectRef = useRef<boolean>(false);
  const reconnectTimerRef = useRef<ReturnType<typeof setTimeout> | null>(null);
  const lastFrameAtRef = useRef<number>(0);
  const connectionModeRef = useRef<ConnectionMode>("simulation");
  const wifiTargetIPRef = useRef<string>("192.168.4.1");

  useEffect(() => {
    connectionModeRef.current = connectionMode;
  }, [connectionMode]);

  useEffect(() => {
    wifiTargetIPRef.current = wifiTargetIP;
  }, [wifiTargetIP]);

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
    // Browsers start the context suspended until a user gesture.
    if (audioCtxRef.current.state === "suspended") {
      audioCtxRef.current.resume().catch(() => {});
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

  /** Single ingestion point for every transport, so link-health tracking can't be bypassed. */
  const ingestTelemetry = useCallback((raw: unknown) => {
    if (!raw || typeof raw !== "object") return;
    const data = raw as Partial<TelemetryData>;
    if (typeof data.score !== "number" && typeof data.rms !== "number") return;

    lastFrameAtRef.current = Date.now();
    // A device may omit or truncate the spectrum; never let that leave the
    // consumer indexing into an undefined array.
    const spectrum =
      Array.isArray(data.visualSpectrum) && data.visualSpectrum.length > 0
        ? data.visualSpectrum.map((v) => (typeof v === "number" && isFinite(v) ? v : 0))
        : null;

    setTelemetry((prev) => {
      const updated: TelemetryData = {
        ...prev,
        ...data,
        visualSpectrum: spectrum ?? prev.visualSpectrum,
        source: "hardware"
      };
      updateAudioPitch(updated.f0, updated.state);
      return updated;
    });
  }, [updateAudioPitch]);

  /** Parses newline-delimited JSON, returning the leftover partial line. */
  const drainLines = useCallback((buffer: string): string => {
    const lines = buffer.split("\n");
    const remainder = lines.pop() || "";
    for (const line of lines) {
      const trimmed = line.trim();
      if (!trimmed.startsWith("{") || !trimmed.endsWith("}")) continue;
      try {
        ingestTelemetry(JSON.parse(trimmed));
      } catch {
        /* Partial or non-telemetry line. */
      }
    }
    return remainder;
  }, [ingestTelemetry]);

  const clearReconnect = useCallback(() => {
    if (reconnectTimerRef.current) {
      clearTimeout(reconnectTimerRef.current);
      reconnectTimerRef.current = null;
    }
  }, []);

  // 1. Connect via Wi-Fi SoftAP (WebSocket primary, HTTP polling fallback)
  const connectWiFi = useCallback((ip: string = "192.168.4.1") => {
    initAudio();
    manualDisconnectRef.current = false;
    clearReconnect();
    setWifiTargetIP(ip);
    wifiTargetIPRef.current = ip;
    setConnectionMode("wifi_ws");
    connectionModeRef.current = "wifi_ws";
    setConnectionError(null);

    if (wsRef.current) {
      // Detach handlers first so this teardown doesn't trigger a reconnect.
      wsRef.current.onclose = null;
      wsRef.current.onerror = null;
      wsRef.current.close();
      wsRef.current = null;
    }

    try {
      const ws = new WebSocket(`ws://${ip}/ws`);

      ws.onopen = () => {
        // Only report connected once the socket is actually established.
        setIsConnected(true);
        setConnectionError(null);
      };

      ws.onmessage = (event) => {
        try {
          ingestTelemetry(JSON.parse(event.data));
        } catch {
          /* Ignore malformed frames. */
        }
      };

      ws.onerror = () => {
        // HTTP polling below keeps the dashboard alive while the socket retries.
        setConnectionError(`Wi-Fi socket error at ${ip} - falling back to HTTP polling`);
      };

      ws.onclose = () => {
        if (wsRef.current === ws) wsRef.current = null;
        if (manualDisconnectRef.current) return;
        if (connectionModeRef.current !== "wifi_ws") return;
        clearReconnect();
        reconnectTimerRef.current = setTimeout(() => connectWiFi(wifiTargetIPRef.current), 2000);
      };

      wsRef.current = ws;
    } catch {
      setConnectionError(`Unable to open ws://${ip}/ws`);
    }
  }, [clearReconnect, initAudio, ingestTelemetry]);

  // 2. Connect via Python WebSocket Backend (ws://localhost:8765/ws)
  const connectWebSocket = useCallback(() => {
    manualDisconnectRef.current = false;
    clearReconnect();

    if (wsRef.current) {
      wsRef.current.onclose = null;
      wsRef.current.onerror = null;
      wsRef.current.close();
      wsRef.current = null;
    }

    try {
      const ws = new WebSocket("ws://localhost:8765/ws");

      ws.onopen = () => {
        setIsConnected(true);
        setConnectionMode("backend_ws");
        connectionModeRef.current = "backend_ws";
        setConnectionError(null);
      };

      ws.onmessage = (event) => {
        try {
          const payload = JSON.parse(event.data);
          if (payload.type === "telemetry" && payload.data) {
            ingestTelemetry(payload.data);
          }
        } catch {
          /* Ignore malformed frames. */
        }
      };

      ws.onclose = () => {
        if (wsRef.current === ws) wsRef.current = null;
        // Never auto-fallback after an explicit disconnect, or the UI can't be turned off.
        if (manualDisconnectRef.current) return;
        // Only fall back if the backend bridge was the transport in play.
        const mode = connectionModeRef.current;
        if (mode !== "backend_ws" && mode !== "simulation") return;
        connectWiFi(wifiTargetIPRef.current);
      };

      wsRef.current = ws;
    } catch {
      connectWiFi(wifiTargetIPRef.current);
    }
  }, [clearReconnect, connectWiFi, ingestTelemetry]);

  // HTTP telemetry fallback. Only fires while the Wi-Fi WebSocket is stale, because the
  // ESP32 has a small socket pool and unconditional polling starves the WebSocket.
  useEffect(() => {
    if (connectionMode !== "wifi_ws") return;

    let cancelled = false;

    const poll = async () => {
      if (cancelled) return;
      if (Date.now() - lastFrameAtRef.current < WS_STALE_MS) return;

      // 1. Next.js server-side proxy (avoids mixed-content and CORS in most setups).
      try {
        const proxyRes = await fetch(`/api/device/telemetry?ip=${wifiTargetIP}`, { cache: "no-store" });
        if (proxyRes.ok) {
          const data = await proxyRes.json();
          if (data && typeof data.score === "number") {
            ingestTelemetry(data);
            if (!cancelled) setIsConnected(true);
            return;
          }
        }
      } catch {
        /* Fall through to the direct fetch. */
      }

      // 2. Direct browser fetch to the SoftAP.
      try {
        const directRes = await fetch(`http://${wifiTargetIP}/api/telemetry`, {
          cache: "no-store",
          signal: AbortSignal.timeout(800)
        });
        if (directRes.ok) {
          const data = await directRes.json();
          if (data && typeof data.score === "number") {
            ingestTelemetry(data);
            if (!cancelled) setIsConnected(true);
          }
        }
      } catch {
        /* Device unreachable this tick. */
      }
    };

    const pollInterval = setInterval(poll, 200);
    poll();

    return () => {
      cancelled = true;
      clearInterval(pollInterval);
    };
  }, [connectionMode, ingestTelemetry, wifiTargetIP]);

  // 3. Connect via Web Bluetooth API (BLE GATT peripheral on the ESP32-S3)
  const connectBluetooth = useCallback(async () => {
    initAudio();
    setConnectionError(null);

    if (typeof navigator === "undefined" || !("bluetooth" in navigator)) {
      setConnectionError("Web Bluetooth is not supported here. Use Chrome or Edge over HTTPS or localhost.");
      return;
    }

    if (typeof window !== "undefined" && !window.isSecureContext) {
      setConnectionError("Web Bluetooth requires a secure context (HTTPS or localhost).");
      return;
    }

    try {
      // Filter on our own service/name. "generic_access" must never appear in
      // optionalServices - it is on the Web Bluetooth blocklist and throws SecurityError.
      const device = await (navigator as any).bluetooth.requestDevice({
        filters: [
          { services: [BLE_SERVICE_UUID] },
          { namePrefix: BLE_DEVICE_NAME }
        ],
        optionalServices: [BLE_SERVICE_UUID]
      });

      manualDisconnectRef.current = false;
      bleDeviceRef.current = device;
      bleBufferRef.current = "";

      const handleDisconnect = () => {
        bleTelemetryCharRef.current = null;
        bleControlCharRef.current = null;
        if (manualDisconnectRef.current) return;
        setIsConnected(false);
        setConnectionError("Bluetooth device disconnected");
      };
      device.removeEventListener?.("gattserverdisconnected", handleDisconnect);
      device.addEventListener("gattserverdisconnected", handleDisconnect);

      const server = await device.gatt.connect();
      const service = await server.getPrimaryService(BLE_SERVICE_UUID);
      const characteristic = await service.getCharacteristic(BLE_TELEMETRY_CHAR_UUID);
      bleTelemetryCharRef.current = characteristic;

      characteristic.addEventListener("characteristicvaluechanged", (event: any) => {
        const chunk = new TextDecoder("utf-8").decode(event.target.value);
        // Firmware splits frames across notifications on small-MTU links and
        // delimits records with "\n", so reassemble before parsing.
        bleBufferRef.current = drainLines(bleBufferRef.current + chunk);
        if (bleBufferRef.current.length > 4096) bleBufferRef.current = "";
      });
      await characteristic.startNotifications();

      // Control characteristic is optional; telemetry still works without it.
      try {
        bleControlCharRef.current = await service.getCharacteristic(BLE_CONTROL_CHAR_UUID);
      } catch {
        bleControlCharRef.current = null;
      }

      // Only now is the link genuinely usable.
      setIsConnected(true);
      setConnectionMode("bluetooth");
      connectionModeRef.current = "bluetooth";
      setConnectionError(null);
    } catch (err: any) {
      setConnectionError(
        err?.name === "NotFoundError"
          ? "No MECHA-WHISPERER device selected. Make sure the board is powered and advertising."
          : `Bluetooth pairing failed: ${err?.message || err}`
      );
      setIsConnected(false);
    }
  }, [drainLines, initAudio]);

  // 4. Connect via Web Serial API
  const connectWebSerial = useCallback(async () => {
    initAudio();
    setConnectionError(null);

    if (typeof navigator === "undefined" || !("serial" in navigator)) {
      setConnectionError("Web Serial is not supported in this browser. Use Chrome or Edge.");
      return;
    }

    try {
      const port = await (navigator as any).serial.requestPort();
      await port.open({ baudRate: 115200 });

      manualDisconnectRef.current = false;
      serialPortRef.current = port;

      const textDecoder = new (window as any).TextDecoderStream();
      // Keep the pipe promise so disconnect() can await an orderly teardown;
      // without it port.close() throws because readable is still locked.
      readableClosedRef.current = port.readable.pipeTo(textDecoder.writable).catch(() => {});
      const reader = textDecoder.readable.getReader();
      readerRef.current = reader;

      try {
        const textEncoder = new (window as any).TextEncoderStream();
        writableClosedRef.current = textEncoder.readable.pipeTo(port.writable).catch(() => {});
        serialWriterRef.current = textEncoder.writable.getWriter();
      } catch {
        serialWriterRef.current = null;
      }

      setIsConnected(true);
      setConnectionMode("web_serial");
      connectionModeRef.current = "web_serial";

      // Run the read loop detached so callers can await connectWebSerial().
      void (async () => {
        let buffer = "";
        try {
          while (true) {
            const { value, done } = await reader.read();
            if (done) break;
            buffer = drainLines(buffer + value);
            if (buffer.length > 8192) buffer = "";
          }
        } catch {
          /* Reader cancelled or device unplugged. */
        } finally {
          if (readerRef.current === reader && !manualDisconnectRef.current) {
            setIsConnected(false);
            setConnectionError("Serial device disconnected");
          }
        }
      })();
    } catch (err: any) {
      setConnectionError(`Serial connection failed: ${err?.message || err}`);
      setIsConnected(false);
    }
  }, [drainLines, initAudio]);

  // 5. Connect Simulator
  const connectSimulator = useCallback(() => {
    initAudio();
    manualDisconnectRef.current = true;
    clearReconnect();
    if (wsRef.current) {
      wsRef.current.onclose = null;
      wsRef.current.onerror = null;
      wsRef.current.close();
      wsRef.current = null;
    }
    setIsConnected(true);
    setConnectionMode("simulation");
    connectionModeRef.current = "simulation";
    setConnectionError(null);
  }, [clearReconnect, initAudio]);

  // Disconnect every transport and stay disconnected.
  const disconnect = useCallback(async () => {
    manualDisconnectRef.current = true;
    clearReconnect();

    if (wsRef.current) {
      wsRef.current.onclose = null;
      wsRef.current.onerror = null;
      wsRef.current.close();
      wsRef.current = null;
    }

    if (bleDeviceRef.current) {
      try {
        if (bleTelemetryCharRef.current) {
          await bleTelemetryCharRef.current.stopNotifications().catch(() => {});
        }
        if (bleDeviceRef.current.gatt?.connected) bleDeviceRef.current.gatt.disconnect();
      } catch {
        /* Already gone. */
      }
      bleTelemetryCharRef.current = null;
      bleControlCharRef.current = null;
      bleDeviceRef.current = null;
      bleBufferRef.current = "";
    }

    // Order matters: cancel the reader, let the pipe settle, then close the port.
    if (readerRef.current) {
      try {
        await readerRef.current.cancel();
      } catch {
        /* Already cancelled. */
      }
      readerRef.current = null;
    }
    if (readableClosedRef.current) {
      await readableClosedRef.current;
      readableClosedRef.current = null;
    }
    if (serialWriterRef.current) {
      try {
        await serialWriterRef.current.close();
      } catch {
        /* Already closed. */
      }
      serialWriterRef.current = null;
    }
    if (writableClosedRef.current) {
      await writableClosedRef.current;
      writableClosedRef.current = null;
    }
    if (serialPortRef.current) {
      try {
        await serialPortRef.current.close();
      } catch {
        /* Already closed. */
      }
      serialPortRef.current = null;
    }

    setIsConnected(false);
    setConnectionMode("simulation");
    connectionModeRef.current = "simulation";
    setConnectionError(null);
  }, [clearReconnect]);

  /**
   * Routes a control command over whichever transport is live. The firmware
   * accepts both the bare token ("CALIB") and the JSON form on every transport.
   */
  const sendCommand = useCallback(async (command: string, param?: string) => {
    const body = JSON.stringify(param ? { command, param } : { command });
    const mode = connectionModeRef.current;

    if (mode === "bluetooth" && bleControlCharRef.current) {
      const bytes = new TextEncoder().encode(body);
      try {
        if (bleControlCharRef.current.writeValueWithoutResponse) {
          await bleControlCharRef.current.writeValueWithoutResponse(bytes);
        } else {
          await bleControlCharRef.current.writeValue(bytes);
        }
        return true;
      } catch {
        /* Fall through to other transports. */
      }
    }

    if (mode === "web_serial" && serialWriterRef.current) {
      try {
        await serialWriterRef.current.write(`${body}\n`);
        return true;
      } catch {
        /* Fall through. */
      }
    }

    if (mode === "wifi_ws") {
      if (wsRef.current?.readyState === WebSocket.OPEN) {
        try {
          wsRef.current.send(body);
          return true;
        } catch {
          /* Fall through to REST. */
        }
      }
      try {
        const res = await fetch(`http://${wifiTargetIPRef.current}/api/control`, {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body,
          signal: AbortSignal.timeout(1500)
        });
        if (res.ok) return true;
      } catch {
        /* Device unreachable. */
      }
    }

    // Python bridge, used by backend_ws and as a last resort.
    try {
      const res = await fetch("http://localhost:8765/api/control", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body,
        signal: AbortSignal.timeout(1500)
      });
      return res.ok;
    } catch {
      return false;
    }
  }, []);

  // Controls
  const calibrate = useCallback(async () => {
    // Optimistic local values are only correct when nothing real is streaming;
    // otherwise they would fight the device's own telemetry.
    if (connectionModeRef.current === "simulation") {
      setTelemetry((prev) => ({ ...prev, score: 100, state: 1, iso: 0.16, source: "simulated" }));
    }
    await sendCommand("CALIBRATE");
  }, [sendCommand]);

  const setFaultMode = useCallback(async (mode: "healthy" | "unbalance" | "bearing") => {
    initAudio();
    const st = mode === "unbalance" ? 3 : mode === "bearing" ? 4 : 1;

    if (connectionModeRef.current === "simulation") {
      const score = mode === "unbalance" ? 18 : mode === "bearing" ? 32 : 98;
      const rms = mode === "unbalance" ? 1.48 : mode === "bearing" ? 0.48 : 0.082;
      const kurt = mode === "unbalance" ? 3.6 : mode === "bearing" ? 8.6 : 2.94;
      const iso = mode === "unbalance" ? 6.42 : mode === "bearing" ? 2.95 : 0.16;
      setTelemetry((prev) => ({ ...prev, state: st, score, rms, kurt, iso, source: "simulated" }));
    }

    updateAudioPitch(telemetry.f0, st);
    await sendCommand("SET_FAULT", mode);
  }, [initAudio, sendCommand, telemetry.f0, updateAudioPitch]);

  // Try auto-connecting to the Python bridge on mount; it falls back to Wi-Fi SoftAP.
  useEffect(() => {
    connectWebSocket();
    return () => {
      manualDisconnectRef.current = true;
      if (reconnectTimerRef.current) clearTimeout(reconnectTimerRef.current);
      if (wsRef.current) {
        wsRef.current.onclose = null;
        wsRef.current.onerror = null;
        wsRef.current.close();
        wsRef.current = null;
      }
    };
    // Runs once on mount: re-running would tear down a live link.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

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
    connectionError,
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
    sendCommand,
    initAudio
  };
}
