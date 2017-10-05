#ifndef WifiManager_h
#define WifiManager_h

#include "FS.h"
#include <WString.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


const char HTTP_ROOT[] PROGMEM = "<!DOCTYPE html> <html lang=\"de\"> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/> <title>Netzwerkkonfiguration</title> <style> button {  border:0;  border-radius: 4px;  line-height:2.4rem;  font-size:1.2rem;  background-color: #4caf50;  width:100%;  box-sizing: border-box;  -moz-box-sizing: border-box; } input {  padding-top: 12px;  padding-bottom: 12px;  text-indent: 10px;  margin: 8px 0px;  border-style: solid;  border-width: 3px;  border-color: #ff9800;  border-radius: 4px;  font-size:1.2rem;  width:100%;  box-sizing: border-box;  -moz-box-sizing: border-box; } </style> </head> <body style=\"background-color: #262626\">  <form method='post' action='setup'> <input id='ssid' name='ssid' length=32 placeholder='SSID'> <br/> <input id='password' name='password' length=64 type='password' placeholder='Passwort'> <br/> <br/> <button type='submit'>&Uumlbertragen</button> </form> </body> </html>";
const char HTTP_OK[] PROGMEM = "<!DOCTYPE html> <html lang=\"de\"> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/> <title>Ok!</title> <style> label {  line-height:2.4rem;  font-size:2.0rem;  color: #4caf50;  padding: 8px;  width:100%; } </style> </head> <body style=\"background-color: #262626\"> <label>Einstellungen erfolgreich &uumlbertragen!</label> </body> </html>";
typedef void (*Callback) (byte, byte, String[]);

class WifiManager {
	public:		
		WifiManager();
		
		uint8_t setup();
		void addListener(Callback callback);
		void reset();
		String getVersion();
		String getIP();
		String getSSID();

	private:
		bool configMode = true;
		String ssid = "";
		String password = "";
		const String  configFile = "/wifi/config.txt";
		const String  version = "1.3.1";
		const String wifiName = "ESP-GSP";
		const String dnsName = "anzeige";
		String ipAdress = "";
		Callback callback;

		DNSServer *dnsServer;
		const byte DNS_PORT = 53;
		ESP8266WebServer *serverSetup;
		void handleRoot();
		void handleSetup();
    
		bool readConfig();
		void saveConfig();
		void deleteConfig();
		void setupWifi(bool);
		void setupWebserver(bool);
		int connectWifi();
		String generatePassword();
		String generateSSID();
		void broadcast(byte service, byte state);
		void broadcast(byte service, byte state, String args[]);
};

#endif

