# ESP8266
 All about ESP8266 / Nodemcu 2.0 (ESP-12E)

# WifiManager
Der WifiManager erstellt einen AccessPoint "Setup". Daraufhin verbindet man sich mit Netzwerk und überträgt die eigentlichen Netzwerkzugangsdaten per Aufruf: 192.168.4.1:8080/setup?ssid=SSID&password=PASSWORT. Sollte alles passen wird ein "OK" zurückgegeben und die Verbindung wird mit dem Netzwerk hergestellt und die Konfiguration im Flash Speicher gesichert.

```c++
#include <WifiManager.h>

WifiManager manager;

void setup() {
  int errorCode;
  while((errorCode = manager.setupWifiManager()) != 0) {
    //TODO
  }
  //TODO
}
```
