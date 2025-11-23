#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

#define ONE_WIRE_BUS D2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define RELAY_PIN D1
#define LED_PIN LED_BUILTIN

#define EEPROM_SIZE 16
float sogliaAlta = 52.0;
float sogliaBassa = 40.0;

const char* ssid = "TempRelays-Setup";
const char* password = "12345678";
const byte DNS_PORT = 53;

DNSServer dnsServer;
ESP8266WebServer server(80);

// Logo base64
const char logoBase64[] = "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";

String htmlPage() {
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  String tempStr = (temp == DEVICE_DISCONNECTED_C ? "--" : String(temp, 1));
  String relayState = digitalRead(RELAY_PIN) ? "ON" : "OFF";
  String relayColor = digitalRead(RELAY_PIN) ? "#00b020" : "#d00000";

  String page =
    "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>Controllo Camino</title>"
    "<style>"
    "body { background:#ffffff; font-family:sans-serif; margin:20px; }"
    ".center { text-align:center; }"
    "img { width:120px; margin-bottom:12px; }"
    "label { display:block; margin-top:14px; font-weight:bold; }"
    "input { width:85%; padding:10px; font-size:18px; border:1px solid #888; border-radius:6px; }"
    "button { width:90%; margin-top:22px; padding:14px; font-size:18px; background:#1976d2; color:#fff; border:none; border-radius:6px; }"
    ".dot { display:inline-block; width:12px; height:12px; border-radius:50%; margin-left:6px; }"
    "</style></head><body>"

    "<div class='center'>"
    "<img src='data:image/png;base64," + String(logoBase64) + "'>"
    "<h2>Controllo Camino</h2>"
    "</div>"

    "<p>Temperatura attuale: <b>" + tempStr + " C</b></p>"
    "<p>Stato rele: <b>" + relayState + "</b>"
    "<span class='dot' style='background:" + relayColor + ";'></span></p>"

    "<form method='POST' action='/save'>"
    "<label>Soglia ALTA (ON >=):</label>"
    "<input name='alta' type='number' step='0.1' value='" + String(sogliaAlta) + "'>"

    "<label>Soglia BASSA (OFF <=):</label>"
    "<input name='bassa' type='number' step='0.1' value='" + String(sogliaBassa) + "'>"

    "<button type='submit'>Salva</button>"

    "<button style='background:#28a745; margin-top:12px;' formaction='/on'>Attiva Rele</button>"
    "<button style='background:#d32f2f; margin-top:8px;' formaction='/off'>Disattiva Rele</button>"

    "</form>"

    "</body></html>";

  return page;
}

void handleSave() {
  if (server.hasArg("alta")) sogliaAlta = server.arg("alta").toFloat();
  if (server.hasArg("bassa")) sogliaBassa = server.arg("bassa").toFloat();

  EEPROM.put(0, sogliaAlta);
  EEPROM.put(8, sogliaBassa);
  EEPROM.commit();

  server.send(200, "text/html","<html><body><h3>Salvato.</h3><a href='/'>Torna</a></body></html>");
}

void controllaRele(float t) {
  if (t == DEVICE_DISCONNECTED_C) return;
  if (t >= sogliaAlta) digitalWrite(RELAY_PIN, HIGH);
  if (t <= sogliaBassa) digitalWrite(RELAY_PIN, LOW);
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, sogliaAlta);
  EEPROM.get(8, sogliaBassa);

  if (isnan(sogliaAlta)) sogliaAlta = 52.0;
  if (isnan(sogliaBassa)) sogliaBassa = 40.0;

  sensors.begin();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/", []() { server.send(200, "text/html", htmlPage()); });
  server.on("/generate_204", []() { server.sendHeader("Location","/",true); server.send(302); });
  server.on("/hotspot-detect.html", []() { server.sendHeader("Location","/",true); server.send(302); });
  server.on("/connectivitycheck.gstatic.com/generate_204", []() { server.sendHeader("Location","/",true); server.send(302); });
  server.on("/save", HTTP_POST, handleSave);

  server.on("/on", []() {
    digitalWrite(RELAY_PIN, HIGH);
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.on("/off", []() {
    digitalWrite(RELAY_PIN, LOW);
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
}

unsigned long lastRead = 0;

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (millis() - lastRead > 2000) {
    lastRead = millis();
    sensors.requestTemperatures();
    float t = sensors.getTempCByIndex(0);
    controllaRele(t);
  }
}
