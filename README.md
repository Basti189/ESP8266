# ESP8266
 All about ESP8266 / Nodemcu 1.0 Amica V2 (ESP-12E)

# WifiManager
Der WifiManager erstellt einen AccessPoint "Setup". Daraufhin verbindet man sich mit Netzwerk und überträgt die eigentlichen Netzwerkzugangsdaten mit dem Webforumlar unter http://192.168.4.1 oder ruft eine andere http Website auf, man wird automatisch auf das Formular umgeleitet. Sollte alles passen wird ein "Einstellungen erfolgreich übertragen" zurückgegeben und die Verbindung wird mit dem Netzwerk hergestellt und die Konfiguration im Flash Speicher gesichert.

```c++
#include <WifiManager.h>

WifiManager *wifi;

void setup() {
  wifi = new WifiManager();
  wifi->addListener(&callback);
  while (wifi->setup() != 0) {
   //TODO
  }
  //TODO
}

void callback(byte service, byte state, String args[]) {
 //TODO
}
```

# TimeManager
Der TimeManager holt sich die Zeit von einem angegeben NTP-Server und rechnet die Differenz zu den internen Millisekunden aus. Ab diesem Zeitpunkt werden zum Berechnen des Datums und der Uhrzeit nur die interne Millisekunden genutzt. Standartmäßig wird jede Minute die Differenz erneut mit dem NTP-Server abgeglichen. Die Berechnung berücksichtigt die Zeitzone und die Sommer-/Winterzeit, sowie Schaltjahre.

```c++
#include <TimeManager.h>

TimeManager *timeManager;

void setup() {
 timeManager = new TimeManager("de.pool.ntp.org", 1.0, true);
 while(!timeManager->sync()); //Eine Schleife, damit wirklich gewartet wird, bis die Zeit sich synchronisiert hat
}

void loop() {
 timeManager->sync(); // Wird zwar aufgerufen, aber nur alle 60 Sekunden wirklich ausgeführt
 //TODO
}
```
