#ifndef WifiManager_h
#define WifiManager_h

#include <WString.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


class WifiManager {
	public:
		WifiManager();
		
		int setupWifiManager();
		int setupWifiManager(int baudrate);
		void reset();
		String getVersion();

	private:
		bool configMode = true;
		String ssid = "";
		String password = "";
		String const configFile = "/wifi/config.txt";
		String const version = "1.0.1";

		ESP8266WebServer *serverSetup;
    
		bool readConfig();
		void saveConfig();
		void deleteConfig();
		void setupWifi(bool);
		void setupWebserver(bool);
		int connectWifi();
};

#endif
