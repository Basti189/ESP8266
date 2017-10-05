/*

   NTPtime for ESP8266
   This routine gets the unixtime from a NTP server and adjusts it to the time zone and the
   Middle European summer time if requested

  Author: Sebastian Wolf V1.1 2017-10-05

  Based on work from John Lassen: http://www.john-lassen.de/index.php/projects/esp-8266-arduino-ide-webconfig
  Based on work from Andreas Spiess <Sensorslot>: https://github.com/SensorsIot/NTPtimeESP

*/

#ifndef TimeManager_h
#define TimeManager_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

struct strDateTime {
	byte hour;
	byte minute;
	byte second;
	int year;
	byte month;
	byte day;
	byte dayofWeek;
	unsigned long timeInMillis;
	unsigned long diffTime;
	boolean valid;
};

class TimeManager {

	public:
		TimeManager(String ntpServer, float timeZone, boolean daylightSaving);
		~TimeManager();
		bool sync();
		unsigned long getLastSync();
		strDateTime getLocalTime();
		String getDateAsString();
		String getTimeAsString();
		String getTimeStampAsString();
		void setSyncInterval(unsigned long syncInterval);  // in seconds
		bool setSendInterval(unsigned long sendInterval);  // in seconds
		bool setRecvTimeout(unsigned long recvTimeout);    // in seconds
		
	private:
		WiFiUDP UDPNTPClient;
		bool _sendPhase;
		unsigned long _sentTime;
		unsigned long _sendInterval;
		unsigned long _recvTimeout;
		unsigned long _difference;
		unsigned long _lastSync;
		unsigned long _syncInterval;

		const int NTP_PACKET_SIZE = 48;
		byte _packetBuffer[48];
		float _timeZone = 0.0;
		boolean _daylightSaving = false;
		String _NTPserver = "";

		strDateTime getNTPtime(float timeZone, boolean DaylightSaving);
		strDateTime convertUnixTimestamp(unsigned long tempTimeStamp);
		boolean summerTime(unsigned long timeStamp );
		boolean daylightSavingTime(unsigned long timeStamp);
		unsigned long adjustTimeZone(unsigned long timeStamp, float timeZone, byte DaylightSavingSaving);	
};


#endif
