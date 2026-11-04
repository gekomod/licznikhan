#pragma once
#include <Arduino.h>

// Dane z licznika AMI (HAN Tauron, tabela 5 "Opis modelu komunikacji modulu wireless M-BUS")
struct MeterData {
  String linkId;          // ID z naglowka ramki (8 cyfr)
  String manufacturer;    // kod producenta (3 litery)
  String serial;          // 0.0:C.1.0   numer seryjny
  String dateTime;        // 0.0:1.0.0   data i czas licznika
  String message;         // 0.0:C.13.0  komunikat dla uzytkownika
  double importKWh[5];    // 1.0:1.8.x   [0]=suma, [1..4]=taryfy T1..T4
  double exportKWh[5];    // 1.0:2.8.x
  double powerImportW;    // 1.0:1.7.0
  double powerExportW;    // 1.0:2.7.0
  double maxPowerW;       // 1.0:1.6.0
  double powerLimitW;     // 1.0:15.35.0
  double voltage[3];      // 1.0:32/52/72.7.0
  String records;         // wszystkie rekordy (diagnostyka)
  uint16_t recTotal;      // liczba rekordow w ramce
  uint16_t recUsed;       // rekordy przypisane do pol powyzej
  uint16_t restBytes;     // bajty po ostatnim rekordzie (poza wypelniaczem 2F)
  String parseNote;       // powod przerwania analizy, gdy nie doszla do konca danych
  void clear();
};

enum class FrameResult { Ok, CrcError, Encrypted_NoKey, DecryptError, Unsupported };

const char *frameResultText(FrameResult r);

// Sprawdza CRC i odczytuje naglowek. linkId/manufacturer sa ustawiane nawet przy bledzie deszyfrowania.
FrameResult parseFrame(const uint8_t *frame, int len, const uint8_t *aesKey, bool haveKey, MeterData &out);
