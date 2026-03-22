/**
 * @file page.h
 * @brief This file is part of the WebServer example for the ESP8266WebServer.
 *  
 * This file contains long, multiline text variables for  all builtin resources.
 */

// used for $upload.htm
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<title>ECG en tiempo real</title>
<style>
  body {
    background: #0b0f14;
    color: #e6e6e6;
    font-family: monospace;
    text-align: center;
  }

  canvas {
    background: #000;
    border: 1px solid #444;
    margin-top: 10px;
  }

  #status {
    margin-top: 10px;
    color: #7dd3fc;
  }

  button {
    margin-top: 8px;
    padding: 8px 16px;
    background: #111827;
    color: #7dd3fc;
    border: 1px solid #334155;
    cursor: pointer;
    font-family: monospace;
  }

  button:hover {
    background: #1f2937;
  }
</style>
</head>
<body>

<h2>ECG WebSocket ESP8266</h2>

<div style="margin-top:10px;">
  <h3>BPM</h3>
  <div id="bpmValue" style="font-size:32px; color:#22c55e;">--</div>
</div>

<canvas id="ecg" width="1000" height="200"></canvas>
<canvas id="fft" width="1000" height="200"></canvas>

<div id="status">Desconectado</div> 
<button id="downloadBtn">Descargar CSV</button>

<script>
// ===== CONFIG =====
let WINDOW_MS = 5000; // ventana visible (ms)
const FS = 250;

// ===== BPM =====
let lastPeakTime = 0;
let bpm = 0;
let peakThreshold = 40;
let refractory = 300;
const bpmDisplay = document.getElementById("bpmValue");

// ===== CANVAS =====
const canvas = document.getElementById("ecg");
const ctx = canvas.getContext("2d");
const fftCanvas = document.getElementById("fft");
const fftCtx = fftCanvas.getContext("2d");

const WIDTH = canvas.width;
const HEIGHT = canvas.height;

// ===== DATA =====
let data = [];
let ts = [];
let csvContent = [];

// ===== FILTER =====
let media = 512.0;
let factorSuavizado = 0.05;
let fpb = 0;

// ===== WEBSOCKET =====
const status = document.getElementById("status");
const WS_URL = "ws://192.168.4.1:81";

let ws = new WebSocket(WS_URL);
ws.binaryType = "arraybuffer";

ws.onopen = () => status.textContent = "Conectado";
ws.onclose = () => status.textContent = "Desconectado";

ws.onmessage = (event) => {
  const buf = event.data;
  const view = new DataView(buf);

  for (let i = 0; i < buf.byteLength; i += 6) {
    const tist = view.getUint32(i, true);
    let val = view.getUint16(i + 4, true);

    if (Math.abs(val) <= fpb) val = 0;

    // filtro suavizado
    media = (val * factorSuavizado) + (media * (1 - factorSuavizado));
    let senal = val - media + 520;

    // BPM usando timestamp real
    if (
      senal > (media + peakThreshold) &&
      tist - lastPeakTime > refractory
    ) {
      if (lastPeakTime !== 0) {
        const rr = tist - lastPeakTime;
        bpm = Math.round(60000 / rr);
        bpmDisplay.textContent = bpm;
      }
      lastPeakTime = tist;
    }

    ts.push(tist);
    data.push(senal);

    // ventana deslizante en tiempo
    while (ts.length > 0 && (tist - ts[0]) > WINDOW_MS) {
      ts.shift();
      data.shift();
    }

    csvContent.push({
      timestamp: new Date().toISOString(),
      ts: tist,
      valor: val,
      fvalor: senal
    });
  }
};

// ===== INTERPOLACIÓN =====
function interpolateAt(t, ts, data) {
  let i = 0;

  while (i < ts.length - 1 && ts[i + 1] < t) i++;

  if (i >= ts.length - 1) return data[data.length - 1];

  const t0 = ts[i];
  const t1 = ts[i + 1];
  const v0 = data[i];
  const v1 = data[i + 1];

  const ratio = (t - t0) / (t1 - t0 || 1);
  return v0 + ratio * (v1 - v0);
}

// ===== DIBUJO ECG =====
function draw() {
  ctx.clearRect(0, 0, WIDTH, HEIGHT);

  if (ts.length < 2) return;

  const tNow = ts[ts.length - 1];
  const tStart = tNow - WINDOW_MS;

  const pixels = WIDTH;
  const dt = WINDOW_MS / pixels;

  ctx.strokeStyle = "#00ff88";
  ctx.lineWidth = 1.5;
  ctx.beginPath();

  let first = true;

  for (let x = 0; x < pixels; x++) {
    const t = tStart + x * dt;

    if (t < ts[0]) continue;

    const v = interpolateAt(t, ts, data);
    const y = mapValue(v, 0, 1023, HEIGHT, 0);

    if (first) {
      ctx.moveTo(x, y);
      first = false;
    } else {
      ctx.lineTo(x, y);
    }
  }

  ctx.stroke();

  drawFFT();
}

// ===== FFT =====
function fftReal(signal) {
  const N = signal.length;
  let re = new Array(N).fill(0);
  let im = new Array(N).fill(0);

  for (let k = 0; k < N; k++) {
    for (let n = 0; n < N; n++) {
      const phi = (2 * Math.PI * k * n) / N;
      re[k] += signal[n] * Math.cos(phi);
      im[k] -= signal[n] * Math.sin(phi);
    }
  }

  return re.map((r, i) => Math.sqrt(r * r + im[i] * im[i]));
}

function drawFFT() {
  const FFT_SIZE = 200;
  if (data.length < FFT_SIZE) return;

  const segment = data.slice(-FFT_SIZE);
  const mean = segment.reduce((a, b) => a + b, 0) / segment.length;
  const centered = segment.map(v => v - mean);
  const spectrum = fftReal(centered);

  const W = fftCanvas.width;
  const H = fftCanvas.height;
  const nyquist = FS / 2;

  fftCtx.clearRect(0, 0, W, H);

  fftCtx.strokeStyle = "#38bdf8";
  fftCtx.beginPath();

  spectrum.slice(0, FFT_SIZE / 2).forEach((v, i) => {
    const freq = (i * FS) / FFT_SIZE;
    const maxVal = Math.max(...spectrum) || 1;
    const x = (freq / nyquist) * W;
    const y = H - (v / maxVal) * H;

    if (i === 0) fftCtx.moveTo(x, y);
    else fftCtx.lineTo(x, y);
  });

  fftCtx.stroke();
}

// ===== UTIL =====
function mapValue(x, inMin, inMax, outMin, outMax) {
  return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

// ===== LOOP =====
function loop() {
  draw();
  requestAnimationFrame(loop);
}
loop();

// ===== CSV =====
document.getElementById("downloadBtn").onclick = () => {
  let rows = ["Timestamp,Ts,Valor,fValor"];
  csvContent.forEach(r => {
    rows.push(`${r.timestamp},${r.ts},${r.valor},${r.fvalor}`);
  });

  const blob = new Blob([rows.join("\n")], { type: 'text/csv' });
  const a = document.createElement('a');
  a.href = URL.createObjectURL(blob);
  a.download = "ecg.csv";
  a.click();
};
</script>

</body>
</html>)rawliteral";