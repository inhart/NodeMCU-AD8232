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

<div style="margin: 20px auto; width: 80%; max-width: 500px;">
<label for="gainSlider">Horizontal resolution: <span id="valDisplay">512</span></label><br>
<input type="range" id="gainSlider" min="32" max="16384" step="1" value="512" list="markers"
style="width: 100%; cursor: pointer;">

  <!-- Marcas de graduación -->
  <datalist id="markers">
    <option value="50" label="Min"></option>
    <option value="100"></option>
    <option value="200"></option>
    <option value="350"></option>
    <option value="500"></option>
    <option value="1000"></option>
    <option value="2000"></option>
    <option value="4000"></option>
    <option value="8000"></option>
    <option value="16000" label="Max"></option>
  </datalist>

</div>



<div style="margin: 20px auto; width: 80%; max-width: 500px;">
<label for="smoothSlider">Suavizado: <span id="smoothvalDisplay">0.1</span></label><br>
<input type="range" id="smoothSlider" min="0.01" max="1.0" step="0.01" value="0.01" list="markers"
style="width: 100%; cursor: pointer;">

  <!-- Marcas de graduación -->
  <datalist id="markers">
    <option value="0.01" label="Min"></option>
    <option value="0.05"></option>
    <option value="0.1"></option>
    <option value="0.2"></option>
    <option value="0.3"></option>
    <option value="0.4"></option>
    <option value="0.5"></option>
    <option value="0.6"></option>
    <option value="0.7"></option>
    <option value="0.8"></option>
    <option value="0.9"></option>
    <option value="1" label="Max"></option>
  </datalist>
</div>

<div class="vertical-container">
  <label for="ampSlider"> Amplitud: <br><span id="ampDisplay">0</span></label>
  <input type="range" id="ampSlider" class="v-slider" min="0" max="600" step="1" value="0" list="markers3"
  style="width: 80%; max-width: 500px; cursor: pointer;">

    <!-- Marcas de graduación -->
  <datalist id="markers3">
    <option value="0" label="Min"></option>
    <option value="50"></option>
    <option value="100"></option>
    <option value="200"></option>
    <option value="300"></option>
    <option value="400"></option>
    <option value="600" label="Max"></option>
  </datalist>
</div>

<div style="margin-top:10px; display:flex; justify-content:space-around;">

      <h3>BPM</h3>
      <div id="bpmValue" style="font-size:32px; color:#22c55e;">--</div>
    
</div>

<div class="canvas-wrapper">
  <canvas id="ecg" width="1000" height="200"></canvas>
</div>

<div>
  <!-- <h3>FFT</h3> -->
  <canvas id="fft" width="1000" height="200"></canvas>
</div>

<div id="status">Desconectado</div> 
  <button id="downloadBtn">Descargar CSV</button>
</div>

<script>
// ===== BPM =====
let lastPeakTime = 0;
let bpm = 0;
let peakThreshold = 40; // ajustable según señal
let refractory = 300; // ms
const bpmDisplay = document.getElementById("bpmValue");

// ===== FFT =====
const fftCanvas = document.getElementById("fft");
const fftCtx = fftCanvas.getContext("2d");
const FFT_SIZE = 200;
const canvas = document.getElementById("ecg");
const ctx = canvas.getContext("2d");
const status = document.getElementById("status");
const downloadBtn = document.getElementById("downloadBtn");

const FS = 250; // Hz (AJÚSTALO a tu ESP8266)

const WIDTH = canvas.width;
const HEIGHT = canvas.height;


var suma = 0;
var media = 512.0; // Usamos float para mayor precisión
var factorSuavizado = 0.0; // Valor entre 0.01 y 1 (más bajo = más estable)
var senalCentrada = 0.0;
var fpb = 0;
var MAX_SAMPLES = 18000;
let data = [];
let csvContent = [];
let lastVal = 0;

const WS_URL = "ws://192.168.4.1:81";
var ws = new WebSocket(WS_URL);
ws.binaryType = "arraybuffer";

ws.onopen = () => { status.textContent = "Conectado al ESP8266"; };
ws.onclose = () => { status.textContent = "Desconectado"; };

ws.onmessage = (event) => {
  const buf = event.data;
  const view = new DataView(buf);

  for (let i = 0; i < buf.byteLength; i += 2) {
    const val = view.getUint16(i, true);
    
    if (Math.abs(val) <= fpb){
        val = 0;
};
 
    media = (val * factorSuavizado) + (media * (1.0 - factorSuavizado));

    senalCentrada = val - media + 520;
   const now = performance.now();

// detección simple de pico R
if (
  senalCentrada > (media + peakThreshold) &&
  now - lastPeakTime > refractory
) {
  if (lastPeakTime !== 0) {
    const rr = now - lastPeakTime;
    bpm = Math.round(60000 / rr);
    bpmDisplay.textContent = bpm;
  }
  lastPeakTime = now;
}
    data.push(senalCentrada); // Desplazar para que esté en rango positivo
    if (data.length > MAX_SAMPLES) data.shift();

    csvContent.push({
      timestamp: new Date().toISOString(),
      valor: val,
      fvalor: senalCentrada

    });
  }
  draw();

};

  const ampSlider = document.getElementById("ampSlider");
  const ampDisplay = document.getElementById("ampDisplay");

  ampSlider.oninput = function () {
    fpb = parseFloat(this.value);
    ampDisplay.textContent = fpb.toFixed(2);
  };

const slider = document.getElementById("gainSlider");
const display = document.getElementById("valDisplay");

  slider.oninput = function() {
  MAX_SAMPLES = parseFloat(this.value);
  display.textContent = MAX_SAMPLES.toFixed(1); // Actualiza el texto en pantalla
  // Ejemplo: Podrías usarlo para ajustar scaleY en tu gráfica en tiempo real
  console.log("Nueva resolución:", MAX_SAMPLES);
};

const smoothSlider = document.getElementById("smoothSlider");
const smoothDisplay = document.getElementById("smoothvalDisplay");

smoothSlider.oninput = function () {
factorSuavizado = parseFloat(this.value);
smoothDisplay.textContent = factorSuavizado.toFixed(2); // Actualiza el texto en pantalla
console.log("Nuevo suavizado:", factorSuavizado);
}
// DESCARGA CSV
downloadBtn.onclick = () => {
  if (csvContent.length === 0) return alert("No hay datos");

  let csvRows = ["Timestamp,Valor,fValor,BMP"];
  csvContent.forEach(row => {
    csvRows.push(`${row.timestamp},${row.valor},${row.fvalor}`);
  });

  const blob = new Blob([csvRows.join("\n")], { type: 'text/csv' });
  const url = window.URL.createObjectURL(blob);
  const a = document.createElement('a');
  
  a.setAttribute('hidden', '');
  a.setAttribute('href', url);
  a.setAttribute('download', `ecg_data_${Date.now()}.csv`);
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
};
function mapValue(x, inMin, inMax, outMin, outMax) {
  return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
};
async function draw() {
  ctx.clearRect(0, 0, WIDTH, HEIGHT);
  
  if (data.length < 2) return;

  ctx.strokeStyle = "#00ff88";
  ctx.lineWidth = 1.5; // Un poco más grueso para apreciar el suavizado
  ctx.lineJoin = "round";
  ctx.lineCap = "round";
  ctx.beginPath();

  // El factor 4ms (FS=250) ya está implícito en la relación 
  // entre MAX_SAMPLES y el ancho del canvas. 
  // Si MAX_SAMPLES representa el tiempo visible total:
  
  let i = 0;
  const getX = (index) => (index / MAX_SAMPLES) * WIDTH;
  const getY = (value) => mapValue(value, 0, 1023, HEIGHT, 0);

  ctx.moveTo(getX(0), getY(data[0]));

  // Algoritmo de Spline mediante puntos medios y quadraticCurveTo
  for (i = 1; i < data.length - 2; i++) {
    const xc = (getX(i) + getX(i + 1)) / 2;
    const yc = (getY(data[i]) + getY(data[i + 1])) / 2;
    ctx.quadraticCurveTo(getX(i), getY(data[i]), xc, yc);
  }

  // Para los últimos dos puntos
  if (i > 0) {
    ctx.quadraticCurveTo(
      getX(i), 
      getY(data[i]), 
      getX(i + 1), 
      getY(data[i + 1])
    );
  }

  ctx.stroke();

  // Llamar a FFT
  drawFFT();
}

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
  if (data.length < FFT_SIZE) return;

  const segment = data.slice(-FFT_SIZE);
  const mean = segment.reduce((a, b) => a + b, 0) / segment.length;
  const centered = segment.map(v => v - mean);
  const spectrum = fftReal(centered);
  // const spectrum = fftReal(segment);

  const W = fftCanvas.width;
  const H = fftCanvas.height;
  const nyquist = FS / 2;

  fftCtx.clearRect(0, 0, W, H);

  // ===== GRID / EJES =====
  fftCtx.strokeStyle = "#1f2937";
  fftCtx.lineWidth = 1;

  fftCtx.beginPath();

  // líneas verticales de frecuencia
  const freqMarks = [0, 4, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110];
  freqMarks.forEach(f => {
    if (f > nyquist) return;
    const x = (f / nyquist) * W;
    fftCtx.moveTo(x, 0);
    fftCtx.lineTo(x, H);
  });

  // líneas horizontales
  for (let i = 1; i <= 4; i++) {
    const y = (i / 5) * H;
    fftCtx.moveTo(0, y);
    fftCtx.lineTo(W, y);
  }

  fftCtx.stroke();

  // ===== ESPECTRO =====
  fftCtx.strokeStyle = "#38bdf8";
  fftCtx.lineWidth = 2;
  fftCtx.beginPath();

  spectrum.slice(0, FFT_SIZE / 2).forEach((v, i) => {
    const freq = (i * FS) / FFT_SIZE;
    const half = spectrum.slice(0, FFT_SIZE / 2);
    const maxVal = Math.max(...half) || 1;
    const scale = (H * 0.95) / maxVal;
    const x = (freq / nyquist) * W;
    const y = H - v * scale;

    if (i === 0) fftCtx.moveTo(x, y);
    else fftCtx.lineTo(x, y);
  });

  fftCtx.stroke();

  // ===== ETIQUETAS =====
  fftCtx.fillStyle = "#9ca3af";
  fftCtx.font = "10px monospace";

  freqMarks.forEach(f => {
    if (f > nyquist) return;
    const x = (f / nyquist) * W;
    fftCtx.fillText(`${f}Hz`, x + 2, H - 4);
  });
}
</script>
</body>
</html>)rawliteral";