# Eksperymentalna integracja TuyaLink z ESP8266

## Cel

Sprawdzić, czy istniejący licznik HAN (NodeMCU ESP8266 + CC1101) może łączyć się bezpośrednio z Tuya Cloud przez Tuya IoT Core SDK, bez dodatkowych modułów i bez naruszania stabilnego firmware.

Gałąź: `experimental/tuya-iot-core-esp8266`

## Stan audytu firmware

- Platforma: PlatformIO, `espressif8266`, `nodemcuv2`, Arduino.
- CPU: 160 MHz.
- CC1101 działa po SPI; nie używa UART.
- UART0 (`Serial`) jest obecnie używany do diagnostyki przy 115200.
- Pętla aplikacji jest jednowątkowa: odczyt CC1101, HTTP server, OTA, mDNS, zapis historii i wysyłka NASDOM.
- Wysyłka NASDOM korzysta z `BearSSL::WiFiClientSecure` i `HTTPClient`; obecnie TLS jest konfigurowany bez weryfikacji certyfikatu.
- Obecna wysyłka HTTPS może blokować pętlę przez około 1–3 sekundy. Integracja Tuya nie może pogorszyć odbioru ramek HAN.
- Dane pomiarowe są już dostępne w strukturze `MeterData` i w funkcji `cloudPayload()`.

## Wnioski z dokumentacji Tuya

Oficjalny Tuya IoT Core SDK jest napisany w C i deklaruje niezależność od systemu/platformy. Wymaga jednak dostarczenia adaptera platformowego:
- alokacja/zwalnianie pamięci i zegary,
- interfejs TLS dla MQTT/HTTP,
- trwały magazyn klucz-wartość.

Dokumentacja SDK pokazuje przykładową implementację TLS opartą o mbedTLS na Linuxie. ESP8266 Arduino używa w tym projekcie BearSSL, więc adapter TLS jest głównym ryzykiem technicznym. Nie zakładać, że SDK skompiluje się bez zmian.

## Ograniczenia i zasady

1. Nie zmieniać pinów ani obsługi CC1101.
2. Nie usuwać ani nie modyfikować istniejącej integracji NASDOM.
3. Nie zmieniać formatu EEPROM `Settings` ani istniejących plików LittleFS.
4. Nie uruchamiać synchronicznych operacji sieciowych Tuya w ścieżce obsługi ramki radiowej.
5. Nie dodawać sekretów Tuya do repozytorium. Dane produktu/urządzenia muszą być lokalną konfiguracją, poza commitem.
6. Nie wgrywać eksperymentalnego firmware na urządzenie, dopóki nie przejdzie kompilacji i analizy pamięci.
7. Każdy etap musi dać się wyłączyć flagą kompilacji, aby domyślny build pozostał bez zmian.

## Kolejne kroki

1. Sprawdzić zależności i wymagane interfejsy SDK oraz możliwość pominięcia jego wbudowanego mbedTLS.
2. Przygotować minimalny adapter ESP8266: system, zegar, KV i TLS.
3. Zbudować osobny przykład/test bez włączania go do głównego firmware.
4. Zmierzyć flash, statyczny RAM i wolny heap podczas Wi-Fi/TLS.
5. Dopiero po pozytywnych wynikach zaplanować nieblokującą obsługę Tuya w głównej pętli.

## Ważne: TuyaLink a Smart Life

Połączenie z Tuya IoT Core/TuyaLink nie oznacza automatycznie, że urządzenie będzie dostępne jako standardowe urządzenie konsumenckie w aplikacji Smart Life. Przed implementacją funkcji należy potwierdzić obsługiwany typ produktu, sposób aktywacji i dostępność urządzenia w aplikacji.
