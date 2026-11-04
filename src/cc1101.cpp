#include "cc1101.h"
#include <SPI.h>

namespace {

constexpr uint8_t SRES = 0x30, SCAL = 0x33, SRX = 0x34, SIDLE = 0x36, SFRX = 0x3A;
constexpr uint8_t REG_PARTNUM = 0x30, REG_VERSION = 0x31, REG_RSSI = 0x34,
                  REG_MARCSTATE = 0x35, REG_RXBYTES = 0x3B, REG_FIFO = 0x3F;
constexpr uint8_t READ_SINGLE = 0x80, READ_BURST = 0xC0;

// Rejestry 0x00..0x2E: wM-Bus T-mode, 868.95 MHz, 103 kBaud, 2-FSK,
// sync 0x543D, nieskonczona dlugosc pakietu (dlugosc liczymy z pola L).
const uint8_t T_MODE_REGS[0x2F] = {
    0x2E,  // 00 IOCFG2   hi-Z
    0x2E,  // 01 IOCFG1   hi-Z
    0x06,  // 02 IOCFG0   GDO0 = sync word wykryty (opcjonalne)
    0x07,  // 03 FIFOTHR
    0x54,  // 04 SYNC1
    0x3D,  // 05 SYNC0
    0xFF,  // 06 PKTLEN
    0x00,  // 07 PKTCTRL1
    0x02,  // 08 PKTCTRL0 infinite length, bez CRC, bez whiteningu
    0x00,  // 09 ADDR
    0x00,  // 0A CHANNR
    0x08,  // 0B FSCTRL1
    0x00,  // 0C FSCTRL0
    0x21,  // 0D FREQ2    868.95 MHz
    0x6B,  // 0E FREQ1
    0xD0,  // 0F FREQ0
    0x5C,  // 10 MDMCFG4  BW 325 kHz
    0x04,  // 11 MDMCFG3  103 kBaud
    0x05,  // 12 MDMCFG2  2-FSK, 15/16 sync + carrier sense
    0x22,  // 13 MDMCFG1
    0xF8,  // 14 MDMCFG0
    0x44,  // 15 DEVIATN  ~38 kHz
    0x07,  // 16 MCSM2
    0x00,  // 17 MCSM1
    0x18,  // 18 MCSM0    autokalibracja IDLE->RX
    0x2E,  // 19 FOCCFG
    0xBF,  // 1A BSCFG
    0x43,  // 1B AGCCTRL2
    0x09,  // 1C AGCCTRL1
    0xB5,  // 1D AGCCTRL0
    0x87,  // 1E WOREVT1
    0x6B,  // 1F WOREVT0
    0xFB,  // 20 WORCTRL
    0xB6,  // 21 FREND1
    0x10,  // 22 FREND0
    0xEA,  // 23 FSCAL3
    0x2A,  // 24 FSCAL2
    0x00,  // 25 FSCAL1
    0x1F,  // 26 FSCAL0
    0x41,  // 27 RCCTRL1
    0x00,  // 28 RCCTRL0
    0x59,  // 29 FSTEST
    0x7F,  // 2A PTEST
    0x3F,  // 2B AGCTEST
    0x81,  // 2C TEST2
    0x35,  // 2D TEST1
    0x09,  // 2E TEST0
};

// Kodowanie 3-z-6 (EN 13757-4): 6-bitowy symbol -> polbajt, 0xFF = niepoprawny
int8_t decode6(uint8_t code) {
  static const uint8_t table[16] = {0x16, 0x0D, 0x0E, 0x0B, 0x1C, 0x19, 0x1A, 0x13,
                                    0x2C, 0x25, 0x26, 0x23, 0x34, 0x31, 0x32, 0x29};
  for (int i = 0; i < 16; i++)
    if (table[i] == code) return i;
  return -1;
}

// Dekoduje `outLen` bajtow z surowego strumienia 3-z-6. false przy blednym symbolu.
bool decode3of6(const uint8_t *raw, uint8_t *out, int outLen) {
  int bitPos = 0;
  for (int i = 0; i < outLen * 2; i++) {
    uint8_t sym = 0;
    for (int b = 0; b < 6; b++, bitPos++) {
      uint8_t bit = (raw[bitPos >> 3] >> (7 - (bitPos & 7))) & 1;
      sym = (sym << 1) | bit;
    }
    int8_t nib = decode6(sym);
    if (nib < 0) return false;
    if (i & 1)
      out[i >> 1] |= nib;
    else
      out[i >> 1] = nib << 4;
  }
  return true;
}

// Dlugosc ramki formatu A (z blokami CRC) na podstawie pola L
int frameLenFromL(uint8_t L) {
  int n = L + 1;
  int crcs = (n <= 10) ? 1 : 1 + (n - 10 + 15) / 16;
  return n + 2 * crcs;
}

}  // namespace

void CC1101::select() {
  digitalWrite(cs_, LOW);
  uint32_t t = micros();
  while (digitalRead(miso_) && micros() - t < 1000) {
  }
}

void CC1101::deselect() { digitalWrite(cs_, HIGH); }

uint8_t CC1101::strobe(uint8_t cmd) {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  select();
  uint8_t s = SPI.transfer(cmd);
  deselect();
  SPI.endTransaction();
  return s;
}

void CC1101::writeReg(uint8_t addr, uint8_t val) {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  select();
  SPI.transfer(addr);
  SPI.transfer(val);
  deselect();
  SPI.endTransaction();
}

uint8_t CC1101::readReg(uint8_t addr) {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  select();
  SPI.transfer(addr | READ_SINGLE);
  uint8_t v = SPI.transfer(0);
  deselect();
  SPI.endTransaction();
  return v;
}

uint8_t CC1101::readStatus(uint8_t addr) {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  select();
  SPI.transfer(addr | READ_BURST);
  uint8_t v = SPI.transfer(0);
  deselect();
  SPI.endTransaction();
  return v;
}

uint8_t CC1101::rxBytes() {
  // Errata CC1101: czytaj dwa razy az wartosci beda zgodne
  uint8_t a, b;
  do {
    a = readStatus(REG_RXBYTES);
    b = readStatus(REG_RXBYTES);
  } while (a != b);
  return a;
}

void CC1101::readFifo(uint8_t *buf, int n) {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  select();
  SPI.transfer(REG_FIFO | READ_BURST);
  for (int i = 0; i < n; i++) buf[i] = SPI.transfer(0);
  deselect();
  SPI.endTransaction();
}

int CC1101::readRssi() {
  int raw = readStatus(REG_RSSI);
  if (raw >= 128) raw -= 256;
  return raw / 2 - 74;
}

bool CC1101::begin(int miso, int cs) {
  cs_ = cs;
  miso_ = miso;
  pinMode(cs_, OUTPUT);
  deselect();
  SPI.begin();

  // Reset wg datasheetu
  digitalWrite(cs_, LOW);
  delayMicroseconds(10);
  digitalWrite(cs_, HIGH);
  delayMicroseconds(40);
  strobe(SRES);
  delay(10);

  partnum_ = readStatus(REG_PARTNUM);
  version_ = readStatus(REG_VERSION);
  if (version_ == 0x00 || version_ == 0xFF) return false;  // brak ukladu na SPI

  for (uint8_t a = 0; a < sizeof(T_MODE_REGS); a++) writeReg(a, T_MODE_REGS[a]);
  strobe(SIDLE);
  strobe(SCAL);
  delay(2);
  startRx();
  return true;
}

void CC1101::startRx() {
  strobe(SIDLE);
  uint32_t t = millis();
  while ((readStatus(REG_MARCSTATE) & 0x1F) != 0x01 && millis() - t < 10) {
  }
  strobe(SFRX);
  writeReg(0x08, 0x02);  // PKTCTRL0: infinite
  strobe(SRX);
}

int CC1101::poll(uint8_t *out, int maxLen) {
  uint8_t n = rxBytes();
  if (n & 0x80) {  // przepelnienie FIFO
    startRx();
    return 0;
  }
  if (n == 0) return 0;
  syncs_++;

  static uint8_t raw[600];
  int got = 0;
  uint32_t t0 = millis();

  // Pierwsze 3 surowe bajty = 2 bajty po dekodowaniu (L + C)
  while (got < 3) {
    uint8_t avail = rxBytes() & 0x7F;
    if (avail) {
      int take = min<int>(avail, 3 - got);
      readFifo(raw + got, take);
      got += take;
    } else if (millis() - t0 > 20) {
      startRx();
      return 0;
    }
  }
  rssi_ = readRssi();

  uint8_t lc[2];
  if (!decode3of6(raw, lc, 2)) {
    badSyncs_++;
    // Tryb C1: po 0x543D nastepuje 0x54CD (format A) lub 0x543D (format B), bez kodowania 3-z-6
    if (raw[0] == 0x54 && (raw[1] == 0xCD || raw[1] == 0x3D)) c1Syncs_++;
    startRx();
    return 0;
  }
  int frameLen = frameLenFromL(lc[0]);
  int rawLen = (frameLen * 3 + 1) / 2;
  if (lc[0] < 9 || frameLen > maxLen || rawLen > (int)sizeof(raw)) {
    startRx();
    return 0;
  }

  // Reszta ramki - czytaj FIFO zanim sie przepelni (64 B ~ 7 ms)
  while (got < rawLen) {
    uint8_t avail = rxBytes();
    if (avail & 0x80) {
      startRx();
      return 0;
    }
    avail &= 0x7F;
    // Errata: w trakcie odbioru nie oprozniaj FIFO do zera (zostaw 1 bajt)
    int remaining = rawLen - got;
    int take = min<int>(avail, remaining);
    if (take < remaining) take -= 1;
    if (take > 0) {
      readFifo(raw + got, take);
      got += take;
      t0 = millis();
    } else if (millis() - t0 > 30) {
      startRx();
      return 0;
    }
  }
  startRx();

  if (!decode3of6(raw, out, frameLen)) return 0;
  return frameLen;
}

// ---------------- Test polaczen ----------------

String CC1101::selfTest(int sck, int mosi, int gdo0, bool &radioOk) {
  enum { VCC, GND, SCK, MISO, MOSI, CSN, GDO0, NPINS };
  enum { OK, WARN, BAD, SKIP };
  struct Pin {
    const char *cc, *esp;
    int gpio;
    int st;
    String msg;
  } p[NPINS] = {{"VCC", "3V3", -1, OK, ""},   {"GND", "GND", -1, OK, ""},  {"SCK", "D5", sck, OK, ""},
                {"MISO", "D6", miso_, OK, ""}, {"MOSI", "D7", mosi, OK, ""}, {"CSN", "D8", cs_, OK, ""},
                {"GDO0", "D2", gdo0, OK, ""}};
  auto mark = [&](int i, int st, const String &m) {
    if (st > p[i].st) p[i].st = st;
    if (m.length()) p[i].msg += (p[i].msg.length() ? "; " : "") + m;
  };

  // 1. Przy CSN=1 wyjscie SO modulu jest w stanie wysokiej impedancji (podciagniete = 1),
  //    po CSN=0 gotowy uklad ustawia SO=0 (CHIP_RDYn). Nie wymaga SCK ani MOSI.
  SPI.end();
  pinMode(cs_, OUTPUT);
  digitalWrite(cs_, HIGH);
  pinMode(miso_, INPUT_PULLUP);
  delayMicroseconds(100);
  bool misoIdle = digitalRead(miso_);
  digitalWrite(cs_, LOW);
  bool ready = false;
  for (uint32_t t = micros(); micros() - t < 2000;)
    if (!digitalRead(miso_)) {
      ready = true;
      break;
    }
  digitalWrite(cs_, HIGH);

  // 2. Komunikacja SPI: wersja ukladu i zapis/odczyt rejestru PKTLEN wzorcami bitow
  SPI.begin();
  strobe(SRES);
  delay(10);
  uint8_t ver = readStatus(REG_VERSION);
  bool spiOk = ver != 0x00 && ver != 0xFF;
  int rbFail = 0;
  uint8_t rbDiff = 0;
  if (spiOk)
    for (uint8_t v : {0x55, 0xAA, 0x00, 0xFF, 0x0F, 0xF0, 0x01, 0x80}) {
      writeReg(0x06, v);
      uint8_t r = readReg(0x06);
      if (r != v) {
        rbFail++;
        rbDiff |= r ^ v;
      }
    }

  // 3. GDO0: modul wystawia na wybranym wyjsciu na stale 0 (IOCFGx=0x2F), potem 1 (0x6F).
  //    Sprawdzane tez GDO2 - czesty przypadek: D2 podlaczony do GDO2 zamiast GDO0.
  if (spiOk)
    for (uint8_t a = 0; a < 3; a++) writeReg(a, 0x2E);  // GDO0..2 w stan wysokiej impedancji
  auto gdoFollows = [&](uint8_t reg) {
    pinMode(gdo0, INPUT_PULLUP);
    writeReg(reg, 0x2F);
    delayMicroseconds(50);
    bool lo = !digitalRead(gdo0);
    pinMode(gdo0, INPUT);
    writeReg(reg, 0x6F);
    delayMicroseconds(50);
    bool hi = digitalRead(gdo0);
    writeReg(reg, 0x2E);
    pinMode(gdo0, INPUT_PULLUP);
    return lo && hi;
  };
  bool gdoOk = false, gdo2 = false, gdoIdleLow = false;
  if (spiOk && gdo0 >= 0) {
    pinMode(gdo0, INPUT_PULLUP);
    delayMicroseconds(50);
    gdoIdleLow = !digitalRead(gdo0);
    gdoOk = gdoFollows(0x02);
    if (!gdoOk) gdo2 = gdoFollows(0x00);
  }

  // 4. Zwarcia: przy CSN=1 wszystkie linie modulu sa wejsciami lub w stanie wysokiej impedancji.
  //    Kazda linia po kolei = 0, pozostale podciagniete - odczyt 0 na innej oznacza zwarcie.
  SPI.end();
  digitalWrite(cs_, HIGH);
  int lines[4] = {SCK, MISO, MOSI, GDO0};
  int nl = (spiOk && gdo0 >= 0) ? 4 : 3;  // bez dzialajacego SPI GDO0 moze wystawiac zegar
  bool stuck[NPINS] = {};
  for (int i = 0; i < nl; i++) pinMode(p[lines[i]].gpio, INPUT_PULLUP);
  delayMicroseconds(100);
  for (int i = 0; i < nl; i++)
    if (!digitalRead(p[lines[i]].gpio)) {
      stuck[lines[i]] = true;
      mark(lines[i], BAD, "stan niski na stałe – zwarcie z GND lub z innym przewodem");
    }
  bool sh[NPINS][NPINS] = {};
  for (int a = 0; a < nl; a++) {
    int pa = p[lines[a]].gpio;
    pinMode(pa, OUTPUT);
    digitalWrite(pa, LOW);
    delayMicroseconds(50);
    for (int b = 0; b < nl; b++)
      if (b != a && !stuck[lines[b]] && !digitalRead(p[lines[b]].gpio)) sh[lines[a]][lines[b]] = sh[lines[b]][lines[a]] = true;
    pinMode(pa, INPUT_PULLUP);
    delayMicroseconds(50);
  }
  // CSN=0: SCK, MOSI, GDO0 musza zostac w stanie 1 (MISO ma wtedy 0 - to odpowiedz ukladu)
  digitalWrite(cs_, LOW);
  delayMicroseconds(50);
  for (int b : {SCK, MOSI, GDO0})
    if ((b != GDO0 || nl == 4) && !stuck[b] && !digitalRead(p[b].gpio)) sh[CSN][b] = sh[b][CSN] = true;
  digitalWrite(cs_, HIGH);
  for (int a = 0; a < NPINS; a++)
    for (int b = 0; b < NPINS; b++)
      if (sh[a][b]) mark(a, BAD, String("zwarcie z ") + p[b].cc + " (" + p[b].esp + ")");

  // 5. Ocena
  if (!misoIdle && !stuck[MISO]) mark(MISO, BAD, "stan niski przy CSN=1 – zwarcie z GND?");
  if (misoIdle && !ready) {
    const char *m = "moduł nie odpowiada po CSN=0 – sprawdź ten przewód";
    mark(CSN, BAD, m);
    mark(MISO, BAD, m);
    mark(VCC, WARN, "moduł nie odpowiada – sprawdź zasilanie 3,3 V");
    mark(GND, WARN, "moduł nie odpowiada – sprawdź masę");
  } else if (ready) {
    mark(VCC, OK, "moduł zasilony, odpowiada");
    mark(GND, OK, "");
  }
  char vs[48];
  snprintf(vs, sizeof vs, "odczyt wersji 0x%02X zamiast np. 0x14", ver);
  if (ready && !spiOk) {
    mark(SCK, BAD, String("moduł odpowiada, ale nie przyjmuje komend – ") + vs);
    mark(MOSI, BAD, String("moduł odpowiada, ale nie przyjmuje komend – ") + vs);
    if (ver == 0xFF) mark(MISO, WARN, vs);
  }
  if (spiOk && rbFail) {
    snprintf(vs, sizeof vs, "błędy zapisu/odczytu rejestru (bity 0x%02X)", rbDiff);
    mark(MOSI, WARN, vs);
    mark(SCK, WARN, vs);
    mark(MISO, WARN, vs);
  }
  if (spiOk && !rbFail)
    for (int i : {SCK, MISO, MOSI, CSN})
      if (p[i].st == OK && !p[i].msg.length()) mark(i, OK, "komunikacja SPI OK");
  if (gdo0 < 0) {
    p[GDO0].st = SKIP;
    p[GDO0].msg = "nieużywany";
  } else if (!spiOk) {
    if (p[GDO0].st == OK) {
      p[GDO0].st = SKIP;
      p[GDO0].msg = "nie sprawdzono (brak komunikacji SPI)";
    }
  } else if (gdoOk) {
    if (p[GDO0].st == OK) mark(GDO0, OK, "OK");
  } else if (gdo2) {
    mark(GDO0, WARN, "D2 jest podłączony do GDO2 zamiast GDO0 – nie przeszkadza, firmware go nie używa");
  } else if (gdoIdleLow) {
    mark(GDO0, WARN, "D2 ma stan niski niezależnie od modułu – zwarcie z GND? (GDO0 jest opcjonalny)");
  } else {
    mark(GDO0, WARN, "niepodłączony (opcjonalny – firmware go nie wymaga)");
  }

  // Przywrocenie normalnej pracy
  if (gdo0 >= 0) pinMode(gdo0, INPUT);
  radioOk = begin(miso_, cs_);
  int noise = 0;
  if (radioOk) {
    delay(5);
    for (int i = 0; i < 8; i++, delay(2)) noise += readRssi();
    noise /= 8;
  }

  int worst = OK;
  for (auto &x : p)
    if (x.st != SKIP && x.st > worst) worst = x.st;
  static const char *stName[] = {"ok", "warn", "bad", "skip"};
  String j = "{\"result\":\"" + String(stName[worst]) + "\",\"version\":" + String(ver) +
             ",\"noise_dbm\":" + (radioOk ? String(noise) : String("null")) + ",\"pins\":[";
  for (int i = 0; i < NPINS; i++) {
    if (i) j += ',';
    j += String("{\"cc\":\"") + p[i].cc + "\",\"esp\":\"" + p[i].esp + "\",\"s\":\"" + stName[p[i].st] +
         "\",\"m\":\"" + p[i].msg + "\"}";
  }
  return j + "]}";
}
