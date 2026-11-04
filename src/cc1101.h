#pragma once
#include <Arduino.h>

// Minimalny sterownik CC1101 do odbioru wireless M-Bus w trybie T1
// (868.95 MHz, 2-FSK, ~100 kchip/s, sync 0x543D, kodowanie 3-z-6).
class CC1101 {
 public:
  bool begin(int miso, int cs);
  void startRx();

  // Zwraca dlugosc odebranej ramki (bajty po dekodowaniu 3-z-6, z CRC)
  // albo 0 jesli brak / blad. Funkcja blokuje na czas odbioru jednej ramki (~50 ms).
  int poll(uint8_t *out, int maxLen);

  uint8_t partnum() const { return partnum_; }
  uint8_t version() const { return version_; }
  int lastRssi() const { return rssi_; }
  int currentRssi() { return readRssi(); }

  // Statystyka odbioru: wykryte slowa synchronizacji, w tym nierozpoznane (blad 3-z-6)
  // i wygladajace na ramki trybu C1 (nieobslugiwanego)
  uint32_t syncs() const { return syncs_; }
  uint32_t badSyncs() const { return badSyncs_; }
  uint32_t c1Syncs() const { return c1Syncs_; }

  // Test przewodow miedzy ESP a modulem (zwarcia, brak odpowiedzi, SPI, GDO0). Przerywa odbior
  // na ok. 30 ms i ponownie inicjalizuje radio (wynik begin() w radioOk). Zwraca JSON.
  String selfTest(int sck, int mosi, int gdo0, bool &radioOk);

 private:
  uint8_t strobe(uint8_t cmd);
  void writeReg(uint8_t addr, uint8_t val);
  uint8_t readReg(uint8_t addr);
  uint8_t readStatus(uint8_t addr);
  uint8_t rxBytes();
  void readFifo(uint8_t *buf, int n);
  void select();
  void deselect();
  int readRssi();

  int cs_ = -1, miso_ = -1;
  uint8_t partnum_ = 0, version_ = 0;
  int rssi_ = 0;
  uint32_t syncs_ = 0, badSyncs_ = 0, c1Syncs_ = 0;
};
