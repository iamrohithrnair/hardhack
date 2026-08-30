#!/usr/bin/env python3
"""
MECHA-WHISPERER | High-Performance Python 3.14 Backend Bridge
Connects to ESP32-S3 via USB Serial (/dev/cu.usbmodem*)
and broadcasts real-time 50Hz-250Hz telemetry over WebSockets to Next.js dashboard.
"""

import asyncio
import glob
import json
import logging
import math
import os
import sys
import time
from contextlib import asynccontextmanager
from typing import List, Set

import serial
import uvicorn
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

logging.basicConfig(level=logging.INFO, format="%(asctime)s [%(levelname)s] %(message)s")
logger = logging.getLogger("mecha_backend")

# Global State
class DeviceState:
    def __init__(self):
        self.connected = False
        self.port_name = "/dev/cu.usbmodem101"
        self.baud_rate = 115200
        self.serial_inst = None
        self.demo_mode = False
        self.demo_state = 1 # 1: Healthy, 3: Imbalance, 4: Bearing
        self.demo_phase = 0.0
        self.last_telemetry = {
            "rpm": 2910,
            "f0": 48.5,
            "rms": 0.082,
            "kurt": 2.94,
            "iso": 0.16,
            "score": 98,
            "state": 1,
            "visualSpectrum": [0.05] * 24,
            "source": "hardware"
        }
        self.subscribers: Set[WebSocket] = set()

state = DeviceState()

def find_serial_ports() -> List[str]:
    """Find connected USB serial ports on macOS / Linux."""
    ports = glob.glob("/dev/cu.usbmodem*") + glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*") + glob.glob("/dev/cu.wch*")
    return ports

class ControlPayload(BaseModel):
    command: str
    param: str = ""

def serial_reader_thread():
    """Background loop that reads from physical USB serial port."""
    while True:
        try:
            ports = find_serial_ports()
            target_port = ports[0] if ports else state.port_name
            
            if not state.serial_inst or not state.serial_inst.is_open:
                try:
                    logger.info(f"Attempting connection to {target_port} @ {state.baud_rate}...")
                    state.serial_inst = serial.Serial(target_port, state.baud_rate, timeout=1)
                    state.port_name = target_port
                    state.connected = True
                    logger.info(f"Connected to ESP32-S3 on {target_port}")
                except Exception as e:
                    state.connected = False
                    time.sleep(2)
                    continue

            line = state.serial_inst.readline().decode("utf-8", errors="ignore").strip()
            if line.startswith("{") and line.endswith("}"):
                try:
                    data = json.loads(line)
                    spectrum = data.get("visualSpectrum", [0.05] * 24)
                    state.last_telemetry = {
                        "rpm": data.get("rpm", 2910),
                        "f0": data.get("f0", 48.5),
                        "rms": data.get("rms", 0.082),
                        "kurt": data.get("kurt", 2.94),
                        "iso": data.get("iso", 0.16),
                        "score": data.get("score", 98),
                        "state": data.get("state", 1),
                        "visualSpectrum": spectrum,
                        "source": "hardware"
                    }
                except Exception:
                    pass
        except Exception as e:
            logger.warning(f"Serial connection error: {e}")
            state.connected = False
            if state.serial_inst:
                try:
                    state.serial_inst.close()
                except Exception:
                    pass
                state.serial_inst = None
            time.sleep(1)

async def broadcast_telemetry(data: dict):
    """Broadcast JSON telemetry packet to all connected dashboard WebSocket clients."""
    if not state.subscribers:
        return
    msg = json.dumps({"type": "telemetry", "data": data})
    dead_sockets = []
    for ws in list(state.subscribers):
        try:
            await ws.send_text(msg)
        except Exception:
            dead_sockets.append(ws)
    for ws in dead_sockets:
        state.subscribers.discard(ws)

async def telemetry_broadcast_loop():
    """Asynchronous loop that broadcasts telemetry at 30 Hz to web clients."""
    while True:
        if state.demo_mode or not state.connected:
            state.demo_phase += 0.15
            if state.demo_state == 1:
                f0 = 48.5 + 0.1 * math.sin(state.demo_phase * 0.05)
                state.last_telemetry = {
                    "rpm": round(f0 * 60),
                    "f0": round(f0, 1),
                    "rms": round(0.082 + 0.003 * math.sin(state.demo_phase * 0.2), 3),
                    "kurt": round(2.94 + 0.05 * math.sin(state.demo_phase * 0.1), 2),
                    "iso": 0.16,
                    "score": 98,
                    "state": 1,
                    "visualSpectrum": [0.08 + 0.04 * math.sin(state.demo_phase * 0.1 + i * 0.3) for i in range(24)],
                    "source": "simulated"
                }
            elif state.demo_state == 3:
                f0 = 48.2 + 0.3 * math.sin(state.demo_phase * 0.1)
                spec = [0.08] * 24
                spec[4] = 0.95
                state.last_telemetry = {
                    "rpm": round(f0 * 60),
                    "f0": round(f0, 1),
                    "rms": round(1.48 + 0.06 * math.sin(state.demo_phase * 0.3), 3),
                    "kurt": round(3.6 + 0.15 * math.sin(state.demo_phase * 0.2), 2),
                    "iso": 6.42,
                    "score": 18,
                    "state": 3,
                    "visualSpectrum": spec,
                    "source": "simulated"
                }
            else:
                f0 = 48.5
                spec = [0.12 + 0.08 * (i % 3) for i in range(24)]
                spec[18] = 0.88
                state.last_telemetry = {
                    "rpm": 2910,
                    "f0": 48.5,
                    "rms": 0.48,
                    "kurt": 8.6,
                    "iso": 2.95,
                    "score": 32,
                    "state": 4,
                    "visualSpectrum": spec,
                    "source": "simulated"
                }
                
        await broadcast_telemetry(state.last_telemetry)
        await asyncio.sleep(0.033)

@asynccontextmanager
async def lifespan(app: FastAPI):
    import threading
    t = threading.Thread(target=serial_reader_thread, daemon=True)
    t.start()
    task = asyncio.create_task(telemetry_broadcast_loop())
    logger.info("Mecha-Whisperer Python 3.14 Backend started on http://localhost:8765")
    yield
    task.cancel()

app = FastAPI(title="Mecha-Whisperer Hardware Bridge", version="2.0.0", lifespan=lifespan)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.get("/api/status")
async def get_status():
    available_ports = find_serial_ports()
    return {
        "connected": state.connected,
        "current_port": state.port_name,
        "available_ports": available_ports,
        "demo_mode": state.demo_mode,
        "subscribers_count": len(state.subscribers)
    }

@app.get("/api/telemetry")
async def get_telemetry():
    return state.last_telemetry

@app.post("/api/control")
async def post_control(payload: ControlPayload):
    cmd = payload.command.upper()
    param = payload.param.lower()
    logger.info(f"Received control command: {cmd} ({param})")

    if cmd == "DEMO_TOGGLE":
        state.demo_mode = not state.demo_mode
        return {"status": "ok", "demo_mode": state.demo_mode}

    if cmd == "SET_FAULT":
        state.demo_mode = True
        if param == "unbalance":
            state.demo_state = 3
        elif param == "bearing":
            state.demo_state = 4
        else:
            state.demo_state = 1
        return {"status": "ok", "state": state.demo_state}

    if cmd == "CALIBRATE":
        if state.serial_inst and state.serial_inst.is_open:
            try:
                state.serial_inst.write(b"CALIBRATE\n")
            except Exception as e:
                logger.error(f"Failed to write CALIBRATE to serial: {e}")
        return {"status": "calibrating"}

    return {"status": "ignored"}

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    state.subscribers.add(websocket)
    logger.info(f"New client connected to telemetry stream ({len(state.subscribers)} total)")
    try:
        # Send initial snapshot immediately
        await websocket.send_text(json.dumps({"type": "telemetry", "data": state.last_telemetry}))
        while True:
            data = await websocket.receive_text()
            try:
                msg = json.loads(data)
                if msg.get("action") == "CALIBRATE":
                    if state.serial_inst and state.serial_inst.is_open:
                        state.serial_inst.write(b"CALIBRATE\n")
                elif msg.get("action") == "SET_FAULT":
                    state.demo_mode = True
                    fault = msg.get("mode", "healthy")
                    state.demo_state = 3 if fault == "unbalance" else 4 if fault == "bearing" else 1
            except Exception:
                pass
    except WebSocketDisconnect:
        state.subscribers.discard(websocket)
        logger.info(f"Client disconnected ({len(state.subscribers)} total)")
    except Exception as e:
        state.subscribers.discard(websocket)
        logger.warning(f"WebSocket error: {e}")

if __name__ == "__main__":
    uvicorn.run("server:app", host="0.0.0.0", port=8765, log_level="info")
