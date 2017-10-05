#include "WifiManager.h"


WifiManager::WifiManager() {
	
}

String WifiManager::getVersion() {
	return this->version;
}

String WifiManager::getIP() {
	return ipAdress;
}

String WifiManager::getSSID() {
	return ssid;
}

void WifiManager::addListener(Callback callback) {
	this->callback = callback;
}

void WifiManager::broadcast(byte service, byte state) {
	String array[] = {};
	broadcast(service, state, array);
}

void WifiManager::broadcast(byte service, byte state, String msg[]) {
	if (callback != nullptr) {
		callback(service, state, msg);
	}
}

uint8_t WifiManager::setup() {
	if (!readConfig()) { //Konfiguration nicht gefunden
		broadcast(0x10, 0x01);
		setupWifi(true);
		setupWebserver(true);
		while (configMode) {
			dnsServer->processNextRequest();
			serverSetup->handleClient();
		}
		delay(500);
		setupWebserver(false);
		setupWifi(false);
		broadcast(0x10, 0x00);
	}
	return connectWifi();
}

/**
 * 0 = OK
 * 1 = Verbindung fehlgeschlagen
 * 2 = Nicht mehr genutzt
 */
int WifiManager::connectWifi() {
	String array[] = {ssid};
	broadcast(0x14, 0x01, array);
	WiFi.mode(WIFI_STA);
	yield();
	WiFi.begin(ssid.c_str(), password.c_str());
	uint8_t count = 0;
	String points = "";
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		count++;
		if (count == 15) {
			broadcast(0x14, 0x00);
			return 1;
		}
		points += ".";
		String array2[] = {points};
		broadcast(0x14, 0x02, array2);
	}
	
	IPAddress lIP = WiFi.localIP();
	ipAdress = String(lIP[0]) + '.' + String(lIP[1]) + '.' + String(lIP[2]) + '.' + String(lIP[3]);
	String array2[] = {ssid, ipAdress};
	broadcast(0x14, 0x03, array2);
	return 0;
}

bool WifiManager::readConfig() {
	SPIFFS.begin();
	File fConfig = SPIFFS.open(configFile, "r");
	if (!fConfig) {
		broadcast(0x13, 0x00);
		return false;
	}
	String result = "";
	uint8_t count = 0;
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
	broadcast(0x13, 0x01);
	return true;
}

void WifiManager::saveConfig() {
	SPIFFS.begin();
	File fConfig = SPIFFS.open(configFile, "w");
	if (!fConfig) {
		broadcast(0x13, 0x02);
		return;
	}
	fConfig.println(ssid);
	fConfig.println(password);
	fConfig.close();
	broadcast(0x13, 0x03);
}

void WifiManager::deleteConfig() {
	SPIFFS.begin();
	if (SPIFFS.exists(configFile)) {
		SPIFFS.remove(configFile);
		broadcast(0x13, 0x04);
	} else {
		broadcast(0x13, 0x05);
	}
}

void WifiManager::setupWifi(bool state) {
	if (state) {
		String genSSID = "ESP-GSP aA1234";//generateSSID();
		String genPassword = "12345678";//generatePassword();
		WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 2558, 255, 0));
		WiFi.softAP(genSSID.c_str(), genPassword.c_str());
		IPAddress apIP = WiFi.softAPIP();
		ipAdress = String(apIP[0]) + '.' + String(apIP[1]) + '.' + String(apIP[2]) + '.' + String(apIP[3]);
		String array[] = {genSSID, genPassword, ipAdress};
		broadcast(0x11, 0x01, array);
	} else {
		WiFi.softAPdisconnect();
		WiFi.disconnect();
		while ( WiFi.status() == WL_CONNECTED ) {
			yield();
		}
		WiFi.mode(WIFI_STA);
		broadcast(0x11, 0x00);
	}
}

String WifiManager::generateSSID() {
	String genSSID = wifiName + " ";
	String letters[26] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"};
	String cLetters[26] = {"A", "B", "C", "D", "E", "F", "g", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};
	genSSID += letters[random(0, 26)];
	genSSID += cLetters[random(0, 26)];
	genSSID += String(random(1000, 10000));
	return genSSID;
}

String WifiManager::generatePassword() {
	return String(random(10000000, 100000000));
}

void WifiManager::reset() {
	deleteConfig();
	WiFi.disconnect();
	while (WiFi.status() == WL_CONNECTED) {
		yield();
	}
	WiFi.mode(WIFI_OFF);
}

void WifiManager::setupWebserver(bool state) {
	if (state) {
		dnsServer = new DNSServer();
		dnsServer->start(DNS_PORT, "*", IPAddress(192, 168, 4, 1));
		serverSetup = new ESP8266WebServer(80);
		serverSetup->on("/", std::bind(&WifiManager::handleRoot, this));
		serverSetup->on("/setup", std::bind(&WifiManager::handleSetup, this));
		serverSetup->onNotFound(std::bind(&WifiManager::handleRoot, this));
		serverSetup->begin();
		broadcast(0x12, 0x01);
	} else {
		serverSetup->stop();
		dnsServer->stop();
		delete serverSetup;
		delete dnsServer;
		broadcast(0x12, 0x00);
	}
}

void WifiManager::handleRoot() {
	String page = FPSTR(HTTP_ROOT);
	serverSetup->sendHeader("Content-Length", String(page.length()));
	serverSetup->send(200, "text/html", page);
}

void WifiManager::handleSetup() {
	ssid = serverSetup->arg("ssid").c_str();
  	password = serverSetup->arg("password").c_str();
	String page = FPSTR(HTTP_OK);
	serverSetup->sendHeader("Content-Length", String(page.length()));
	serverSetup->send(200, "text/html", page);
	saveConfig();
	configMode = false;
}
