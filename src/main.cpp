// Odczyt licznika energii (AMI, np. MA309M) przez HAN - wireless M-Bus T1 868.95 MHz
// ESP8266 NodeMCU V3 + CC1101. Wyniki: port szeregowy 115200, strona http://licznik-han.local/ , JSON /json
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClientSecureBearSSL.h>
#include <LittleFS.h>
#include <sys/time.h>
#include <time.h>

#include "cc1101.h"
#include "config.h"
#include "index_html.h"
#include "wmbus.h"

CC1101 radio;
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

// Ustawienia zapisywane w EEPROM (emulowany we flash). Poczatek zgodny z wersja HAN1 (klucz, ID).
struct Settings {
  uint32_t magic;
  char key[33];
  char id[17];
  char pass[33];   // haslo administratora
  float price[4];  // zl/kWh brutto dla T1..T4 (0 = jak T1)
  float fee;       // oplaty stale, zl/mies.
  char cloudId[41];   // X-Device-ID (puste = wysylanie wylaczone)
  char cloudKey[65];  // X-Device-Key
  char cloudUrl[65];  // adres chmury z rejestracji (puste = CLOUD_URL)
  uint16_t cloudInterval;  // s, 0 = CLOUD_INTERVAL_S
  char appUser[41];   // login i haslo do aplikacji Android z rejestracji (tylko do wyswietlenia)
  char appPass[41];
};
constexpr uint32_t SETTINGS_MAGIC_V1 = 0x48414E31;  // "HAN1"
constexpr uint32_t SETTINGS_MAGIC_V2 = 0x48414E32;  // "HAN2" - bez pol chmury
constexpr uint32_t SETTINGS_MAGIC_V3 = 0x48414E33;  // "HAN3" - bez adresu i interwalu chmury
constexpr uint32_t SETTINGS_MAGIC_V4 = 0x48414E34;  // "HAN4" - bez danych logowania do aplikacji
constexpr uint32_t SETTINGS_MAGIC = 0x48414E35;     // "HAN5"

Settings cfg;
uint8_t aesKey[16];
bool haveKey = false;
String keyHex, meterIdFilter;
bool radioOk = false;

MeterData meter;            // ostatnie poprawnie odczytane dane z wybranego licznika
uint32_t meterUpdatedMs = 0;
int meterRssi = 0;
uint32_t framesTotal = 0, framesCrcErr = 0;

struct Device {
  String id, mfr, lastResult, lastHex;
  uint32_t count = 0, lastMs = 0;
  int rssi = 0;
};
constexpr int MAX_DEVICES = 10;
Device devices[MAX_DEVICES];

// ---------------- Zegar ----------------
// Czas z NTP, a dopoki go nie ma - ustawiany z zegara licznika. 0 = czas nieznany.
uint32_t nowEpoch() {
  time_t t = time(nullptr);
  return t > 1700000000 ? (uint32_t)t : 0;
}

void syncClockFromMeter(const MeterData &d) {
  if (nowEpoch()) return;
  tm t = {};
  if (sscanf(d.dateTime.c_str(), "%d-%d-%d %d:%d:%d", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min,
             &t.tm_sec) < 5 || t.tm_year < 2024)
    return;
  t.tm_year -= 1900;
  t.tm_mon -= 1;
  t.tm_isdst = -1;
  timeval tv = {mktime(&t), 0};
  settimeofday(&tv, nullptr);
  Serial.printf("Zegar ustawiony z licznika: %s\n", d.dateTime.c_str());
}

// ---------------- Historia mocy ----------------
// Srednia z HIST_STEP_S sekund, 720 probek = 6 h (ok. 6 kB RAM). Zapisywana do flash co HIST_SAVE_MS,
// wiec wykres przetrwa restart i aktualizacje.
struct Sample {
  uint32_t t;          // czas unix
  uint16_t imp, exp;   // W, HIST_NONE = brak danych
};
constexpr int HIST_N = 720;
constexpr uint32_t HIST_STEP_S = 30;
constexpr uint32_t HIST_SAVE_MS = 15 * 60 * 1000UL;
constexpr uint16_t HIST_NONE = 0xFFFF;
const char *HIST_FILE = "/hist.bin";
Sample hist[HIST_N];
int histHead = 0, histCount = 0;
bool histDirty = false;
double accImp = 0, accExp = 0;
int accImpN = 0, accExpN = 0;
uint32_t accStart = 0;
bool accActive = false;
bool fsOk = false;

uint16_t histValue(double sum, int n) {
  if (!n) return HIST_NONE;
  double v = sum / n;
  return v <= 0 ? 0 : v >= HIST_NONE - 1 ? HIST_NONE - 1 : (uint16_t)lround(v);
}

void addHistory(const MeterData &d) {
  uint32_t up = millis() / 1000, now = nowEpoch();
  if (!now) return;
  if (!isnan(d.powerImportW)) { accImp += d.powerImportW; accImpN++; }
  if (!isnan(d.powerExportW)) { accExp += d.powerExportW; accExpN++; }
  if (!accImpN && !accExpN) return;
  if (!accActive) {
    accActive = true;
    accStart = up;
  }
  if (up - accStart < HIST_STEP_S) return;
  hist[histHead] = {now, histValue(accImp, accImpN), histValue(accExp, accExpN)};
  histHead = (histHead + 1) % HIST_N;
  if (histCount < HIST_N) histCount++;
  histDirty = true;
  accImp = accExp = 0;
  accImpN = accExpN = 0;
  accActive = false;
}

const Sample &histAt(int i) { return hist[(histHead - histCount + i + HIST_N) % HIST_N]; }

void saveHistory() {
  if (!fsOk || !histDirty) return;
  File f = LittleFS.open(HIST_FILE, "w");
  if (!f) return;
  for (int i = 0; i < histCount; i++) f.write((const uint8_t *)&histAt(i), sizeof(Sample));
  histDirty = false;
}

void loadHistory() {
  File f = LittleFS.open(HIST_FILE, "r");
  if (!f) return;
  size_t n = f.size() / sizeof(Sample);
  if (n > HIST_N) f.seek((n - HIST_N) * sizeof(Sample));
  Sample s;
  while (f.read((uint8_t *)&s, sizeof s) == sizeof s) {
    if (!s.t) continue;
    hist[histHead] = s;
    histHead = (histHead + 1) % HIST_N;
    if (histCount < HIST_N) histCount++;
  }
}

// ---------------- Dobowe stany licznika ----------------
// Pierwszy poprawny odczyt kazdej doby jest dopisywany do pliku w LittleFS (28 B/dobe), plik jest
// posortowany po dacie. Stany wpisane recznie (np. z rachunku) maja ustawiony bit DAY_MANUAL w dacie.
// Zuzycie dni i miesiecy liczy strona WWW.
struct DayRec {
  uint32_t date;    // RRRRMMDD (+ DAY_MANUAL)
  uint32_t imp[5];  // Wh: suma, T1..T4 (DAY_NONE = brak)
  uint32_t exp;     // Wh
};
const char *DAYS_FILE = "/days.bin";
constexpr uint32_t DAY_NONE = 0xFFFFFFFF;
constexpr uint32_t DAY_MANUAL = 0x80000000;
constexpr size_t DAYS_MAX = 1200, DAYS_KEEP = 1100;  // ok. 3 lata
uint32_t lastDay = 0;
size_t dayCount = 0;

// Stany na poczatek biezacej doby i miesiaca (do /json i Home Assistant)
DayRec dayStart, monthStart;
bool hasDayStart = false, hasMonthStart = false;
uint32_t startsFor = 0;

uint32_t toWh(double kwh) { return isnan(kwh) || kwh < 0 ? DAY_NONE : (uint32_t)llround(kwh * 1000); }
uint32_t dayOf(const DayRec &r) { return r.date & ~DAY_MANUAL; }

// Data biezacej doby z zegara licznika, a gdy go brak - z zegara ESP (0 = nieznana)
uint32_t currentDate(const MeterData &d) {
  int y, m, dd;
  if (sscanf(d.dateTime.c_str(), "%d-%d-%d", &y, &m, &dd) == 3 && y >= 2024 && y < 2100 && m >= 1 && m <= 12 &&
      dd >= 1 && dd <= 31)
    return y * 10000 + m * 100 + dd;
  time_t t = nowEpoch();
  if (!t) return 0;
  tm lt;
  localtime_r(&t, &lt);
  return (lt.tm_year + 1900) * 10000 + (lt.tm_mon + 1) * 100 + lt.tm_mday;
}

void loadDays() {
  File f = LittleFS.open(DAYS_FILE, "r");
  if (!f) return;
  dayCount = f.size() / sizeof(DayRec);
  if (dayCount) {
    DayRec r;
    f.seek((dayCount - 1) * sizeof(DayRec));
    if (f.read((uint8_t *)&r, sizeof r) == sizeof r) lastDay = dayOf(r);
  }
}

void ensureStarts(uint32_t today) {
  if (!today || today == startsFor) return;
  startsFor = today;
  hasDayStart = hasMonthStart = false;
  File f = LittleFS.open(DAYS_FILE, "r");
  DayRec r;
  while (f && f.read((uint8_t *)&r, sizeof r) == sizeof r) {
    uint32_t d = dayOf(r);
    if (d / 100 == today / 100 && !hasMonthStart) {
      monthStart = r;
      hasMonthStart = true;
    }
    if (d == today) {
      dayStart = r;
      hasDayStart = true;
    }
  }
}

// Zostawia keep najnowszych rekordow pliku
void trimFile(const char *path, size_t recSize, size_t &count, size_t keep) {
  File in = LittleFS.open(path, "r"), out = LittleFS.open("/trim.tmp", "w");
  if (!in || !out) return;
  in.seek((count - keep) * recSize);
  uint8_t buf[256];
  int n;
  while ((n = in.read(buf, sizeof buf)) > 0) out.write(buf, n);
  in.close();
  out.close();
  LittleFS.remove(path);
  LittleFS.rename("/trim.tmp", path);
  count = keep;
}

void recordDay(const MeterData &d) {
  if (!fsOk || isnan(d.importKWh[0])) return;
  uint32_t date = currentDate(d);
  if (!date || date <= lastDay) return;
  DayRec r = {date, {}, toWh(d.exportKWh[0])};
  for (int t = 0; t < 5; t++) r.imp[t] = toWh(d.importKWh[t]);
  File f = LittleFS.open(DAYS_FILE, "a");
  if (!f) return;
  f.write((const uint8_t *)&r, sizeof r);
  f.close();
  lastDay = date;
  startsFor = 0;
  Serial.printf("Zapisano stan dobowy %lu: %.3f kWh\n", (unsigned long)date, d.importKWh[0]);
  if (++dayCount > DAYS_MAX) trimFile(DAYS_FILE, sizeof(DayRec), dayCount, DAYS_KEEP);
}

// Wstawia/zastepuje stan z dnia nr.date albo (del) usuwa go; zachowuje kolejnosc dat
bool putDay(const DayRec &nr, bool del) {
  File in = LittleFS.open(DAYS_FILE, "r"), out = LittleFS.open("/days.tmp", "w");
  if (!out) return false;
  uint32_t nd = dayOf(nr), last = 0;
  size_t n = 0;
  bool done = false;
  DayRec r;
  auto put = [&](const DayRec &x) {
    out.write((const uint8_t *)&x, sizeof x);
    last = dayOf(x);
    n++;
  };
  while (in && in.read((uint8_t *)&r, sizeof r) == sizeof r) {
    uint32_t d = dayOf(r);
    if (!done && d >= nd) {
      done = true;
      if (!del) put(nr);
      if (d == nd) continue;
    }
    put(r);
  }
  if (!done && !del) put(nr);
  in.close();
  out.close();
  LittleFS.remove(DAYS_FILE);
  LittleFS.rename("/days.tmp", DAYS_FILE);
  dayCount = n;
  lastDay = last;
  startsFor = 0;
  return true;
}

// ---------------- Odchylenia napiecia ----------------
// Okresy, gdy napiecie fazy jest poza norma 207-253 V (230 V +/-10%, PN-EN 50160). Zakonczone zdarzenia
// sa dopisywane do pliku w LittleFS (16 B/zdarzenie); eksport do XML/PDF robi strona WWW.
struct VoltEv {
  uint32_t start, end;    // czas unix: pierwszy odczyt poza norma, powrot do normy (lub ostatni odczyt)
  uint16_t minDv, maxDv;  // 0,1 V
  uint8_t phase;          // 1..3
  uint8_t high;           // 1 = przepiecie, 0 = zanizenie
  uint16_t reserved;
};
const char *VOLT_FILE = "/volt.bin";
constexpr float VOLT_LOW = 207, VOLT_HIGH = 253;
constexpr uint32_t VOLT_GAP_S = 600;  // brak odczytow dluzej = koniec zdarzenia
constexpr size_t VOLT_MAX = 2000, VOLT_KEEP = 1800;
VoltEv voltOpen[3];
bool voltIsOpen[3] = {};
size_t voltCount = 0;

void loadVolt() {
  File f = LittleFS.open(VOLT_FILE, "r");
  if (f) voltCount = f.size() / sizeof(VoltEv);
}

void closeVolt(int p) {
  if (!voltIsOpen[p]) return;
  voltIsOpen[p] = false;
  const VoltEv &e = voltOpen[p];
  Serial.printf("Odchylenie L%u (%s) %.1f-%.1f V, %lu s\n", e.phase, e.high ? "przepiecie" : "zanizenie",
                e.minDv / 10.0, e.maxDv / 10.0, (unsigned long)(e.end - e.start));
  if (!fsOk) return;
  File f = LittleFS.open(VOLT_FILE, "a");
  if (!f) return;
  f.write((const uint8_t *)&e, sizeof e);
  f.close();
  if (++voltCount > VOLT_MAX) trimFile(VOLT_FILE, sizeof(VoltEv), voltCount, VOLT_KEEP);
}

// Zamyka trwajace zdarzenia (przed restartem)
void closeAllVolt() {
  uint32_t now = nowEpoch();
  for (int p = 0; p < 3; p++) {
    if (voltIsOpen[p] && now) voltOpen[p].end = now;
    closeVolt(p);
  }
}

void trackVoltage(const MeterData &d) {
  uint32_t now = nowEpoch();
  if (!now) return;
  for (int p = 0; p < 3; p++) {
    VoltEv &e = voltOpen[p];
    if (voltIsOpen[p] && now - e.end > VOLT_GAP_S) closeVolt(p);
    double u = d.voltage[p];
    if (isnan(u) || u <= 0) continue;
    bool low = u < VOLT_LOW, high = u > VOLT_HIGH;
    uint16_t dv = (uint16_t)lround(u * 10);
    if (voltIsOpen[p] && (!(low || high) || e.high != high)) {
      e.end = now;
      closeVolt(p);
    }
    if (!low && !high) continue;
    if (!voltIsOpen[p]) {
      e = {now, now, dv, dv, (uint8_t)(p + 1), high, 0};
      voltIsOpen[p] = true;
    } else {
      e.end = now;
      e.minDv = min(e.minDv, dv);
      e.maxDv = max(e.maxDv, dv);
    }
  }
}

// ---------------- Ustawienia ----------------

bool parseKey(const String &hex, uint8_t *out) {
  if (hex.length() != 32) return false;
  for (int i = 0; i < 16; i++) {
    char b[3] = {hex[2 * i], hex[2 * i + 1], 0};
    char *end;
    out[i] = strtoul(b, &end, 16);
    if (*end) return false;
  }
  return true;
}

void applySettings() {
  keyHex = cfg.key;
  keyHex.trim();
  keyHex.replace(" ", "");
  haveKey = parseKey(keyHex, aesKey);
  meterIdFilter = cfg.id;
  ArduinoOTA.setPassword(cfg.pass);
  httpUpdater.updateCredentials(ADMIN_USER, cfg.pass);
}

void saveSettings() {
  cfg.magic = SETTINGS_MAGIC;
  EEPROM.put(0, cfg);
  EEPROM.commit();
  applySettings();
}

void loadSettings() {
  EEPROM.get(0, cfg);
  // starsze wersje: nowe pola sa na koncu, zerowane od pierwszego brakujacego
  if (cfg.magic == SETTINGS_MAGIC_V2 || cfg.magic == SETTINGS_MAGIC_V3) {
    cfg.cloudId[0] = cfg.cloudKey[0] = cfg.cloudUrl[0] = 0;
    cfg.cloudInterval = 0;
    cfg.magic = SETTINGS_MAGIC_V4;
  }
  if (cfg.magic == SETTINGS_MAGIC_V4) {
    cfg.appUser[0] = cfg.appPass[0] = 0;
    cfg.magic = SETTINGS_MAGIC;
  }
  if (cfg.magic != SETTINGS_MAGIC) {
    Settings d = {};
    d.magic = SETTINGS_MAGIC;
    if (cfg.magic == SETTINGS_MAGIC_V1) {  // przeniesienie klucza i ID z poprzedniej wersji
      memcpy(d.key, cfg.key, sizeof d.key);
      memcpy(d.id, cfg.id, sizeof d.id);
    } else {
      strlcpy(d.key, DEFAULT_AES_KEY, sizeof d.key);
      strlcpy(d.id, DEFAULT_METER_ID, sizeof d.id);
    }
    strlcpy(d.pass, ADMIN_PASSWORD, sizeof d.pass);
    cfg = d;
  }
  cfg.key[32] = cfg.id[16] = cfg.pass[32] = cfg.cloudId[40] = cfg.cloudKey[64] = cfg.cloudUrl[64] = cfg.appUser[40] = cfg.appPass[40] = 0;
  for (float &p : cfg.price)
    if (isnan(p) || p < 0) p = 0;
  if (isnan(cfg.fee) || cfg.fee < 0) cfg.fee = 0;
  applySettings();
}

// ---------------- Odbior ramek ----------------

String hexDump(const uint8_t *d, int n) {
  String s;
  char b[3];
  for (int i = 0; i < n; i++) {
    snprintf(b, sizeof(b), "%02X", d[i]);
    s += b;
  }
  return s;
}

Device &deviceSlot(const String &id) {
  int oldest = 0;
  for (int i = 0; i < MAX_DEVICES; i++) {
    if (devices[i].id == id) return devices[i];
    if (devices[i].lastMs < devices[oldest].lastMs) oldest = i;
  }
  devices[oldest] = Device();
  devices[oldest].id = id;
  return devices[oldest];
}

void handleFrame(const uint8_t *f, int len) {
  framesTotal++;
  MeterData d;
  FrameResult r = parseFrame(f, len, aesKey, haveKey, d);
  if (r == FrameResult::CrcError) {
    framesCrcErr++;
    return;
  }
  Device &dev = deviceSlot(d.linkId);
  dev.mfr = d.manufacturer;
  dev.count++;
  dev.lastMs = millis();
  dev.rssi = radio.lastRssi();
  dev.lastResult = frameResultText(r);
  dev.lastHex = hexDump(f, len);

  Serial.printf("[%lu] ID %s (%s) RSSI %d dBm, %d B: %s\n", millis() / 1000, d.linkId.c_str(),
                d.manufacturer.c_str(), dev.rssi, len, frameResultText(r));

  bool selected = meterIdFilter.isEmpty() || meterIdFilter == d.linkId || meterIdFilter == d.serial;
  if (r == FrameResult::Ok && selected) {
    meter = d;
    meterUpdatedMs = millis();
    meterRssi = dev.rssi;
    syncClockFromMeter(d);
    addHistory(d);
    recordDay(d);
    trackVoltage(d);
    Serial.print(d.records);
  }
}

// ---------------- Chmura ----------------
// Po rejestracji (/cloudregister) co send_interval_sec (domyslnie CLOUD_INTERVAL_S) wysyla najnowszy odczyt (jesli jest swiezy). Polaczenie HTTPS blokuje petle na 1-3 s,
// wiec moze przepasc pojedyncza ramka radiowa - przy odczycie co kilka sekund nie ma to znaczenia.
uint32_t cloudSent = 0, cloudLastMs = 0, cloudLastSentUpdate = 0;
int cloudLastCode = 0;
uint32_t cloudFails = 0;  // kolejne nieudane wysylki - wydluzaja przerwe przed ponowieniem
uint32_t cloudWaitS();
String cloudLastError;
String cloudLastPayload;  // ostatnio wysylany JSON (diagnostyka, bez klucza urzadzenia)

String num(double v, int dec);
String jsonEscape(const String &s);

// Format ustalony z mini-cloud: ts (Unix UTC w sekundach), power_w, import_kwh, export_kwh, voltage_l1..l3 jako liczby.
// Dalej pozostale odczytane pola (nazwy jak w /json). Pola bez wartosci sa pomijane (bez null i bez 0).
// ID i klucz urzadzenia ida w naglowkach.
String cloudPayload() {
  String j = "{";
  auto add = [&](const String &k, double v, int dec) {
    if (isnan(v)) return;
    if (j.length() > 1) j += ',';
    j += '"' + k + "\":" + String(v, dec);
  };
  auto addStr = [&](const char *k, const String &v) {
    if (v.isEmpty()) return;
    if (j.length() > 1) j += ',';
    j += String('"') + k + "\":\"" + jsonEscape(v) + '"';
  };
  if (uint32_t t = nowEpoch()) j += "\"ts\":" + String(t);  // Unix UTC, sekundy
  double pw = meter.powerImportW;  // moc netto: pobor minus oddawanie
  if (!isnan(meter.powerExportW)) pw = (isnan(pw) ? 0 : pw) - meter.powerExportW;
  add("power_w", pw, 0);
  add("import_kwh", meter.importKWh[0], 3);
  add("export_kwh", meter.exportKWh[0], 3);
  for (int p = 0; p < 3; p++) add("voltage_l" + String(p + 1), meter.voltage[p], 1);
  add("power_import_w", meter.powerImportW, 0);
  add("power_export_w", meter.powerExportW, 0);
  add("power_max_w", meter.maxPowerW, 0);
  add("power_limit_w", meter.powerLimitW, 0);
  for (int t = 1; t <= 4; t++) {
    add("import_t" + String(t) + "_kwh", meter.importKWh[t], 3);
    add("export_t" + String(t) + "_kwh", meter.exportKWh[t], 3);
  }
  addStr("meter_id", meter.linkId);
  addStr("manufacturer", meter.manufacturer);
  addStr("serial", meter.serial);
  addStr("meter_time", meter.dateTime);
  addStr("message", meter.message);
  add("rssi", meterRssi, 0);
  j += "}";
  return j;
}

String cloudBase() { return cfg.cloudUrl[0] ? String(cfg.cloudUrl) : String(CLOUD_URL); }
uint32_t cloudIntervalS() { return cfg.cloudInterval ? cfg.cloudInterval : CLOUD_INTERVAL_S; }

// POST JSON na adres chmury + path; zwraca kod HTTP (<0 = blad polaczenia), odpowiedz w *resp
int cloudPost(const String &path, const String &body, bool deviceAuth, String *resp) {
  String url = cloudBase();
  if (url.endsWith("/")) url.remove(url.length() - 1);
  url += path;
  BearSSL::WiFiClientSecure tls;
  WiFiClient plain;
  tls.setInsecure();  // brak weryfikacji certyfikatu (ESP8266 nie ma magazynu CA)
  tls.setBufferSizes(4096, 512);
  tls.setTimeout(5000);
  HTTPClient http;
  if (!http.begin(url.startsWith("https://") ? (WiFiClient &)tls : plain, url)) return HTTPC_ERROR_CONNECTION_FAILED;
  http.setTimeout(5000);
  http.addHeader("Content-Type", "application/json");
  if (deviceAuth) {
    http.addHeader("X-Device-ID", cfg.cloudId);
    http.addHeader("X-Device-Key", cfg.cloudKey);
  }
  int code = http.POST(body);
  if (resp && code > 0) *resp = http.getString();
  http.end();
  return code;
}

// Wartosc pola z plaskiego JSON-a (tekst albo liczba), pusty tekst gdy brak
String jsonStr(const String &j, const char *key) {
  int i = j.indexOf(String('"') + key + '"');
  if (i < 0) return "";
  i = j.indexOf(':', i + strlen(key) + 2);
  if (i < 0) return "";
  while (++i < (int)j.length() && isspace(j[i])) {}
  String v;
  if (i < (int)j.length() && j[i] == '"') {
    for (i++; i < (int)j.length() && j[i] != '"'; i++) {
      if (j[i] == '\\' && i + 1 < (int)j.length()) i++;
      v += j[i];
    }
  } else {
    for (; i < (int)j.length() && j[i] != ',' && j[i] != '}' && !isspace(j[i]); i++) v += j[i];
    if (v == "null") v = "";
  }
  return v;
}

String htmlEscape(const String &s) {
  String o;
  for (char c : s) {
    if (c == '<') o += "&lt;";
    else if (c == '>') o += "&gt;";
    else if (c == '&') o += "&amp;";
    else o += c;
  }
  return o;
}

void cloudSend() {
  if (!cfg.cloudId[0] || !cfg.cloudKey[0] || WiFi.status() != WL_CONNECTED) return;
  if (!meterUpdatedMs || meterUpdatedMs == cloudLastSentUpdate || millis() - meterUpdatedMs > 5 * 60000UL) return;
  String resp;
  cloudLastPayload = cloudPayload();
  cloudLastCode = cloudPost("/api/device/measurements", cloudLastPayload, true, &resp);
  cloudLastMs = millis();
  if (cloudLastCode >= 200 && cloudLastCode < 300) {
    cloudSent++;
    cloudLastSentUpdate = meterUpdatedMs;
    cloudLastError = "";
    if (cloudFails) Serial.printf("Chmura: wysylka znow dziala (kod %d) po %lu bledach\n", cloudLastCode, (unsigned long)cloudFails);
    cloudFails = 0;
  } else {
    // pomiar zostaje niewyslany - przy ponowieniu idzie najnowszy odczyt
    cloudFails++;
    cloudLastError = cloudLastCode > 0 ? resp.substring(0, 200) : HTTPClient::errorToString(cloudLastCode);
    Serial.printf("Chmura: blad %d (proba %lu, ponowienie za %lu s) %s\n", cloudLastCode, (unsigned long)cloudFails,
                  (unsigned long)cloudWaitS(), cloudLastError.c_str());
  }
}

// Przerwa do nastepnej wysylki: normalnie interwal, po bledach 15, 30, 60 ... s, najwyzej 10 min
uint32_t cloudWaitS() {
  if (!cloudFails) return cloudIntervalS();
  return min<uint32_t>(15UL << min<uint32_t>(cloudFails - 1, 6), 600);
}

// ---------------- WWW ----------------

String num(double v, int dec) { return isnan(v) ? "null" : String(v, dec); }

String jsonEscape(const String &s) {
  String o;
  for (char c : s) {
    if (c == '"' || c == '\\') o += '\\';
    if (c == '\n') { o += "\\n"; continue; }
    o += c;
  }
  return o;
}

// Przyrost licznika od zapisanego stanu (kWh), NAN gdy brak danych lub licznik sie cofnal
double since(double nowKWh, uint32_t startWh) {
  if (isnan(nowKWh) || startWh == DAY_NONE) return NAN;
  double v = nowKWh - startWh / 1000.0;
  return v < -0.0005 ? NAN : max(v, 0.0);
}

// Koszt energii pobranej od stanu s wg cen taryf (bez oplat stalych)
double energyCost(const DayRec &s) {
  if (cfg.price[0] <= 0) return NAN;
  double c = 0;
  bool tariffs = false;
  for (int t = 1; t <= 4; t++) {
    double v = since(meter.importKWh[t], s.imp[t]);
    if (isnan(v)) continue;
    c += v * (cfg.price[t - 1] > 0 ? cfg.price[t - 1] : cfg.price[0]);
    tariffs = true;
  }
  return tariffs ? c : since(meter.importKWh[0], s.imp[0]) * cfg.price[0];
}

void handleJson() {
  String j = "{";
  j += "\"radio_ok\":" + String(radioOk ? "true" : "false");
  j += ",\"key_set\":" + String(haveKey ? "true" : "false");
  j += ",\"meter_filter\":\"" + jsonEscape(meterIdFilter) + "\"";
  j += ",\"frames\":" + String(framesTotal) + ",\"crc_errors\":" + String(framesCrcErr);
  j += ",\"sync\":" + String(radio.syncs()) + ",\"sync_bad\":" + String(radio.badSyncs()) +
       ",\"sync_c1\":" + String(radio.c1Syncs());
  j += ",\"noise_dbm\":" + (radioOk ? String(radio.currentRssi()) : String("null"));
  j += ",\"uptime_s\":" + String(millis() / 1000);
  j += ",\"time\":" + String(nowEpoch());
  j += ",\"days_stored\":" + String(fsOk ? (int)dayCount : -1);
  j += ",\"price\":[";
  for (int t = 0; t < 4; t++) {
    if (t) j += ',';
    j += num(cfg.price[t] > 0 ? cfg.price[t] : NAN, 4);
  }
  j += "],\"fee_month\":" + num(cfg.fee, 2);
  j += ",\"cloud\":{\"id\":\"" + jsonEscape(cfg.cloudId) + "\",\"app_user\":\"" + jsonEscape(cfg.appUser) + "\",\"registered\":" +
       String(cfg.cloudId[0] && cfg.cloudKey[0] ? "true" : "false") + ",\"url\":\"" + jsonEscape(cloudBase()) +
       "\",\"interval_s\":" + String(cloudIntervalS()) +
       ",\"error\":\"" + jsonEscape(cloudLastError) + "\",\"fails\":" + String(cloudFails) + ",\"last_payload\":" +
       (cloudLastPayload.length() ? cloudLastPayload : String("null")) + ",\"sent\":" + String(cloudSent) + ",\"last_code\":" + String(cloudLastCode) +
       ",\"last_age_s\":" + (cloudLastMs ? String((millis() - cloudLastMs) / 1000) : String("null")) + "}";
  if (meterUpdatedMs) {
    j += ",\"meter\":{";
    j += "\"age_s\":" + String((millis() - meterUpdatedMs) / 1000);
    j += ",\"rssi\":" + String(meterRssi);
    j += ",\"id\":\"" + meter.linkId + "\",\"manufacturer\":\"" + meter.manufacturer + "\"";
    j += ",\"serial\":\"" + jsonEscape(meter.serial) + "\",\"datetime\":\"" + meter.dateTime + "\"";
    j += ",\"import_kwh\":" + num(meter.importKWh[0], 3);
    j += ",\"export_kwh\":" + num(meter.exportKWh[0], 3);
    for (int t = 1; t <= 4; t++) {
      j += ",\"import_t" + String(t) + "_kwh\":" + num(meter.importKWh[t], 3);
      j += ",\"export_t" + String(t) + "_kwh\":" + num(meter.exportKWh[t], 3);
    }
    j += ",\"power_import_w\":" + num(meter.powerImportW, 0);
    j += ",\"power_export_w\":" + num(meter.powerExportW, 0);
    j += ",\"power_max_w\":" + num(meter.maxPowerW, 0);
    j += ",\"power_limit_w\":" + num(meter.powerLimitW, 0);
    for (int p = 0; p < 3; p++) j += ",\"voltage_l" + String(p + 1) + "\":" + num(meter.voltage[p], 1);
    ensureStarts(currentDate(meter));
    if (hasDayStart) {
      j += ",\"today_import_kwh\":" + num(since(meter.importKWh[0], dayStart.imp[0]), 3);
      j += ",\"today_export_kwh\":" + num(since(meter.exportKWh[0], dayStart.exp), 3);
    }
    if (hasMonthStart) {
      j += ",\"month_start\":" + String(dayOf(monthStart));
      j += ",\"month_import_kwh\":" + num(since(meter.importKWh[0], monthStart.imp[0]), 3);
      j += ",\"month_export_kwh\":" + num(since(meter.exportKWh[0], monthStart.exp), 3);
      j += ",\"month_cost_pln\":" + num(energyCost(monthStart), 2);
    }
    j += ",\"message\":\"" + jsonEscape(meter.message) + "\"";
    j += ",\"records\":\"" + jsonEscape(meter.records) + "\"";
    j += ",\"records_total\":" + String(meter.recTotal) + ",\"records_used\":" + String(meter.recUsed);
    j += ",\"rest_bytes\":" + String(meter.restBytes) + ",\"parse_note\":\"" + jsonEscape(meter.parseNote) + "\"";
    j += "}";
  }
  j += ",\"devices\":[";
  bool first = true;
  for (auto &d : devices) {
    if (d.id.isEmpty()) continue;
    if (!first) j += ",";
    first = false;
    j += "{\"id\":\"" + d.id + "\",\"mfr\":\"" + d.mfr + "\",\"count\":" + String(d.count) +
         ",\"rssi\":" + String(d.rssi) + ",\"age_s\":" + String((millis() - d.lastMs) / 1000) +
         ",\"result\":\"" + d.lastResult + "\",\"hex\":\"" + d.lastHex + "\"}";
  }
  j += "]}";
  server.send(200, "application/json", j);
}

// POST /selftest - test przewodow do CC1101 (ok. 30 ms przerwy w odbiorze)
void handleSelfTest() {
  int gdo0 = -1;
#ifdef PIN_CC1101_GDO0
  gdo0 = PIN_CC1101_GDO0;
#endif
  String j = radio.selfTest(PIN_CC1101_SCK, PIN_CC1101_MOSI, gdo0, radioOk);
  Serial.println("Test polaczen CC1101: " + j);
  server.send(200, "application/json", j);
}

bool checkAuth() {
  if (!cfg.pass[0] || server.authenticate(ADMIN_USER, cfg.pass)) return true;
  server.requestAuthentication(BASIC_AUTH, "Licznik HAN", "Wymagane haslo administratora");
  return false;
}

void sendError(const String &msg) {
  server.send(400, "text/html",
              "<!doctype html><meta charset=utf-8><meta name=viewport content='width=device-width'>"
              "<body style='font:16px system-ui;padding:24px'><p>" + msg + "</p><p><a href='/'>Wróć</a></p>");
}

void redirectHome() {
  server.sendHeader("Location", "/#ustawienia");
  server.send(303);
}

// Liczba z formularza ("1,23" lub "1.23"); pusty tekst = 0, brak pola = def
float argFloat(const String &name, float def) {
  if (!server.hasArg(name)) return def;
  String s = server.arg(name);
  s.trim();
  s.replace(',', '.');
  return s.isEmpty() ? 0 : s.toFloat();
}

// POST /config - zmienia tylko pola obecne w formularzu
void handleConfig() {
  if (!checkAuth()) return;
  if (server.hasArg("key") && !server.arg("key").isEmpty()) {
    String k = server.arg("key");
    k.trim();
    k.replace(" ", "");
    uint8_t tmp[16];
    if (!parseKey(k, tmp)) return sendError("Klucz musi mieć 32 znaki hex.");
    strlcpy(cfg.key, k.c_str(), sizeof cfg.key);
  }
  if (server.hasArg("id")) {
    String id = server.arg("id");
    id.trim();
    strlcpy(cfg.id, id.c_str(), sizeof cfg.id);
  }
  for (int t = 0; t < 4; t++) cfg.price[t] = max(0.0f, argFloat("p" + String(t + 1), cfg.price[t]));
  cfg.fee = max(0.0f, argFloat("fee", cfg.fee));
  if (server.arg("cloud_forget") == "1") cfg.cloudId[0] = cfg.cloudKey[0] = cfg.appUser[0] = cfg.appPass[0] = 0;
  if (server.hasArg("pass") && !server.arg("pass").isEmpty()) {
    String p = server.arg("pass");
    if (p != server.arg("pass2")) return sendError("Hasła nie są takie same.");
    if (p.length() < 4 || p.length() > 32) return sendError("Hasło musi mieć od 4 do 32 znaków.");
    strlcpy(cfg.pass, p.c_str(), sizeof cfg.pass);
  }
  saveSettings();
  redirectHome();
}

// POST /cloudregister - rejestruje ESP w chmurze i zapisuje dane urzadzenia oraz login i haslo do aplikacji
// Android (haslo widoczne tylko po zalogowaniu - /cloudapp).
void handleCloudRegister() {
  if (!checkAuth()) return;
  if (WiFi.status() != WL_CONNECTED) return sendError("Brak połączenia Wi-Fi.");
  String req = "{\"chip_id\":\"" + String(ESP.getChipId(), HEX) + "\",\"mac\":\"" + WiFi.macAddress() + "\"";
  req += ",\"meter_id\":\"" + jsonEscape(meterUpdatedMs ? meter.linkId : meterIdFilter) + "\"";
  req += ",\"meter_serial\":\"" + jsonEscape(meter.serial) + "\",\"hostname\":\"" HOSTNAME "\"}";
  String body;
  int code = cloudPost("/api/device/register", req, false, &body);
  if (code < 200 || code >= 300)
    return sendError("Rejestracja nieudana (kod " + String(code) + "): " +
                     htmlEscape(code > 0 ? body.substring(0, 200) : HTTPClient::errorToString(code)));
  String id = jsonStr(body, "device_id"), key = jsonStr(body, "device_key"), url = jsonStr(body, "cloud_url");
  if (id.isEmpty() || key.isEmpty()) return sendError("Chmura nie zwróciła device_id / device_key.");
  if (id.length() >= sizeof cfg.cloudId || key.length() >= sizeof cfg.cloudKey || url.length() >= sizeof cfg.cloudUrl)
    return sendError("Za długie dane urządzenia z chmury.");
  strlcpy(cfg.cloudId, id.c_str(), sizeof cfg.cloudId);
  strlcpy(cfg.cloudKey, key.c_str(), sizeof cfg.cloudKey);
  if (url.startsWith("https://") || url.startsWith("http://")) strlcpy(cfg.cloudUrl, url.c_str(), sizeof cfg.cloudUrl);
  String login = jsonStr(body, "app_username"), pass = jsonStr(body, "app_password");
  if (login.isEmpty()) login = jsonStr(body, "login");
  if (login.isEmpty()) login = jsonStr(body, "username");
  if (pass.isEmpty()) pass = jsonStr(body, "password");
  strlcpy(cfg.appUser, login.c_str(), sizeof cfg.appUser);
  strlcpy(cfg.appPass, pass.c_str(), sizeof cfg.appPass);
  long iv = jsonStr(body, "send_interval_sec").toInt();
  cfg.cloudInterval = iv >= 10 && iv <= 3600 ? iv : 0;
  saveSettings();
  cloudSent = cloudLastCode = 0;
  cloudLastMs = 0;
  Serial.printf("Chmura: zarejestrowano jako %s\n", cfg.cloudId);
  String h = "<!doctype html><meta charset=utf-8><meta name=viewport content='width=device-width'>"
             "<body style='font:16px system-ui;padding:24px;max-width:520px'><h2>Zarejestrowano w mini-cloud</h2>"
             "<p>Device ID: <b>" + htmlEscape(id) + "</b></p>";
  if (login.length() || pass.length())
    h += "<p>Dane logowania do aplikacji Android (są też w Ustawieniach → Mini-cloud):</p><p>Login: <b>" + htmlEscape(login) + "</b><br>Hasło: <b>" + htmlEscape(pass) + "</b></p>";
  h += "<p><a href='/#ustawienia'>Wróć</a></p>";
  server.send(200, "text/html", h);
}

// /cloudapp - login i haslo do aplikacji Android (tylko dla administratora)
void handleCloudApp() {
  if (!checkAuth()) return;
  server.send(200, "application/json",
              "{\"user\":\"" + jsonEscape(cfg.appUser) + "\",\"pass\":\"" + jsonEscape(cfg.appPass) + "\"}");
}

// POST /reading - reczny stan licznika z dnia (np. z rachunku): date=RRRR-MM-DD, imp, t1, t2, exp [kWh]; del=1 usuwa
void handleReading() {
  if (!checkAuth()) return;
  if (!fsOk) return sendError("Pamięć flash niedostępna.");
  int y, m, d;
  if (sscanf(server.arg("date").c_str(), "%d-%d-%d", &y, &m, &d) != 3 || y < 2000 || y > 2099 || m < 1 || m > 12 ||
      d < 1 || d > 31)
    return sendError("Niepoprawna data.");
  DayRec r = {(uint32_t)(y * 10000 + m * 100 + d) | DAY_MANUAL, {DAY_NONE, DAY_NONE, DAY_NONE, DAY_NONE, DAY_NONE},
              DAY_NONE};
  bool del = server.arg("del") == "1";
  if (!del) {
    uint32_t today = currentDate(meter);
    if (today && dayOf(r) > today) return sendError("Data z przyszłości.");
    auto wh = [](float v) { return v > 0 ? toWh(v) : DAY_NONE; };
    r.imp[0] = wh(argFloat("imp", 0));
    r.imp[1] = wh(argFloat("t1", 0));
    r.imp[2] = wh(argFloat("t2", 0));
    r.exp = wh(argFloat("exp", 0));
    if (r.imp[0] == DAY_NONE) return sendError("Podaj stan energii pobranej (kWh).");
  }
  if (!putDay(r, del)) return sendError("Błąd zapisu.");
  redirectHome();
}

// /history?since=<t> - probki nowsze niz podany czas unix, wysylane kawalkami
void handleHistory() {
  uint32_t sinceT = server.arg("since").toInt();
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json", "");
  String buf = "{\"now\":" + String(nowEpoch()) + ",\"step\":" + String(HIST_STEP_S) + ",\"s\":[";
  bool first = true;
  for (int i = 0; i < histCount; i++) {
    const Sample &s = histAt(i);
    if (s.t <= sinceT) continue;
    if (!first) buf += ',';
    first = false;
    buf += String('[') + String(s.t) + ',' + (s.imp == HIST_NONE ? String("null") : String(s.imp)) + ',' +
           (s.exp == HIST_NONE ? String("null") : String(s.exp)) + ']';
    if (buf.length() > 1200) {
      server.sendContent(buf);
      buf = "";
    }
  }
  buf += "]}";
  server.sendContent(buf);
  server.sendContent("");
}

// /days - dobowe stany licznika [RRRRMMDD, pobor, T1..T4, oddanie (, 1 = wpisany recznie)] w Wh
void handleDays() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json", "");
  String buf = "{\"d\":[";
  File f = LittleFS.open(DAYS_FILE, "r");
  DayRec r;
  bool first = true;
  auto add = [&](uint32_t v) {
    buf += ',';
    if (v == DAY_NONE) buf += "null";
    else buf += v;
  };
  while (f && f.read((uint8_t *)&r, sizeof r) == sizeof r) {
    if (!first) buf += ',';
    first = false;
    buf += '[';
    buf += dayOf(r);
    for (uint32_t v : r.imp) add(v);
    add(r.exp);
    if (r.date & DAY_MANUAL) buf += ",1";
    buf += ']';
    if (buf.length() > 1200) {
      server.sendContent(buf);
      buf = "";
    }
  }
  buf += "]}";
  server.sendContent(buf);
  server.sendContent("");
}

// /voltlog - odchylenia napiecia [poczatek, koniec, faza, przepiecie, min, maks (0,1 V) (, 1 = trwa)]
void handleVoltLog() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json", "");
  String buf = "{\"min\":" + String(VOLT_LOW, 0) + ",\"max\":" + String(VOLT_HIGH, 0) + ",\"e\":[";
  bool first = true;
  auto add = [&](const VoltEv &e, bool open) {
    if (!first) buf += ',';
    first = false;
    buf += '[';
    for (uint32_t v : {e.start, e.end, (uint32_t)e.phase, (uint32_t)e.high, (uint32_t)e.minDv, (uint32_t)e.maxDv}) {
      buf += v;
      buf += ',';
    }
    buf += open ? "1]" : "0]";
    if (buf.length() > 1200) {
      server.sendContent(buf);
      buf = "";
    }
  };
  File f = LittleFS.open(VOLT_FILE, "r");
  VoltEv e;
  while (f && f.read((uint8_t *)&e, sizeof e) == sizeof e) add(e, false);
  for (int p = 0; p < 3; p++)
    if (voltIsOpen[p]) add(voltOpen[p], true);
  buf += "]}";
  server.sendContent(buf);
  server.sendContent("");
}

// POST /voltclear - usuwa zapisane odchylenia napiecia
void handleVoltClear() {
  if (!checkAuth()) return;
  LittleFS.remove(VOLT_FILE);
  voltCount = 0;
  server.sendHeader("Location", "/#napiecia");
  server.send(303);
}

void setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Lacze z Wi-Fi %s", WIFI_SSID);
  for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print('.');
  }
  if (WiFi.status() == WL_CONNECTED)
    Serial.printf("\nWi-Fi OK, IP: %s  (http://%s.local/)\n", WiFi.localIP().toString().c_str(), HOSTNAME);
  else
    Serial.println("\nBrak polaczenia Wi-Fi - bede probowal dalej w tle");
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== Odczyt licznika HAN (wM-Bus T1) ===");
  EEPROM.begin(sizeof(Settings));
  httpUpdater.setup(&server, "/update", ADMIN_USER, ADMIN_PASSWORD);
  loadSettings();
  fsOk = LittleFS.begin();
  loadDays();
  loadHistory();
  loadVolt();
  Serial.printf("LittleFS: %s, zapisanych dob: %u, probek mocy: %d, odchylen napiecia: %u\n", fsOk ? "OK" : "BLAD",
                (unsigned)dayCount, histCount, (unsigned)voltCount);
  configTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.google.com");
  Serial.printf("Klucz AES: %s, filtr ID: %s\n", haveKey ? "ustawiony" : "BRAK", meterIdFilter.isEmpty() ? "brak" : meterIdFilter.c_str());

  radioOk = radio.begin(PIN_CC1101_MISO, PIN_CC1101_CS);
  Serial.printf("CC1101: %s (partnum 0x%02X, version 0x%02X)\n", radioOk ? "OK" : "NIE WYKRYTO - sprawdz polaczenia",
                radio.partnum(), radio.version());

  setupWifi();
  // OTA z PlatformIO (env:ota); uruchamia tez mDNS licznik-han.local
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.onStart([] {
    saveHistory();
    closeAllVolt();
    Serial.println("OTA: start");
  });
  ArduinoOTA.onError([](ota_error_t e) { Serial.printf("OTA: blad %u\n", e); });
  ArduinoOTA.begin();

  server.on("/", [] { server.send_P(200, "text/html", INDEX_HTML); });
  server.on("/json", handleJson);
  server.on("/history", handleHistory);
  server.on("/days", handleDays);
  server.on("/voltlog", handleVoltLog);
  server.on("/voltclear", HTTP_POST, handleVoltClear);
  server.on("/selftest", HTTP_POST, handleSelfTest);
  server.on("/config", HTTP_POST, handleConfig);
  server.on("/reading", HTTP_POST, handleReading);
  server.on("/cloudregister", HTTP_POST, handleCloudRegister);
  server.on("/cloudapp", handleCloudApp);
  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void loop() {
  static uint8_t frame[300];
  if (radioOk) {
    int n = radio.poll(frame, sizeof(frame));
    if (n > 0) handleFrame(frame, n);
  }
  server.handleClient();
  ArduinoOTA.handle();
  MDNS.update();

  static uint32_t lastHistSave = 0;
  if (millis() - lastHistSave > HIST_SAVE_MS) {
    lastHistSave = millis();
    saveHistory();
  }

  static uint32_t lastCloud = 0;
  if (millis() - lastCloud > cloudWaitS() * 1000UL) {
    lastCloud = millis();
    cloudSend();
  }

  static uint32_t lastWifiLog = 0;
  if (millis() - lastWifiLog > 60000) {
    lastWifiLog = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Wi-Fi rozlaczone, ponawiam...");
      WiFi.reconnect();
    }
  }
}
