/**
 * MECHA-WHISPERER | Real-Time Hardware Web Streaming Dashboard
 * Connects directly to ESP32-S3 via Web Serial API or Simulates Live Telemetry
 */

// State
let isConnected = false;
let serialPort = null;
let reader = null;
let simulationMode = false;
let isExamRunning = true;
let examSeconds = 45;

// Telemetry Metrics
let currentData = {
    rpm: 2910,
    f0: 48.5,
    rms: 0.082,
    kurt: 2.94,
    iso: 0.16,
    score: 98,
    state: 1 // 1: Healthy, 3: Critical Unbalance, 4: Bearing Damage
};

// Oscilloscope & Spectrum Buffers
const WAVE_LEN = 200;
let waveBuffer = new Array(WAVE_LEN).fill(0);
const FFT_BARS = 24;
let fftBuffer = new Array(FFT_BARS).fill(0.05);
let fftPeaks = new Array(FFT_BARS).fill(0.05);
let phase = 0;

// Web Audio API Synthesizer
let audioCtx = null;
let motorOsc = null;
let motorGain = null;
let soundEnabled = true;

// DOM Elements
const connBtn = document.getElementById('connect-serial-btn');
const connDot = document.getElementById('conn-dot');
const connLabel = document.getElementById('conn-label');

const statRpm = document.getElementById('stat-rpm');
const statFreq = document.getElementById('stat-freq');
const statRms = document.getElementById('stat-rms-val');
const topHealthVal = document.getElementById('top-health-val');
const topBalanceVal = document.getElementById('top-balance-val');
const topIsoVal = document.getElementById('top-iso-val');
const topKurtVal = document.getElementById('top-kurt-val');

const bigVibeNum = document.getElementById('big-vibe-num');
const activeLiveBar = document.getElementById('active-live-bar');
const barTooltip = document.getElementById('bar-tooltip');

const healthPercentBadge = document.getElementById('health-percent-badge');
const profileBadge = document.getElementById('profile-badge');
const profileStatusText = document.getElementById('profile-status-text');
const accSeverity = document.getElementById('acc-severity');

const examTimer = document.getElementById('exam-timer');
const gaugeArc = document.getElementById('gauge-arc');
const playPauseBtn = document.getElementById('play-pause-exam-btn');
const calibBtn = document.getElementById('calib-btn');
const simToggleBtn = document.getElementById('sim-toggle-btn');
const soundToggleBtn = document.getElementById('sound-toggle-btn');

const canvas = document.getElementById('live-stream-canvas');
const ctx = canvas.getContext('2d');

// Initialize Gauge Ticks
function initGaugeTicks() {
    const ticksGroup = document.getElementById('gauge-ticks');
    if (!ticksGroup) return;
    ticksGroup.innerHTML = '';
    
    const count = 36;
    for (let i = 0; i < count; i++) {
        const angle = (i / count) * 360;
        const rad = (angle * Math.PI) / 180;
        const r1 = 82;
        const r2 = (i % 3 === 0) ? 88 : 85;
        
        const x1 = 100 + r1 * Math.cos(rad);
        const y1 = 100 + r1 * Math.sin(rad);
        const x2 = 100 + r2 * Math.cos(rad);
        const y2 = 100 + r2 * Math.sin(rad);
        
        const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
        line.setAttribute('x1', x1);
        line.setAttribute('y1', y1);
        line.setAttribute('x2', x2);
        line.setAttribute('y2', y2);
        line.setAttribute('stroke', '#D1D5DB');
        line.setAttribute('stroke-width', (i % 3 === 0) ? '1.5' : '1');
        ticksGroup.appendChild(line);
    }
}

// Audio Synthesizer Setup
function initAudio() {
    if (!audioCtx) {
        audioCtx = new (window.AudioContext || window.webkitAudioContext)();
        
        motorOsc = audioCtx.createOscillator();
        motorOsc.type = 'sawtooth';
        motorOsc.frequency.setValueAtTime(currentData.f0, audioCtx.currentTime);
        
        const filter = audioCtx.createBiquadFilter();
        filter.type = 'lowpass';
        filter.frequency.setValueAtTime(180, audioCtx.currentTime);
        
        motorGain = audioCtx.createGain();
        motorGain.gain.setValueAtTime(soundEnabled ? 0.04 : 0.0, audioCtx.currentTime);
        
        motorOsc.connect(filter);
        filter.connect(motorGain);
        motorGain.connect(audioCtx.destination);
        motorOsc.start();
    }
}

function updateAudio() {
    if (!audioCtx || !motorOsc || !motorGain) return;
    if (!soundEnabled) {
        motorGain.gain.setTargetAtTime(0, audioCtx.currentTime, 0.05);
        return;
    }
    
    motorOsc.frequency.setTargetAtTime(currentData.f0, audioCtx.currentTime, 0.05);
    let vol = (currentData.state === 3) ? 0.12 : (currentData.state === 4 ? 0.08 : 0.03);
    motorGain.gain.setTargetAtTime(vol, audioCtx.currentTime, 0.05);
}

// Web Serial Connection
async function toggleSerialConnection() {
    initAudio();
    
    if (isConnected) {
        disconnectSerial();
        return;
    }
    
    if (!('serial' in navigator)) {
        alert('Web Serial API is not supported in this browser. Please use Google Chrome, Edge, or enable Simulation Mode.');
        startSimulation();
        return;
    }
    
    try {
        serialPort = await navigator.serial.requestPort();
        await serialPort.open({ baudRate: 115200 });
        
        isConnected = true;
        simulationMode = false;
        connBtn.classList.add('connected');
        connLabel.textContent = 'ESP32-S3 Paired';
        
        readSerialStream();
    } catch (err) {
        console.warn('Serial pairing error or cancelled:', err);
        startSimulation();
    }
}

async function readSerialStream() {
    const textDecoder = new TextDecoderStream();
    const readableStreamClosed = serialPort.readable.pipeTo(textDecoder.writable);
    reader = textDecoder.readable.getReader();
    
    let buffer = '';
    try {
        while (true) {
            const { value, done } = await reader.read();
            if (done) break;
            
            buffer += value;
            const lines = buffer.split('\n');
            buffer = lines.pop(); // Keep incomplete line
            
            for (const line of lines) {
                parseTelemetryLine(line.trim());
            }
        }
    } catch (err) {
        console.error('Serial stream reading error:', err);
    } finally {
        reader.releaseLock();
    }
}

function disconnectSerial() {
    if (reader) {
        reader.cancel();
    }
    if (serialPort) {
        serialPort.close();
    }
    isConnected = false;
    connBtn.classList.remove('connected');
    connLabel.textContent = 'Pair ESP32-S3';
}

function parseTelemetryLine(line) {
    if (!line.startsWith('{') || !line.endsWith('}')) return;
    try {
        const data = JSON.parse(line);
        currentData.rpm = data.rpm || currentData.rpm;
        currentData.f0 = data.f0 || currentData.f0;
        currentData.rms = data.rms !== undefined ? data.rms : currentData.rms;
        currentData.kurt = data.kurt !== undefined ? data.kurt : currentData.kurt;
        currentData.iso = data.iso !== undefined ? data.iso : currentData.iso;
        currentData.score = data.score !== undefined ? data.score : currentData.score;
        currentData.state = data.state !== undefined ? data.state : currentData.state;
        
        updateUI();
    } catch (e) {
        // Ignore non-json lines
    }
}

// Fallback Simulation Mode
function startSimulation() {
    simulationMode = true;
    connBtn.classList.add('connected');
    connLabel.textContent = 'Simulation Live';
}

function updateSimulationPhysics() {
    if (!simulationMode) return;
    
    phase += 0.15;
    
    if (currentData.state === 1) {
        // Nominal state
        currentData.f0 = 48.5 + Math.sin(phase * 0.05) * 0.1;
        currentData.rpm = Math.round(currentData.f0 * 60);
        currentData.rms = 0.082 + (Math.random() - 0.5) * 0.004;
        currentData.kurt = 2.94 + (Math.random() - 0.5) * 0.05;
        currentData.iso = 0.16;
        currentData.score = 98;
    } else if (currentData.state === 3) {
        // Critical Imbalance
        currentData.f0 = 48.2 + Math.sin(phase * 0.1) * 0.4;
        currentData.rpm = Math.round(currentData.f0 * 60);
        currentData.rms = 1.48 + (Math.random() - 0.5) * 0.08;
        currentData.kurt = 3.6 + (Math.random() - 0.5) * 0.2;
        currentData.iso = 6.42;
        currentData.score = 18;
    }
    
    updateUI();
}

// UI Updates
function updateUI() {
    statRpm.textContent = currentData.rpm.toLocaleString();
    statFreq.textContent = currentData.f0.toFixed(1);
    statRms.textContent = currentData.rms.toFixed(3);
    
    topHealthVal.textContent = `${currentData.score}%`;
    topBalanceVal.textContent = (currentData.state === 3) ? '18%' : '96%';
    topIsoVal.textContent = (currentData.iso < 1.12) ? 'Class A' : (currentData.iso < 2.8 ? 'Class B' : 'Class D');
    topKurtVal.textContent = currentData.kurt.toFixed(1);
    
    bigVibeNum.textContent = currentData.rms.toFixed(2);
    let barH = Math.min(100, Math.max(10, currentData.rms * 60));
    activeLiveBar.style.height = `${barH}%`;
    barTooltip.textContent = `${currentData.rms.toFixed(2)}g`;
    
    healthPercentBadge.textContent = `${currentData.score}%`;
    accSeverity.textContent = `${currentData.iso.toFixed(2)} mm/s`;
    
    const isFault = currentData.score < 50;
    
    if (isFault) {
        profileStatusText.textContent = 'Critical Imbalance';
        profileBadge.style.background = 'rgba(244, 63, 94, 0.85)';
        document.getElementById('event-pill-2').style.display = 'flex';
        document.getElementById('task-3').classList.add('completed');
        document.getElementById('task-3-sub').textContent = 'Mass Unbalance (+18dB 1X Spike)';
        document.getElementById('task-counter').textContent = '5/5';
    } else {
        profileStatusText.textContent = 'Healthy Operation';
        profileBadge.style.background = 'rgba(28, 31, 38, 0.85)';
        document.getElementById('event-pill-2').style.display = 'none';
        document.getElementById('task-3').classList.remove('completed');
        document.getElementById('task-3-sub').textContent = 'Attach tape to blade for anomaly';
        document.getElementById('task-counter').textContent = '4/5';
    }
    
    // Update Radial Arc
    const circumference = 452;
    const offset = circumference - (currentData.score / 100) * circumference;
    gaugeArc.style.strokeDashoffset = offset;
    gaugeArc.style.stroke = isFault ? '#F43F5E' : '#F5C544';
    
    updateAudio();
}

// Canvas Waveform & FFT Spectrum Rendering
function renderLiveStream() {
    const w = canvas.width;
    const h = canvas.height;
    
    ctx.fillStyle = '#0D1017';
    ctx.fillRect(0, 0, w, h);
    
    // Center baseline
    const midY = h / 2;
    ctx.strokeStyle = '#1E2330';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(0, midY);
    ctx.lineTo(w, midY);
    ctx.stroke();
    
    // Generate next wave sample
    phase += 0.12;
    let noise = (Math.random() - 0.5) * 0.05;
    let sample = 0;
    
    if (currentData.state === 1) {
        sample = 0.28 * Math.sin(phase) + noise;
    } else {
        let mod = 1.0 + 0.3 * Math.sin(phase * 0.2);
        sample = 0.85 * Math.sin(phase) * mod + 0.3 * Math.sin(phase * 2) + noise * 4;
    }
    
    waveBuffer.shift();
    waveBuffer.push(sample);
    
    // Draw 24-Band FFT Bars in Background
    const barWidth = (w - (FFT_BARS - 1) * 6) / FFT_BARS;
    for (let i = 0; i < FFT_BARS; i++) {
        let target = 0.04;
        if (currentData.state === 1) {
            if (i === 4) target = 0.75;
            if (i === 8) target = 0.15;
        } else {
            if (i === 4) target = 0.95;
            if (i === 8) target = 0.65;
            if (i === 12) target = 0.45;
        }
        
        fftBuffer[i] = fftBuffer[i] * 0.8 + target * 0.2;
        if (fftBuffer[i] > fftPeaks[i]) fftPeaks[i] = fftBuffer[i];
        else fftPeaks[i] = Math.max(0, fftPeaks[i] - 0.01);
        
        const bx = i * (barWidth + 6);
        const bh = fftBuffer[i] * (h * 0.7);
        const by = h - bh;
        
        ctx.fillStyle = (currentData.score < 50) ? 'rgba(244, 63, 94, 0.25)' : 'rgba(245, 197, 68, 0.2)';
        ctx.fillRect(bx, by, barWidth, bh);
        
        // Peak line
        const py = h - fftPeaks[i] * (h * 0.7);
        ctx.fillStyle = (currentData.score < 50) ? '#F43F5E' : '#F5C544';
        ctx.fillRect(bx, py, barWidth, 2);
    }
    
    // Draw Primary Micro-Vibration Trace
    ctx.strokeStyle = (currentData.score < 50) ? '#F43F5E' : '#0EA5E9';
    ctx.lineWidth = 3;
    ctx.shadowColor = (currentData.score < 50) ? '#F43F5E' : '#0EA5E9';
    ctx.shadowBlur = 10;
    ctx.beginPath();
    
    for (let i = 0; i < waveBuffer.length; i++) {
        const x = (i / waveBuffer.length) * w;
        const y = midY - waveBuffer[i] * (h * 0.4);
        if (i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
    }
    ctx.stroke();
    ctx.shadowBlur = 0; // reset
}

// Timer Loop
setInterval(() => {
    if (isExamRunning) {
        examSeconds++;
        const mins = String(Math.floor(examSeconds / 60)).padStart(2, '0');
        const secs = String(examSeconds % 60).padStart(2, '0');
        examTimer.textContent = `${mins}:${secs}`;
    }
}, 1000);

// Animation Loop
function animate() {
    updateSimulationPhysics();
    renderLiveStream();
    requestAnimationFrame(animate);
}

// Event Listeners
connBtn.addEventListener('click', toggleSerialConnection);

soundToggleBtn.addEventListener('click', () => {
    soundEnabled = !soundEnabled;
    soundToggleBtn.style.color = soundEnabled ? '#12141A' : '#9CA3AF';
    updateAudio();
});

playPauseBtn.addEventListener('click', () => {
    isExamRunning = !isExamRunning;
    playPauseBtn.style.background = isExamRunning ? '#1C1F26' : '#F5C544';
});

calibBtn.addEventListener('click', () => {
    currentData.state = 1;
    currentData.score = 100;
    updateUI();
});

simToggleBtn.addEventListener('click', () => {
    initAudio();
    if (currentData.state === 1) {
        currentData.state = 3; // Switch to unbalance
    } else {
        currentData.state = 1; // Switch to nominal
    }
    updateUI();
});

document.getElementById('tab-nominal').addEventListener('click', (e) => {
    document.querySelectorAll('.seg-tab').forEach(t => t.classList.remove('active'));
    e.target.classList.add('active');
    currentData.state = 1;
    updateUI();
});

document.getElementById('tab-unbalance').addEventListener('click', (e) => {
    document.querySelectorAll('.seg-tab').forEach(t => t.classList.remove('active'));
    e.target.classList.add('active');
    currentData.state = 3;
    updateUI();
});

document.getElementById('tab-bearing').addEventListener('click', (e) => {
    document.querySelectorAll('.seg-tab').forEach(t => t.classList.remove('active'));
    e.target.classList.add('active');
    currentData.state = 4;
    updateUI();
});

// Start
initGaugeTicks();
startSimulation();
animate();
