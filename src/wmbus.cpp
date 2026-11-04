#include "wmbus.h"
#include <bearssl/bearssl_block.h>
#include <math.h>

void MeterData::clear() {
  linkId = manufacturer = serial = dateTime = message = records = parseNote = "";
  recTotal = recUsed = restBytes = 0;
  for (int i = 0; i < 5; i++) importKWh[i] = exportKWh[i] = NAN;
  powerImportW = powerExportW = maxPowerW = powerLimitW = NAN;
  for (int i = 0; i < 3; i++) voltage[i] = NAN;
}

const char *frameResultText(FrameResult r) {
  switch (r) {
    case FrameResult::Ok: return "OK";
    case FrameResult::CrcError: return "blad CRC";
    case FrameResult::Encrypted_NoKey: return "zaszyfrowana - brak klucza AES";
    case FrameResult::DecryptError: return "bledny klucz AES (brak 2F2F po deszyfrowaniu)";
    case FrameResult::Unsupported: return "nieobslugiwany format";
  }
  return "?";
}

namespace {

uint16_t crc16(const uint8_t *d, int n) {
  uint16_t crc = 0;
  for (int i = 0; i < n; i++) {
    crc ^= (uint16_t)d[i] << 8;
    for (int b = 0; b < 8; b++) crc = (crc & 0x8000) ? (crc << 1) ^ 0x3D65 : crc << 1;
  }
  return ~crc;
}

// Ramka formatu A: blok 1 = 10 B + CRC, kolejne = 16 B + CRC. Zwraca dlugosc danych albo -1.
int stripCrc(const uint8_t *in, int len, uint8_t *out) {
  int L = in[0];
  int total = L + 1;
  int pos = 0, o = 0;
  while (o < total) {
    int blk = (o == 0) ? 10 : min(16, total - o);
    if (pos + blk + 2 > len) return -1;
    uint16_t c = crc16(in + pos, blk);
    if (((c >> 8) & 0xFF) != in[pos + blk] || (c & 0xFF) != in[pos + blk + 1]) return -1;
    memcpy(out + o, in + pos, blk);
    o += blk;
    pos += blk + 2;
  }
  return total;
}

String manufacturerCode(uint16_t m) {
  char s[4] = {(char)(((m >> 10) & 0x1F) + 64), (char)(((m >> 5) & 0x1F) + 64), (char)((m & 0x1F) + 64), 0};
  return s;
}

String idString(const uint8_t *p) {  // 4 bajty BCD, LSB first
  char s[9];
  snprintf(s, sizeof(s), "%02X%02X%02X%02X", p[3], p[2], p[1], p[0]);
  return s;
}

double bcdValue(const uint8_t *p, int n, bool &ok) {
  double v = 0;
  ok = true;
  bool neg = false;
  for (int i = n - 1; i >= 0; i--) {
    uint8_t hi = p[i] >> 4, lo = p[i] & 0x0F;
    if (i == n - 1 && hi == 0x0F) { neg = true; hi = 0; }
    if (hi > 9 || lo > 9) ok = false;
    v = v * 100 + hi * 10 + lo;
  }
  return neg ? -v : v;
}

double floatValue(const uint8_t *p) {  // typ 5: 32-bit IEEE 754, LSB first
  uint32_t u = p[0] | (p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
  float f;
  memcpy(&f, &u, 4);
  return f;
}

double intValue(const uint8_t *p, int n) {
  int64_t v = 0;
  for (int i = n - 1; i >= 0; i--) v = (v << 8) | p[i];
  if (n < 8 && (p[n - 1] & 0x80)) v -= (int64_t)1 << (8 * n);  // liczba ze znakiem
  return (double)v;
}

String dateTimeString(const uint8_t *p, int n) {
  char s[24];
  if (n == 6) {  // typ I (CP48)
    int sec = p[0] & 0x3F, min = p[1] & 0x3F, hour = p[2] & 0x1F, day = p[3] & 0x1F, mon = p[4] & 0x0F;
    int year = ((p[3] & 0xE0) >> 5) | ((p[4] & 0xF0) >> 1);
    if (year == 127)  // rok "dowolny" - data powtarzana co roku (np. granice okresow taryfowych)
      snprintf(s, sizeof(s), "co roku %02d-%02d %02d:%02d:%02d", mon, day, hour, min, sec);
    else
      snprintf(s, sizeof(s), "20%02d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, sec);
  } else if (n == 4) {  // typ F (CP32)
    int min = p[0] & 0x3F, hour = p[1] & 0x1F, day = p[2] & 0x1F, mon = p[3] & 0x0F;
    int year = ((p[2] & 0xE0) >> 5) | ((p[3] & 0xF0) >> 1);
    if (year == 127)
      snprintf(s, sizeof(s), "co roku %02d-%02d %02d:%02d", mon, day, hour, min);
    else
      snprintf(s, sizeof(s), "20%02d-%02d-%02d %02d:%02d", year, mon, day, hour, min);
  } else {
    return "";
  }
  return s;
}

void parseRecords(const uint8_t *p, int len, MeterData &out) {
  static const int8_t sizes[16] = {0, 1, 2, 3, 4, 4, 6, 8, 0, 1, 2, 3, 4, -1, 6, 0};
  int i = 0;
  auto stop = [&](const char *why) { out.parseNote = String(why) + " (bajt " + String(i) + "/" + String(len) + ")"; };
  while (i < len) {
    uint8_t dif = p[i++];
    if (dif == 0x2F) continue;                  // wypelniacz
    if ((dif & 0x0F) == 0x0F) {                 // dane producenta / koniec
      i--;
      break;
    }
    int function = (dif >> 4) & 0x03;
    // numer pamieci: bit 6 DIF + bity 0..3 kolejnych DIFE; > 0 = wartosci archiwalne, nie biezace
    uint32_t storage = (dif >> 6) & 1;
    int tariff = 0, k = 0;
    uint8_t last = dif;
    while ((last & 0x80) && i < len) {
      last = p[i++];
      tariff |= ((last >> 4) & 0x03) << (2 * k);
      storage |= (uint32_t)(last & 0x0F) << (1 + 4 * k);
      k++;
    }
    if (i >= len) {
      stop("urwany rekord (DIF)");
      break;
    }
    uint8_t vif = p[i++];
    uint8_t vifes[8];
    int nvife = 0;
    last = vif;
    while ((last & 0x80) && i < len) {
      last = p[i++];
      if (nvife < 8) vifes[nvife++] = last;
    }

    int type = dif & 0x0F;
    int size = sizes[type];
    if (size < 0) {  // zmienna dlugosc (LVAR)
      if (i >= len) {
        stop("urwany rekord (LVAR)");
        break;
      }
      size = p[i++];
      if (size > 0xBF) {
        stop("nieobslugiwany LVAR");
        break;
      }
    }
    if (i + size > len) {
      stop("urwany rekord (dane)");
      break;
    }
    const uint8_t *d = p + i;
    i += size;

    bool isBcd = type >= 0x09 && type <= 0x0E && type != 0x0D;
    bool ok = true;
    double raw = isBcd ? bcdValue(d, size, ok)
                 : type == 0x05 ? floatValue(d)
                 : (size > 0 && type != 0x0D ? intValue(d, size) : 0);
    out.recTotal++;
    // do pol biezacych tylko wartosci z pamieci 0 i bez funkcji min/blad; reszta tylko w diagnostyce
    bool current = storage == 0 && function != 2 && function != 3;
    bool used = false;

    bool exportFlag = false, hasLimit = false;
    for (int v = 0; v < nvife; v++) {
      if ((vifes[v] & 0x7F) == 0x3C) exportFlag = true;
      if ((vifes[v] & 0x7F) == 0x48) hasLimit = true;
    }

    char rec[96];
    snprintf(rec, sizeof(rec), "DIF %02X VIF %02X", dif, vif);
    String r = rec;
    for (int v = 0; v < nvife; v++) {
      snprintf(rec, sizeof(rec), " %02X", vifes[v]);
      r += rec;
    }
    r += " T" + String(tariff);
    if (storage) r += " S" + String(storage);
    if (function) r += function == 1 ? " max" : function == 2 ? " min" : " blad";
    r += " = ";

    uint8_t v7 = vif & 0x7F;
    if (type == 0x0D) {  // tekst, bajty w odwrotnej kolejnosci
      String s;
      for (int c = size - 1; c >= 0; c--)
        if (d[c] >= 0x20 && d[c] < 0x7F) s += (char)d[c];
      s.trim();
      if (current) {
        out.message = s;
        used = true;
      }
      r += "\"" + s + "\"";
    } else if (v7 <= 0x07 && ok) {  // energia [Wh * 10^(n-3)]
      double kwh = raw * pow(10, (int)(v7 & 0x07) - 3) / 1000.0;
      if (tariff <= 4 && current && function == 0) {
        (exportFlag ? out.exportKWh : out.importKWh)[tariff] = kwh;
        used = true;
      }
      r += String(kwh, 3) + " kWh" + (exportFlag ? " (oddana)" : " (pobrana)");
    } else if (v7 >= 0x28 && v7 <= 0x2F && ok) {  // moc [W * 10^(n-3)]
      double w = raw * pow(10, (int)(v7 & 0x07) - 3);
      if (current && tariff == 0) {
        used = true;
        if (hasLimit) out.powerLimitW = w;
        else if (function == 1 && !exportFlag) out.maxPowerW = w;
        else if (function == 1) used = false;  // maks. mocy oddanej - brak pola
        else if (exportFlag) out.powerExportW = w;
        else out.powerImportW = w;
      }
      r += String(w, 0) + " W" + (exportFlag ? " (oddana)" : hasLimit ? " (prog)" : function == 1 ? " (max)" : "");
    } else if (vif == 0xFD && nvife > 0 && (vifes[0] & 0x70) == 0x40 && ok) {  // napiecie [V * 10^(n-9)]
      double volt = raw * pow(10, (int)(vifes[0] & 0x0F) - 9);
      int phase = (nvife >= 3) ? (vifes[nvife - 1] & 0x7F) : 1;
      if (phase >= 1 && phase <= 3 && current && function == 0) {
        out.voltage[phase - 1] = volt;
        used = true;
      }
      r += String(volt, 1) + " V L" + String(phase);
    } else if (v7 == 0x6D) {
      String dt = dateTimeString(d, size);
      if (vif == 0x6D && current) {  // czas licznika; ED xx = inne daty (np. okresy taryf)
        out.dateTime = dt;
        used = true;
      }
      r += dt;
    } else if (v7 == 0x78) {
      char s[20];
      snprintf(s, sizeof(s), "%.0f", raw);
      out.serial = s;
      used = true;
      r += s;
    } else {
      r += String(raw, 0);
    }
    if (used) out.recUsed++;
    out.records += r + (used ? "\n" : "  [nieuzyty]\n");
  }
  for (int j = i; j < len; j++)  // dane po ostatnim rekordzie (np. producenta), bez wypelniacza
    if (p[j] != 0x2F) out.restBytes++;
  out.records += "-- rekordy: " + String(out.recTotal) + ", uzyte: " + String(out.recUsed) +
                 ", bajty po rekordach: " + String(out.restBytes) +
                 (out.parseNote.length() ? ", przerwano: " + out.parseNote : String()) + "\n";
}

}  // namespace

FrameResult parseFrame(const uint8_t *frame, int len, const uint8_t *aesKey, bool haveKey, MeterData &out) {
  out.clear();
  static uint8_t d[300];
  int n = stripCrc(frame, len, d);
  if (n < 11) return FrameResult::CrcError;

  // Naglowek warstwy lacza: L C M M A A A A V T CI
  out.manufacturer = manufacturerCode(d[2] | (d[3] << 8));
  out.linkId = idString(d + 4);

  uint8_t iv[16];
  memcpy(iv, d + 2, 8);  // M(2) + A(6)
  int pos = 10;
  uint8_t ci = d[pos++];

  if (ci == 0x8C && pos + 2 < n) {  // ELL bez szyfrowania: CC ACC, potem wlasciwe CI
    pos += 2;
    ci = d[pos++];
  }

  uint8_t acc;
  uint16_t cw;
  if (ci == 0x7A) {  // krotki naglowek
    if (pos + 4 > n) return FrameResult::Unsupported;
    acc = d[pos];
    cw = d[pos + 2] | (d[pos + 3] << 8);
    pos += 4;
  } else if (ci == 0x72) {  // dlugi naglowek: ID(4) M(2) V T ACC STS CW(2)
    if (pos + 12 > n) return FrameResult::Unsupported;
    iv[0] = d[pos + 4];
    iv[1] = d[pos + 5];
    memcpy(iv + 2, d + pos, 4);
    iv[6] = d[pos + 6];
    iv[7] = d[pos + 7];
    acc = d[pos + 8];
    cw = d[pos + 10] | (d[pos + 11] << 8);
    pos += 12;
  } else if (ci == 0x78) {  // bez naglowka, bez szyfrowania
    parseRecords(d + pos, n - pos, out);
    return FrameResult::Ok;
  } else {
    return FrameResult::Unsupported;
  }

  int mode = (cw >> 8) & 0x1F;
  uint8_t *payload = d + pos;
  int plen = n - pos;

  if (mode == 0) {
    parseRecords(payload, plen, out);
    return FrameResult::Ok;
  }
  if (mode != 5) return FrameResult::Unsupported;
  if (!haveKey) return FrameResult::Encrypted_NoKey;

  int blocks = (cw >> 4) & 0x0F;
  int encLen = blocks ? blocks * 16 : (plen / 16) * 16;
  if (encLen > plen || encLen == 0) return FrameResult::DecryptError;

  memset(iv + 8, acc, 8);
  br_aes_small_cbcdec_keys ctx;
  br_aes_small_cbcdec_init(&ctx, aesKey, 16);
  br_aes_small_cbcdec_run(&ctx, iv, payload, encLen);

  if (payload[0] != 0x2F || payload[1] != 0x2F) return FrameResult::DecryptError;
  parseRecords(payload, plen, out);
  return FrameResult::Ok;
}
