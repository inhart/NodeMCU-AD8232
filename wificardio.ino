
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Arduino.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Ticker.h>
#include "page.h"


const char* ssid = "ECG";
const char* password = "cardio2026!";

#define SENSOR A0
#define D5_PIN D5
#define D6_PIN D6

#define BUF_SIZE 50


const unsigned long spu = 4;  // intervalo en ms
unsigned long lasts = 0;

// Create AsyncWebServer object on port 80
static AsyncWebServer server(80);

// --- WebSocket ---

static WebSocketsServer webSocket(81);


// --- Double Buffer ---
struct Sample {
  uint16_t value;
  //uint32_t ts;  // timestamp en ms
};

struct Packet {
  uint16_t n;  // número de samples
  Sample samples[BUF_SIZE];
};

Sample bufferA[BUF_SIZE];
Sample bufferB[BUF_SIZE];

Sample* writeBuf = bufferA;
Sample* sendBuf = bufferB;

volatile uint8_t idx = 0;
volatile bool bufferReady = false;

void swapBuffer() {
  // swap buffers
  Sample* tmp;
  noInterrupts();
  tmp = (Sample*)writeBuf;
  writeBuf = sendBuf;
  sendBuf = tmp;
  idx = 0;
  bufferReady = false;
  interrupts();
}

// --- Muestreo ECG ---
void IRAM_ATTR sampleECG() {
  if (bufferReady) { return; }
  if ((digitalRead(D5_PIN) == 0) && (digitalRead(D6_PIN) == 0)) {

    //uint16_t val = analogRead(SENSOR) & 0x03ff;

    //delay(1);
    writeBuf[idx].value = system_adc_read();  //& 0x03ff constrain(val, 0, 1023);
    //writeBuf[idx].ts =system_get_time();;
    idx++;
    if (idx >= BUF_SIZE) {
      // swap buffers
      bufferReady = true;
    }
  }
}

// --- Serial ---
// void enviarSerial() {
//   for (int i = 0; i < BUF_SIZE; i++) {
//     Serial.println(sendBuf[i].value);
//   }
// }

// --- WebSocket binario ---
void enviarWebSocket() {


  //ws.binaryAll((uint8_t*)sendBuf, sizeof(Sample) * BUF_SIZE);
  uint8_t payload[BUF_SIZE * 2];
  uint16_t p = 0;
  //uint16_t siz = sizeof(&sendBuf->value);
  for (int i = 0; i < BUF_SIZE; i++) {
    //  memcpy(&payload[p], &sendBuf[i].ts, 4);
    //  p += 4;

    memcpy(&payload[p], &sendBuf[i].value, 2);
    p += 2;
  }
  webSocket.broadcastBIN(payload, p);
  webSocket.loop();
}



// --- Setup ---
void setup() {
  // Serial.begin(115200);
  pinMode(SENSOR, INPUT);
  pinMode(D5_PIN, INPUT);
  pinMode(D6_PIN, INPUT);
  WiFi.softAP(ssid, password);


  // Route for root / web page
  server.begin();
  webSocket.begin();
  webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {});



  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", index_html);
  });
}


// --- Loop principal ---
void loop() {

  unsigned long now = millis();
  if (now - lasts >= spu) {
    lasts = now;
    noInterrupts();
    sampleECG();
    interrupts();
  }
  if (bufferReady) {
    swapBuffer();
    //enviarSerial();  // debug
    enviarWebSocket();  // gráfica en navegador
  }
}
