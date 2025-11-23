#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ================== PIN ==================
const int RELAY1_PIN = D1;   // attivo LOW
const int RELAY2_PIN = D2;   // attivo LOW
const int DS_PIN = D4;       // DS18B20

// ================== DS18B20 ==================
OneWire oneWire(DS_PIN);
DallasTemperature sensors(&oneWire);

// ================== WEB + DNS ==================
ESP8266WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

// ================== EEPROM STRUCT ==================
struct Config {
  uint32_t magic;
  float sogliaAlta;    
  float sogliaBassa;   
  uint32_t crc;
};

Config conf;

const uint32_t MAGIC = 0xAABBCCDD;
const int EEPROM_SIZE = 128;
const float DEFAULT_ALTA = 50;
const float DEFAULT_BASSA = 40;

// ================== STATO ==================
float tempNow = NAN;
bool relaysOn = false;

// ================== UTILS ==================
uint32_t simpleCrc(const uint8_t *data, size_t len) {
  uint32_t c = 0xDEADBEEF;
  for (size_t i = 0; i < len; i++) c = (c << 5) ^ (c >> 27) ^ data[i];
  return c;
}

void saveConfig() {
  conf.crc = simpleCrc((uint8_t*)&conf, sizeof(Config) - sizeof(uint32_t));
  EEPROM.put(0, conf);
  EEPROM.commit();
}

void loadConfig() {
  EEPROM.begin(EEPROM_SIZE);
  Config tmp;
  EEPROM.get(0, tmp);

  uint32_t crcOK = simpleCrc((uint8_t*)&tmp, sizeof(Config) - sizeof(uint32_t));

  if (tmp.magic != MAGIC || crcOK != tmp.crc ||
      tmp.sogliaAlta < -40 || tmp.sogliaAlta > 125 ||
      tmp.sogliaBassa < -40 || tmp.sogliaBassa > 125 ||
      tmp.sogliaBassa >= tmp.sogliaAlta) 
  {
    conf.magic = MAGIC;
    conf.sogliaAlta = DEFAULT_ALTA;
    conf.sogliaBassa = DEFAULT_BASSA;
    saveConfig();
  } else {
    conf = tmp;
  }
}

// ================== RELÈ ==================
void setRelays(bool on) {
  relaysOn = on;
  digitalWrite(RELAY1_PIN, on ? LOW : HIGH);
  digitalWrite(RELAY2_PIN, on ? LOW : HIGH);
}

// ================== LOGICA TEMPERATURA ==================
void applyLogic(float t) {
  if (isnan(t)) return;

  if (t >= conf.sogliaAlta) setRelays(true);
  else if (t <= conf.sogliaBassa) setRelays(false);
}

// ================== HTML ==================
String htmlPage() {
  String s = "";
  s += "<html><head><meta name='viewport' content='width=device-width,initial-scale=1'>";
  s += "<style>body{font-family:Arial;margin:20px;}input{width:100%;padding:8px;margin:5px 0;}";
  s += "button{padding:10px;width:100%;background:#4285F4;color:#fff;border:0;border-radius:6px;}</style>";
  s += "</head><body>";
  s += "<h2>TempRelays - Configurazione</h2>";
  s += "<form action='/save' method='POST'>";
  s += "Soglia ALTA (>=):<input name='alta' value='" + String(conf.sogliaAlta) + "'>";
  s += "Soglia BASSA (<=):<input name='bassa' value='" + String(conf.sogliaBassa) + "'>";
  s += "<button type='submit'>Salva</button>";
  s += "</form>";

  s += "<hr>Temperatura attuale: <b>";
  if (isnan(tempNow)) s += "--";
  else s += String(tempNow, 1);
  s += " °C</b><br>";

  s += "Relè: <b>" + String(relaysOn ? "ON" : "OFF") + "</b>";

  s += "</body></html>";
  return s;
}

void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleSave() {
  float alta  = server.arg("alta").toFloat();
  float bassa = server.arg("bassa").toFloat();

  if (alta <= bassa || alta < -40 || alta > 125 || bassa < -40 || bassa > 125) {
    server.send(400, "text/plain", "Valori non validi");
    return;
  }

  conf.sogliaAlta = alta;
  conf.sogliaBassa = bassa;
  saveConfig();

  server.send(200, "text/plain", "Salvato!");
}

// ================== SETUP ==================
unsigned long lastRead = 0;

void setup() {
  Serial.begin(115200);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  setRelays(false);

  loadConfig();
  sensors.begin();

  WiFi.mode(WIFI_AP);
  WiFi.softAP("TempRelays-Setup", "12345678");

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.begin();

  Serial.println("AP avviato. SSID: TempRelays-Setup  PASS: 12345678");
}

// ================== LOOP ==================
void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (millis() - lastRead > 1500) {
    lastRead = millis();
    sensors.requestTemperatures();
    tempNow = sensors.getTempCByIndex(0);
    applyLogic(tempNow);
  }
}
