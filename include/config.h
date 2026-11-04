#pragma once

// ---------------- Wi-Fi ----------------
// Mozna nadpisac flagami kompilacji (-D WIFI_SSID=...), np. w osobnym env w platformio.ini
#ifndef WIFI_SSID
#define WIFI_SSID     "Xiaomi_E9C7"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "nowak1234"
#endif
#define HOSTNAME      "licznik-han"

// ---------------- Haslo administratora ----------------
// Uzytkownik "admin". Chroni zmiane ustawien na stronie WWW, aktualizacje /update i OTA z PlatformIO.
// Domyslne haslo - zmien je na stronie (Ustawienia), wtedy zmien tez --auth w platformio.ini [env:ota].
#define ADMIN_USER     "admin"
#define ADMIN_PASSWORD "licznik"

// ---------------- CC1101 (sprzetowe SPI NodeMCU) ----------------
// CC1101 zasilany z 3.3V (NIE 5V!)
// SCK = D5 (GPIO14), MISO = D6 (GPIO12), MOSI = D7 (GPIO13) - stale piny HSPI
#define PIN_CC1101_SCK   14   // D5 (tylko do testu polaczen)
#define PIN_CC1101_MOSI  13   // D7 (tylko do testu polaczen)
#define PIN_CC1101_MISO  12   // D6
#define PIN_CC1101_CS    15   // D8
#define PIN_CC1101_GDO0   4   // D2, opcjonalnie - firmware odpytuje FIFO po SPI

// ---------------- Licznik ----------------
// Klucz AES-128 (32 znaki hex) z platformy TAURON eLicznik.
// Mozna go tez wpisac pozniej przez strone WWW (zapisuje sie w pamieci ESP).
#define DEFAULT_AES_KEY  ""

// ID licznika (8 cyfr z ramki / numer seryjny), pusty = pokazuj wszystkie.
// Mozna tez ustawic przez strone WWW.
#define DEFAULT_METER_ID ""

// ---------------- Chmura (mini cloud) ----------------
// Rejestracja: POST {CLOUD_URL}/api/device/register (przycisk na stronie WWW), potem pomiary POST-em na
// /api/device/measurements z naglowkami X-Device-ID / X-Device-Key. Adres i interwal moze zmienic odpowiedz rejestracji.
#define CLOUD_URL        "https://cloud.nasdom.tech"
#define CLOUD_INTERVAL_S 60
