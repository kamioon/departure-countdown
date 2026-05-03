#include "web_server.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// ---------------------------------------------------------------------------
// Web UI — single-page app served from flash
// ---------------------------------------------------------------------------
static const char INDEX_HTML[] PROGMEM = R"rawhtml(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Departure Countdown</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:sans-serif;background:#1a1a2e;color:#e0e0e0;padding:16px;max-width:520px;margin:0 auto}
h1{color:#7ec8e3;margin-bottom:16px;font-size:1.4em}
h2{font-size:.78em;color:#888;margin-bottom:10px;text-transform:uppercase;letter-spacing:1px}
.card{background:#16213e;border-radius:8px;padding:16px;margin-bottom:14px}
.row{display:flex;gap:10px;align-items:center;flex-wrap:wrap;margin-bottom:8px}
.badge{border-radius:4px;padding:3px 9px;font-size:.8em;background:#0f3460}
.badge.ok{background:#1b4332}.badge.warn{background:#5c1a1a}
.sync{display:none;background:#5c3a00;color:#ffa500;border-radius:4px;padding:2px 8px;font-size:.75em}
.cnt{font-size:2em;font-weight:700;color:#7ec8e3}
.dir{font-size:.75em;color:#888;margin-top:2px}
.info-grid{display:grid;grid-template-columns:1fr 1fr;gap:6px 16px;margin-top:6px}
.info-item{font-size:.8em}.info-item span{color:#7ec8e3}
label{display:block;font-size:.8em;color:#aaa;margin:10px 0 3px}
input,select{width:100%;background:#0d0d1a;border:1px solid #2a2a4a;color:#e0e0e0;padding:7px 9px;border-radius:4px;font-size:.9em}
button{padding:8px 18px;border:none;border-radius:4px;cursor:pointer;font-size:.88em;font-weight:600}
.btn-p{background:#7ec8e3;color:#1a1a2e}.btn-d{background:#e37e7e;color:#1a1a2e}
.btn-sm{padding:5px 12px;font-size:.8em}
.modes{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;margin-top:8px}
.m{padding:10px;border:2px solid #2a2a4a;border-radius:6px;background:#0f3460;color:#e0e0e0;cursor:pointer;font-size:.85em;transition:.15s}
.m.active{border-color:#7ec8e3;color:#7ec8e3}
.msg{padding:8px 10px;border-radius:4px;margin-top:10px;font-size:.85em;display:none}
.msg.ok{background:#1b4332;display:block}.msg.err{background:#5c1a1a;display:block}
.acts{display:flex;gap:8px;margin-top:14px;flex-wrap:wrap}
hr{border:none;border-top:1px solid #2a2a4a;margin:12px 0}
</style>
</head>
<body>
<h1>&#x1F686; Departure Countdown</h1>

<div class="card">
  <h2>Live Status</h2>
  <div class="row">
    <span id="wb" class="badge">WiFi</span>
    <span id="rb" class="badge"></span>
    <span id="sb" class="sync">SYNCING</span>
  </div>
  <div class="row" style="margin-top:10px;justify-content:space-around">
    <div style="text-align:center">
      <div style="font-size:.8em;color:#aaa;margin-bottom:4px">&#x1F6B6; Walk</div>
      <div id="tw" class="cnt">--:--</div>
      <div id="dw" class="dir"></div>
    </div>
    <div style="text-align:center">
      <div style="font-size:.8em;color:#aaa;margin-bottom:4px">&#x1F6B2; Bike</div>
      <div id="tb" class="cnt">--:--</div>
      <div id="db" class="dir"></div>
    </div>
  </div>
  <div class="acts">
    <button class="btn-p btn-sm" onclick="doFetch()">&#x21BB; Fetch Now</button>
  </div>

  <hr>
  <h2>Device</h2>
  <div class="info-grid">
    <div class="info-item">Chip &nbsp;<span id="di-chip">—</span></div>
    <div class="info-item">CPU &nbsp;<span id="di-cpu">—</span></div>
    <div class="info-item">Flash &nbsp;<span id="di-flash">—</span></div>
    <div class="info-item">Free heap &nbsp;<span id="di-heap">—</span></div>
    <div class="info-item">Uptime &nbsp;<span id="di-up">—</span></div>
    <div class="info-item">Display &nbsp;<span id="di-disp">—</span></div>
  </div>
</div>

<div class="card">
  <h2>Transport Mode</h2>
  <div class="modes">
    <button class="m" id="m-walk" onclick="setMode('walk')">&#x1F6B6;<br>Walk</button>
    <button class="m" id="m-bike" onclick="setMode('bike')">&#x1F6B2;<br>Bike</button>
    <button class="m" id="m-bus"  onclick="setMode('bus')">&#x1F68C;<br>Bus</button>
  </div>
</div>

<div class="card">
  <h2>Configuration</h2>
  <form id="cf" onsubmit="return false">
    <label>Station Code</label>
    <input name="stationCode" maxlength="4" placeholder="e.g. AMST">
    <label>Walk time (min)</label>
    <input name="walkTime" type="number" min="1" max="120">
    <label>Bike time (min)</label>
    <input name="bikeTime" type="number" min="1" max="120">
    <label>Bus time (min)</label>
    <input name="busTime" type="number" min="1" max="120">
    <label>Buffer time (min)</label>
    <input name="bufferTime" type="number" min="0" max="30">
    <label>NS API Key <span style="color:#666">(leave blank to keep current)</span></label>
    <input name="nsApiKey" type="password" autocomplete="new-password">
    <label>WiFi SSID <span style="color:#666">(leave blank to keep current)</span></label>
    <input name="wifiSsid" autocomplete="off">
    <label>WiFi Password <span style="color:#666">(leave blank to keep current)</span></label>
    <input name="wifiPassword" type="password" autocomplete="new-password">
    <div class="acts">
      <button type="button" class="btn-p" onclick="saveConfig()">Save</button>
      <button type="button" class="btn-d btn-sm" onclick="if(confirm('Factory reset? Device will restart.'))factoryReset()">Factory Reset</button>
    </div>
  </form>
  <div id="cm" class="msg"></div>
</div>

<script>
function fmt(s){
  if(s==null||s==undefined)return'--:--';
  if(s<=0)return'00:00';
  var m=Math.floor(s/60),sc=s%60;
  return(m<10?'0'+m:m)+':'+(sc<10?'0'+sc:sc);
}
function fmtUptime(s){
  var h=Math.floor(s/3600),m=Math.floor((s%3600)/60),sc=s%60;
  if(h>0)return h+'h '+m+'m';
  if(m>0)return m+'m '+sc+'s';
  return sc+'s';
}
function tick(){
  fetch('/api/status').then(r=>r.json()).then(d=>{
    var wc=d.wifi&&d.wifi.connected;
    document.getElementById('wb').textContent=wc?'WiFi: '+d.wifi.ip:'WiFi: off';
    document.getElementById('wb').className='badge '+(wc?'ok':'warn');
    document.getElementById('rb').textContent=(d.wifi&&d.wifi.rssi?d.wifi.rssi:'')+' dBm';
    document.getElementById('sb').style.display=d.fetchInProgress?'inline':'none';
    document.getElementById('tw').textContent=fmt(d.walk&&d.walk.secondsUntilLeave);
    document.getElementById('tb').textContent=fmt(d.bike&&d.bike.secondsUntilLeave);
    document.getElementById('dw').textContent=(d.walk&&d.walk.direction)||'';
    document.getElementById('db').textContent=(d.bike&&d.bike.direction)||'';
    var cur=(d.mode||'').toLowerCase();
    ['walk','bike','bus'].forEach(m=>document.getElementById('m-'+m).className='m'+(cur===m?' active':''));
    if(d.device){
      var dv=d.device;
      document.getElementById('di-chip').textContent=(dv.chipModel||'?')+' rev'+dv.chipRevision;
      document.getElementById('di-cpu').textContent=dv.cpuMhz+'MHz';
      document.getElementById('di-flash').textContent=dv.flashKb+'KB';
      document.getElementById('di-heap').textContent=dv.heapFreeKb+'KB / '+dv.heapTotalKb+'KB';
      document.getElementById('di-up').textContent=fmtUptime(dv.uptimeSeconds);
    }
    if(d.peripherals){
      var disp=d.peripherals.display;
      var el=document.getElementById('di-disp');
      el.textContent=disp?'Connected':'Not found';
      el.style.color=disp?'#7ec8e3':'#e37e7e';
    }
  }).catch(()=>{});
}
function loadCfg(){
  fetch('/api/config').then(r=>r.json()).then(d=>{
    var f=document.getElementById('cf');
    ['stationCode','walkTime','bikeTime','busTime','bufferTime'].forEach(k=>{
      if(f[k]&&d[k]!=null)f[k].value=d[k];
    });
  });
}
function saveConfig(){
  var f=document.getElementById('cf'),body={};
  ['stationCode'].forEach(k=>{if(f[k]&&f[k].value.trim())body[k]=f[k].value.trim();});
  ['walkTime','bikeTime','busTime','bufferTime'].forEach(k=>{if(f[k]&&f[k].value)body[k]=parseInt(f[k].value);});
  if(f.nsApiKey.value)body.nsApiKey=f.nsApiKey.value;
  if(f.wifiSsid.value){body.wifiSsid=f.wifiSsid.value;body.wifiPassword=f.wifiPassword.value;}
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})
    .then(r=>r.json()).then(d=>{
      var el=document.getElementById('cm');
      el.textContent=d.ok?'Saved successfully!':'Error: '+(d.error||'unknown');
      el.className='msg '+(d.ok?'ok':'err');
      if(d.ok){f.nsApiKey.value='';f.wifiPassword.value='';}
      setTimeout(()=>el.style.display='none',4000);
    }).catch(()=>{var el=document.getElementById('cm');el.textContent='Request failed';el.className='msg err';});
}
function setMode(m){
  fetch('/api/transport',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({mode:m})})
    .then(r=>r.json()).then(d=>{if(d.ok)tick();});
}
function doFetch(){
  fetch('/api/fetch',{method:'POST'}).then(()=>tick());
}
function factoryReset(){
  fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({factoryReset:true})})
    .then(()=>{var el=document.getElementById('cm');el.textContent='Factory reset. Rebooting...';el.className='msg ok';});
}
tick();loadCfg();
setInterval(tick,2000);
</script>
</body>
</html>
)rawhtml";

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void addCORS(AsyncWebServerResponse* resp) {
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
}

static void sendJSON(AsyncWebServerRequest* request, int code, const String& body) {
    AsyncWebServerResponse* resp = request->beginResponse(code, "application/json", body);
    addCORS(resp);
    request->send(resp);
}

// ---------------------------------------------------------------------------
// WebServerManager
// ---------------------------------------------------------------------------

WebServerManager::WebServerManager()
    : server(nullptr), configManager(nullptr), nsApiClient(nullptr), countdownCalc(nullptr) {}

WebServerManager::~WebServerManager() {
    if (server) delete server;
}

bool WebServerManager::begin(ConfigManager* config, NSApiClient* apiClient,
                              CountdownCalculator* calculator) {
    configManager = config;
    nsApiClient   = apiClient;
    countdownCalc = calculator;

    server = new AsyncWebServer(80);
    setupWebUI();
    setupRoutes();
    server->begin();

    Serial.print("Web server started — http://");
    Serial.println(WiFi.localIP());
    return true;
}

bool WebServerManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WebServerManager::getIPAddress() {
    return WiFi.localIP().toString();
}

void WebServerManager::update() {
    // AsyncWebServer is interrupt-driven; nothing to poll.
}

void WebServerManager::setFetchCallback(FetchCallback cb) {
    fetchCallback = cb;
}

void WebServerManager::updateStatus(long walkSeconds, long bikeSeconds, bool fetching,
                                     const String& walkDir, const String& bikeDir, int rssi) {
    statusCache.walkSecondsUntilLeave = walkSeconds;
    statusCache.bikeSecondsUntilLeave = bikeSeconds;
    statusCache.fetchInProgress       = fetching;
    statusCache.walkDir               = walkDir;
    statusCache.bikeDir               = bikeDir;
    statusCache.rssi                  = rssi;
}

void WebServerManager::setDisplayConnected(bool connected) {
    statusCache.displayConnected = connected;
}

// ---------------------------------------------------------------------------
// Route registration
// ---------------------------------------------------------------------------

void WebServerManager::setupWebUI() {
    server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send_P(200, "text/html", INDEX_HTML);
    });
}

void WebServerManager::setupRoutes() {
    // CORS preflight for cross-origin smarthome clients
    server->on("/api/status", HTTP_OPTIONS, [](AsyncWebServerRequest* req) {
        AsyncWebServerResponse* resp = req->beginResponse(204);
        addCORS(resp);
        req->send(resp);
    });

    // GET /api/status
    server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* req) {
        handleApiStatus(req);
    });

    // GET /api/departures
    server->on("/api/departures", HTTP_GET, [this](AsyncWebServerRequest* req) {
        handleApiDepartures(req);
    });

    // GET /api/config
    server->on("/api/config", HTTP_GET, [this](AsyncWebServerRequest* req) {
        handleApiConfig(req);
    });

    // POST /api/config  (JSON body)
    server->on("/api/config", HTTP_POST,
        [](AsyncWebServerRequest* req) {},
        nullptr,
        [this](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
            handleApiConfigPost(req, data, len);
        });

    // POST /api/transport  (JSON body: {"mode":"walk"|"bike"|"bus"})
    server->on("/api/transport", HTTP_POST,
        [](AsyncWebServerRequest* req) {},
        nullptr,
        [this](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
            handleApiTransportMode(req, data, len);
        });

    // POST /api/fetch — trigger a background departure refresh
    server->on("/api/fetch", HTTP_POST, [this](AsyncWebServerRequest* req) {
        if (fetchCallback) fetchCallback();
        sendJSON(req, 200, "{\"ok\":true,\"message\":\"Fetch triggered\"}");
    });

    server->onNotFound([](AsyncWebServerRequest* req) {
        sendJSON(req, 404, "{\"error\":\"Not found\"}");
    });
}

// ---------------------------------------------------------------------------
// GET /api/status
// ---------------------------------------------------------------------------

void WebServerManager::handleApiStatus(AsyncWebServerRequest* request) {
    JsonDocument doc;

    // WiFi
    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["connected"] = (WiFi.status() == WL_CONNECTED);
    wifi["rssi"]      = statusCache.rssi;
    wifi["ip"]        = WiFi.localIP().toString();
    wifi["ssid"]      = WiFi.SSID();

    // Device hardware info (safe to read from any task)
    JsonObject device = doc["device"].to<JsonObject>();
    device["chipModel"]     = ESP.getChipModel();
    device["chipRevision"]  = (int)ESP.getChipRevision();
    device["cpuMhz"]        = (int)ESP.getCpuFreqMHz();
    device["flashKb"]       = (int)(ESP.getFlashChipSize() / 1024);
    device["heapFreeKb"]    = (int)(ESP.getFreeHeap() / 1024);
    device["heapTotalKb"]   = (int)(ESP.getHeapSize() / 1024);
    device["uptimeSeconds"] = (unsigned long)(millis() / 1000);

    // Connected peripherals
    JsonObject peripherals = doc["peripherals"].to<JsonObject>();
    peripherals["display"] = statusCache.displayConnected;

    // Fetch / countdown state
    doc["fetchInProgress"] = statusCache.fetchInProgress;

    JsonObject walk = doc["walk"].to<JsonObject>();
    walk["secondsUntilLeave"] = statusCache.walkSecondsUntilLeave;
    walk["direction"]         = statusCache.walkDir;

    JsonObject bike = doc["bike"].to<JsonObject>();
    bike["secondsUntilLeave"] = statusCache.bikeSecondsUntilLeave;
    bike["direction"]         = statusCache.bikeDir;

    doc["mode"] = ConfigManager::getModeName(configManager->getConfig().activeMode);

    String body;
    serializeJson(doc, body);
    sendJSON(request, 200, body);
}

// ---------------------------------------------------------------------------
// GET /api/departures
// ---------------------------------------------------------------------------

void WebServerManager::handleApiDepartures(AsyncWebServerRequest* request) {
    JsonDocument doc;
    doc["fetchInProgress"] = statusCache.fetchInProgress;

    JsonArray arr = doc["departures"].to<JsonArray>();

    if (statusCache.walkSecondsUntilLeave > 0) {
        JsonObject d = arr.add<JsonObject>();
        d["mode"]             = "walk";
        d["secondsUntilLeave"] = statusCache.walkSecondsUntilLeave;
        d["direction"]        = statusCache.walkDir;
    }
    if (statusCache.bikeSecondsUntilLeave > 0) {
        JsonObject d = arr.add<JsonObject>();
        d["mode"]             = "bike";
        d["secondsUntilLeave"] = statusCache.bikeSecondsUntilLeave;
        d["direction"]        = statusCache.bikeDir;
    }

    String body;
    serializeJson(doc, body);
    sendJSON(request, 200, body);
}

// ---------------------------------------------------------------------------
// GET /api/config
// ---------------------------------------------------------------------------

void WebServerManager::handleApiConfig(AsyncWebServerRequest* request) {
    const Config& cfg = configManager->getConfig();
    JsonDocument doc;

    doc["stationCode"]        = cfg.stationCode;
    doc["walkTime"]           = cfg.walkTime;
    doc["bikeTime"]           = cfg.bikeTime;
    doc["busTime"]            = cfg.busTime;
    doc["bufferTime"]         = cfg.bufferTime;
    doc["activeMode"]         = ConfigManager::getModeName(cfg.activeMode);
    doc["audioAlertsEnabled"] = cfg.audioAlertsEnabled;
    doc["ledAlertsEnabled"]   = cfg.ledAlertsEnabled;
    doc["alertAtLeaveTime"]   = cfg.alertAtLeaveTime;
    doc["alertFiveMinBefore"] = cfg.alertFiveMinBefore;
    // Expose SSID so the UI can show it; mask secrets
    doc["wifiSsid"]     = cfg.wifiSsid;
    doc["nsApiKey"]     = strlen(cfg.nsApiKey) > 0 ? "***" : "";
    doc["wifiPassword"] = strlen(cfg.wifiPassword) > 0 ? "***" : "";

    String body;
    serializeJson(doc, body);
    sendJSON(request, 200, body);
}

// ---------------------------------------------------------------------------
// POST /api/config
// ---------------------------------------------------------------------------

void WebServerManager::handleApiConfigPost(AsyncWebServerRequest* request,
                                            uint8_t* data, size_t len) {
    JsonDocument doc;
    if (deserializeJson(doc, data, len)) {
        sendJSON(request, 400, "{\"ok\":false,\"error\":\"Invalid JSON\"}");
        return;
    }

    // Factory reset shortcut
    if (doc["factoryReset"].is<bool>() && doc["factoryReset"].as<bool>()) {
        sendJSON(request, 200, "{\"ok\":true,\"message\":\"Factory reset. Rebooting...\"}");
        delay(500);
        configManager->factoryReset();
        ESP.restart();
        return;
    }

    bool changed = false;

    if (doc["stationCode"].is<const char*>()) {
        String v = doc["stationCode"].as<String>();
        v.trim();
        if (v.length() > 0 && v.length() <= 4) {
            configManager->setStationCode(v);
            changed = true;
        }
    }
    if (doc["walkTime"].is<int>()) {
        configManager->setTravelTime(WALK, doc["walkTime"].as<int>());
        changed = true;
    }
    if (doc["bikeTime"].is<int>()) {
        configManager->setTravelTime(BIKE, doc["bikeTime"].as<int>());
        changed = true;
    }
    if (doc["busTime"].is<int>()) {
        configManager->setTravelTime(BUS, doc["busTime"].as<int>());
        changed = true;
    }
    if (doc["bufferTime"].is<int>()) {
        configManager->setBufferTime(doc["bufferTime"].as<int>());
        changed = true;
    }
    if (doc["nsApiKey"].is<const char*>()) {
        String v = doc["nsApiKey"].as<String>();
        if (v.length() > 0) {
            configManager->setNsApiKey(v);
            nsApiClient->setApiKey(v);
            changed = true;
        }
    }
    if (doc["wifiSsid"].is<const char*>()) {
        String ssid = doc["wifiSsid"].as<String>();
        ssid.trim();
        if (ssid.length() > 0) {
            String pass = doc["wifiPassword"].is<const char*>()
                          ? doc["wifiPassword"].as<String>() : String("");
            configManager->setWiFiCredentials(ssid, pass);
            changed = true;
        }
    }
    if (doc["audioAlertsEnabled"].is<bool>()) {
        configManager->setAudioAlerts(doc["audioAlertsEnabled"].as<bool>());
        changed = true;
    }
    if (doc["ledAlertsEnabled"].is<bool>()) {
        configManager->setLedAlerts(doc["ledAlertsEnabled"].as<bool>());
        changed = true;
    }
    if (doc["alertAtLeaveTime"].is<bool>()) {
        configManager->getConfig().alertAtLeaveTime = doc["alertAtLeaveTime"].as<bool>();
        changed = true;
    }
    if (doc["alertFiveMinBefore"].is<bool>()) {
        configManager->getConfig().alertFiveMinBefore = doc["alertFiveMinBefore"].as<bool>();
        changed = true;
    }

    if (changed) configManager->save();

    sendJSON(request, 200, "{\"ok\":true}");
}

// ---------------------------------------------------------------------------
// POST /api/transport
// ---------------------------------------------------------------------------

void WebServerManager::handleApiTransportMode(AsyncWebServerRequest* request,
                                               uint8_t* data, size_t len) {
    JsonDocument doc;
    if (deserializeJson(doc, data, len) || !doc["mode"].is<const char*>()) {
        sendJSON(request, 400, "{\"ok\":false,\"error\":\"Expected {\\\"mode\\\":\\\"walk|bike|bus\\\"}\"}");
        return;
    }

    String mode = doc["mode"].as<String>();

    TransportMode newMode;
    if (mode.equalsIgnoreCase("walk"))      newMode = WALK;
    else if (mode.equalsIgnoreCase("bike")) newMode = BIKE;
    else if (mode.equalsIgnoreCase("bus"))  newMode = BUS;
    else {
        sendJSON(request, 400, "{\"ok\":false,\"error\":\"Invalid mode — use walk, bike, or bus\"}");
        return;
    }

    configManager->setActiveMode(newMode);
    configManager->save();

    JsonDocument resp;
    resp["ok"]   = true;
    resp["mode"] = ConfigManager::getModeName(newMode);
    String body;
    serializeJson(resp, body);
    sendJSON(request, 200, body);
}
