#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

/* Versione 1.0 */

/* ===================== PIN ===================== */
#define ONE_WIRE_BUS D2          // DS18B20
#define RELAY_PIN_1  D1          // Relè 1
#define RELAY_PIN_2  D5          // Relè 2

/* ================== SENSORI ==================== */
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

/* ================== EEPROM ===================== */
// Struttura EEPROM: [0..7]=sogliaAlta (float) | [8..11]=sogliaBassa (float) | [12]=modalitaAuto (uint8_t)
#define EEPROM_SIZE        16
#define EEPROM_ADDR_ALTA    0
#define EEPROM_ADDR_BASSA   8
#define EEPROM_ADDR_AUTO   12

float   sogliaAlta  = 52.0;
float   sogliaBassa = 40.0;
bool    modalitaAuto = true; // true=AUTO, false=MANUALE

/* ================== WIFI AP ==================== */
const char* ssid = "SA-Robotics-ControlloCamino";
const char* password = "12345678";
const byte  DNS_PORT = 53;

DNSServer dnsServer;
ESP8266WebServer server(80);

/* ============ LOGO (PLACEHOLDER) ============== */
// Sostituisci questa stringa col tuo base64 quando vuoi.
const char logoBase64[] = "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";

/* ================== HTML PAGE ================== */
String htmlPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="it">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Controllo Camino</title>

<style>
:root{
  --blue:#1976d2;
  --green:#2e7d32;
  --orange:#f57c00;
  --grey:#bdbdbd;
}

body{
  font-family: Arial, sans-serif;
  background:#fff;
  margin:0;
  padding:20px;
  text-align:center;
}

h2{ margin:10px 0 16px; }

img#logo{
  width:140px;
  margin-top:6px;
}

.value{
  font-size:20px;
  margin:12px 0;
}

/* ---- stato pompe ---- */
.status-row{
  display:flex;
  align-items:center;
  justify-content:center;
  gap:8px;
  font-size:20px;
  margin:12px 0;
}

.status-dot{
  width:12px;
  height:12px;
  border-radius:50%;
  background:var(--grey);
}
.status-dot.on{ background:var(--green); }

/* ---- accordion ---- */
.accordion{
  margin-top:20px;
  max-width:420px;
  margin-left:auto;
  margin-right:auto;
}

.accordion-header{
  display:flex;
  justify-content:center;
  cursor:pointer;
  padding:10px 0;
  border-bottom:1px solid #ddd;
}

.accordion-header .arrow{
  font-size:20px;
  color:#9e9e9e;
  letter-spacing:4px;
  transition:transform .2s ease;
}

.accordion.open .accordion-header .arrow{
  transform:rotate(180deg);
}

.accordion-content{
  display:none;
  padding-top:14px;
  text-align:left;
}
.accordion.open .accordion-content{ display:block; }

/* ---- soglie ---- */
.thresholds-grid{
  display:grid;
  grid-template-columns:1fr auto;
  column-gap:16px;
  align-items:start;
}

.thresholds-left{
  display:flex;
  flex-direction:column;
  gap:14px;
}

.thresholds-left label{
  font-weight:bold;
  display:block;
  margin-bottom:4px;
}

.thresholds-left input{
  width:120px;
  text-align:center;
  padding:8px;
  font-size:18px;
  border:1px solid #ccc;
  border-radius:6px;
}

/* ---- salva ---- */
.thresholds-action{
  display:flex;
  align-items:flex-start;
  padding-top:22px;
}

button.save.icon-only{
  background:transparent;
  color:var(--green);
  font-size:22px;
  padding:6px;
  width:auto;
  border:none;
}

button.save.icon-only:disabled{
  opacity:.3;
  cursor:not-allowed;
}

@keyframes flashGreen{
  0%{ transform:scale(1); }
  50%{ transform:scale(1.2); }
  100%{ transform:scale(1); }
}
button.save.flash{
  animation:flashGreen .3s ease;
}

/* ---- pulsanti ---- */
button{
  display:block;
  width:220px;
  margin:10px auto;
  padding:12px;
  font-size:16px;
  border:none;
  border-radius:6px;
  color:#fff;
}

button.auto{ background:var(--blue); }
button.manual{ background:var(--orange); }
button.on{ background:var(--green); }
button.off{ background:var(--grey); }

button.outline{
  background:#fff!important;
  color:inherit;
  box-shadow:inset 0 0 0 2px currentColor;
}

.button-row{
  display:flex;
  gap:12px;
  justify-content:center;
}

.hidden{ display:none!important; }

.separator{
  height:1px;
  background:#e0e0e0;
  margin:18px 0;
}

small{
  color:#666;
  text-align:center;
}
</style>
</head>

<body>

<img id="logo" src="data:image/png;base64,)rawliteral" + String(logoBase64) + R"rawliteral(">

<h2>Controllo Camino</h2>

<div class="value">Temperatura: <b id="temp">--</b> °C</div>

<div class="status-row">
  <div id="relayDot" class="status-dot"></div>
  <div>Pompe: <b id="relay">--</b></div>
</div>

<div class="value">Modalità: <b id="mode">--</b></div>

<div class="accordion" id="accordion">
  <div class="accordion-header" onclick="toggleAccordion()">
    <span class="arrow">⌄⌄</span>
  </div>

  <div class="accordion-content">

    <button class="auto" id="btnAuto" onclick="setAuto()">AUTO</button>


    <div class="thresholds-grid">

      <div class="thresholds-left">
        <div>
          <label>Temp Attivazione pompe</label>
          <input id="alta" type="number" step="0.1">
        </div>

        <div>
          <label>Temp Disattivazione pompe</label>
          <input id="bassa" type="number" step="0.1">
        </div>
      </div>

      <div class="thresholds-action">
        <button class="save icon-only"
                id="btnSave"
                onclick="salva()"
                title="Salva soglie"
                disabled>💾</button>
      </div>

    </div>

    <div class="separator"></div>

    <button class="manual" id="btnManual" onclick="setManual()">Modalità MANUALE</button>
    <small>In modalità AUTO i controlli manuali non sono disponibili</small>

    <div class="button-row hidden" id="manualControls">
      <button class="on" id="btnOn" onclick="relayOn()">ON</button>
      <button class="off" id="btnOff" onclick="relayOff()">OFF</button>
    </div>

  </div>
</div>

<script>
let lastAlta=null, lastBassa=null;
let editing=false;

function toggleAccordion(){
  accordion.classList.toggle('open');
}

async function aggiorna(){
  if(editing) return;

  const r = await fetch('/status');
  const j = await r.json();

  temp.innerText=j.temp;
  relay.innerText=j.relay;
  mode.innerText=j.mode;

  relayDot.classList.toggle('on', j.relay==="ON");

  if(lastAlta===null){
    lastAlta=j.alta;
    lastBassa=j.bassa;
  }

  alta.value=j.alta;
  bassa.value=j.bassa;

  btnSave.disabled=(alta.value==lastAlta && bassa.value==lastBassa);

  const isManual=(j.mode==="MANUALE");

  manualControls.classList.toggle('hidden', !isManual);
  btnOn.disabled=!isManual;
  btnOff.disabled=!isManual;

  btnAuto.classList.toggle('outline', j.mode!=="AUTO");
  btnManual.classList.toggle('outline', j.mode!=="MANUALE");
}

async function salva(){
  btnSave.disabled=true;
  await fetch(`/set?alta=${alta.value}&bassa=${bassa.value}`);
  lastAlta=alta.value;
  lastBassa=bassa.value;
  editing=false;
  btnSave.classList.add('flash');
  setTimeout(()=>btnSave.classList.remove('flash'),300);
}

async function relayOn(){ await fetch('/relay/on'); aggiorna(); }
async function relayOff(){ await fetch('/relay/off'); aggiorna(); }
async function setAuto(){ await fetch('/mode?auto=1'); aggiorna(); }
async function setManual(){ await fetch('/mode?auto=0'); aggiorna(); }

alta.addEventListener('input',()=>{ editing=true; btnSave.disabled=false; });
bassa.addEventListener('input',()=>{ editing=true; btnSave.disabled=false; });

setInterval(aggiorna,2000);
aggiorna();
</script>

</body>
</html>
)rawliteral";
}




/* ================== UTIL / EEPROM ================== */
void salvaSoglie() {
  EEPROM.put(EEPROM_ADDR_ALTA,  sogliaAlta);
  EEPROM.put(EEPROM_ADDR_BASSA, sogliaBassa);
  EEPROM.commit();
}

void salvaModalita() {
  uint8_t m = modalitaAuto ? 1 : 0;
  EEPROM.put(EEPROM_ADDR_AUTO, m);
  EEPROM.commit();
}

/* ================== API HANDLERS ================== */
void handleStatus() {
  sensors.requestTemperatures();
  float t = sensors.getTempCByIndex(0);

  String tempStr = (t == DEVICE_DISCONNECTED_C) ? String("--") : String(t, 1);
  String json = "{";
  json += "\"temp\":\""  + tempStr + "\",";
  json += "\"relay\":\"" + String(digitalRead(RELAY_PIN_1) ? "ON" : "OFF") + "\",";
  json += "\"alta\":"    + String(sogliaAlta, 1) + ",";
  json += "\"bassa\":"   + String(sogliaBassa, 1) + ",";
  json += "\"mode\":\""  + String(modalitaAuto ? "AUTO" : "MANUALE") + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void handleSet() {
  if (server.hasArg("alta"))  sogliaAlta  = server.arg("alta").toFloat();
  if (server.hasArg("bassa")) sogliaBassa = server.arg("bassa").toFloat();
  salvaSoglie();
  server.send(200, "text/plain", "OK");
}

void handleMode() {
  if (server.hasArg("auto")) {
    modalitaAuto = (server.arg("auto") == "1");
    salvaModalita();
  }
  server.send(200, "text/plain", "OK");
}

/* ================== LOGICA RELÈ ================== */
void controllaRele(float t) {
  if (!modalitaAuto) return;                // In MANUALE non toccare i relè
  if (t == DEVICE_DISCONNECTED_C) return;   // Niente sensore → nessuna azione

  if (t >= sogliaAlta) {
    digitalWrite(RELAY_PIN_1, HIGH);
    digitalWrite(RELAY_PIN_2, HIGH);
  }
  if (t <= sogliaBassa) {
    digitalWrite(RELAY_PIN_1, LOW);
    digitalWrite(RELAY_PIN_2, LOW);
  }
}

/* ====================== SETUP ===================== */
void setup() {
  // IO
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  digitalWrite(RELAY_PIN_1, LOW);
  digitalWrite(RELAY_PIN_2, LOW);

  // EEPROM
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(EEPROM_ADDR_ALTA,  sogliaAlta);
  EEPROM.get(EEPROM_ADDR_BASSA, sogliaBassa);
  if (isnan(sogliaAlta))  sogliaAlta  = 52.0;
  if (isnan(sogliaBassa)) sogliaBassa = 40.0;
  uint8_t modeByte = 1;
  EEPROM.get(EEPROM_ADDR_AUTO, modeByte);
  if (modeByte != 0 && modeByte != 1) modeByte = 1;
  modalitaAuto = (modeByte == 1);

  // Sensore
  sensors.begin();

  // AP + DNS
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  // ROUTES
  server.on("/", [](){ server.send(200, "text/html; charset=utf-8", htmlPage()); });
  server.on("/status", handleStatus);
  server.on("/set",    handleSet);
  server.on("/mode",   handleMode);

  server.on("/relay/on", [](){
    digitalWrite(RELAY_PIN_1, HIGH);
    digitalWrite(RELAY_PIN_2, HIGH);
    server.send(200, "text/plain", "ON");
  });
  server.on("/relay/off", [](){
    digitalWrite(RELAY_PIN_1, LOW);
    digitalWrite(RELAY_PIN_2, LOW);
    server.send(200, "text/plain", "OFF");
  });

  // Captive portal (Android/iOS) + fallback
  server.on("/generate_204", [](){ server.sendHeader("Location","/",true); server.send(302); });
  server.on("/hotspot-detect.html", [](){ server.sendHeader("Location","/",true); server.send(302); });
  server.on("/connectivitycheck.gstatic.com/generate_204", [](){ server.sendHeader("Location","/",true); server.send(302); });
  server.onNotFound([](){ server.sendHeader("Location","/",true); server.send(302); });

  server.begin();
}

/* ======================= LOOP ==================== */
unsigned long lastRead = 0;

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (millis() - lastRead > 2000) {
    lastRead = millis();
    sensors.requestTemperatures();
    controllaRele(sensors.getTempCByIndex(0));
  }
}
