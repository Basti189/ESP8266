#include "WifiManager.h"
#include "FS.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>


WifiManager::WifiManager() {
	
}

String WifiManager::getVersion() {
  return this->version;
}

int WifiManager::setupWifiManager() {
	return setupWifiManager(115200);
}

int WifiManager::setupWifiManager(int baudrate) {
	Serial.begin(baudrate);
	if (!readConfig()) { //Konfiguration nicht gefunden
		setupWifi(true);
		setupWebserver(true);
		while (configMode) {
			serverSetup->handleClient();
		}
		delay(500);
		setupWebserver(false);
		setupWifi(false);
	}
	return connectWifi();
}

/**
 * 0 = OK
 * 1 = Verbindung fehlgeschlagen
 * 2 = Nicht mehr genutzt
 */
int WifiManager::connectWifi() {
	Serial.print("[WifiManager] Verbindung zum Netzwerk \"" + ssid + "\" wird hergestellt");
	char cssid[50];
	ssid.toCharArray(cssid, 50);
	char cpassword[100];
	password.toCharArray(cpassword, 100);
	WiFi.mode(WIFI_STA);
	yield();
	WiFi.begin(cssid, cpassword);
	int count = 0;
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		Serial.print(".");
		count++;
		if (count == 15) {
			Serial.println("\n[WifiManager] Verbindung konnte nicht hergestellt werden  ...\n");
			return 1;
		}
	}
	Serial.println("\n[WifiManager] Verbindung zum Netzwerk \"" + ssid + "\" hergestellt");
	Serial.print("[WifiManager] IP-Adresse: ");
	Serial.println(WiFi.localIP());
	return 0;
}

bool WifiManager::readConfig() {
	Serial.println("[WifiManager] Lade Konfiguration...");
	SPIFFS.begin();
	File fConfig = SPIFFS.open(configFile, "r");
	if (!fConfig) {
		Serial.println("[WifiManager] Keine Konfiguration gefunden!");
		return false;
	}
	String result = "";
	int count = 0;
	while(fConfig.available()) {
		char c = fConfig.read();
		if (c != '\n') {
			result += c;
		} else {
			result = result.substring(0, result.length() - 1);
		if (count == 0) {
			ssid = result;
		} else if (count == 1) {
			password = result;
		}
		result = "";
		count++;
		}
	}
	fConfig.close();
	Serial.println("[WifiManager] Konfiguration erfolgreich geladen");
	return true;
}

void WifiManager::saveConfig() {
	Serial.println("[WifiManager] Speichere Konfiguration...");
	SPIFFS.begin();
	File fConfig = SPIFFS.open(configFile, "w");
	if (!fConfig) {
		Serial.println("Speichern fehlgeschlagen!");
		return;
	}
	fConfig.println(ssid);
	fConfig.println(password);
	fConfig.close();
	Serial.println("[WifiManager] Konfiguration gespeichert");
}

void WifiManager::deleteConfig() {
	Serial.println("[WifiManager] Lösche Konfiguration...");
	SPIFFS.begin();
	if (SPIFFS.exists(configFile)) {
		SPIFFS.remove(configFile);
		Serial.println("[WifiManager] Konfiguration gelöscht");
	} else {
		Serial.println("[WifiManager] Löschen der Konfiguration fehlgeschlagen!");
	}
	delay(500);
}

void WifiManager::setupWifi(bool state) {
	if (state) {
		Serial.println("[WifiManager] Zugriffspunkt wird eingerichtet...");
		WiFi.softAP("Setup", "");
		Serial.println("[WifiManager] Zugriffspunkt eingerichtet");
		IPAddress apIP = WiFi.softAPIP();
		Serial.print("[WifiManager] IP-Adresse: ");
		Serial.println(apIP);
	} else {
		Serial.println("[WifiManager] Zugriffspunkt wird abgeschaltet...");
		WiFi.softAPdisconnect();
		WiFi.disconnect();
		if ( WiFi.status() != WL_CONNECTED ) {
			delay (250);
		}
		WiFi.mode(WIFI_STA);
		Serial.println("[WifiManager] Zugriffspunkt abgeschaltet");
	}
}

void WifiManager::reset() {
	deleteConfig();
}

void WifiManager::setupWebserver(bool state) {
	if (state) {
		Serial.println("[WifiManager] Webserver wird eingerichtet...");
		serverSetup = new ESP8266WebServer(8080);
		serverSetup->on("/setup", [=]() {
			for (uint8_t i = 0; i < serverSetup->args(); i++) {
				if (serverSetup->argName(i) == "ssid") {
					ssid = serverSetup->arg(i);
				} else if (serverSetup->argName(i) == "password") {
					password = serverSetup->arg(i);
				}
			}
			if (ssid != "" && password != "") {
				serverSetup->send(200, "text/plain", "OK");
				Serial.println("[WifiManager] Konfiguration:");
				Serial.println("  -> SSID: " + ssid);
				Serial.println("  -> Passwort: " + password);
				saveConfig();
				configMode = false;
				delay(200);
			} else {
				serverSetup->send(200, "text/plain", "error");
			}
		});
		serverSetup->begin();
		Serial.println("[WifiManager] Webserver eingerichtet");
	} else {
		Serial.println("[WifiManager] Webserver wird abgeschaltet...");
		serverSetup->stop();
		delete serverSetup;
		Serial.println("[WifiManager] Webserver abgeschaltet");
	}
}
