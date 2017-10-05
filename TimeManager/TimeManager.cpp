/*
NTP
This routine gets the unixtime from a NTP server and adjusts it to the time zone and the
Middle European summer time if requested

Author: Sebastian Wolf V1.1 2017-10-05

Based on work from John Lassen: http://www.john-lassen.de/index.php/projects/esp-8266-arduino-ide-webconfig
Based on work from Sensorslot: https://github.com/SensorsIot/NTPtimeESP

*/
#include "TimeManager.h"

#define LEAP_YEAR(Y) ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

#define SEC_TO_MS             1000
#define RECV_TIMEOUT_DEFAULT  1       // 1 second
#define SEND_INTRVAL_DEFAULT  1       // 1 second
#define SYNC_INTERVAL         60      // 60 seconds
#define MAX_SEND_INTERVAL     60      // 60 seconds
#define MAC_RECV_TIMEOUT      60      // 60 seconds


static const uint8_t _monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static const String _monthDaysName[] = {"Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"};
//static const String _dayOfWeekName[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};

TimeManager::TimeManager(String ntpServer, float timeZone, boolean daylightSaving) {
	_NTPserver = ntpServer;
	_timeZone = timeZone;
	_daylightSaving = daylightSaving;
	_sendPhase = true;
	_sentTime  = 0;
	_lastSync = 0UL;
	_syncInterval = SYNC_INTERVAL * SEC_TO_MS;
	_sendInterval = SEND_INTRVAL_DEFAULT * SEC_TO_MS;
	_recvTimeout = RECV_TIMEOUT_DEFAULT * SEC_TO_MS;
}

TimeManager::~TimeManager() {
	UDPNTPClient.stop();
}

bool TimeManager::setSendInterval(unsigned long sendInterval) {
	if(sendInterval <= MAX_SEND_INTERVAL) {
		_sendInterval = sendInterval * SEC_TO_MS;
		return true;
	}
	return false;
}

bool TimeManager::setRecvTimeout(unsigned long recvTimeout) {
	if(recvTimeout <= MAC_RECV_TIMEOUT) {
		_recvTimeout = recvTimeout * SEC_TO_MS;
		return true;
	}
	return false;  
}

void TimeManager::setSyncInterval(unsigned long syncInterval) {
	_syncInterval = syncInterval * SEC_TO_MS;
}

bool TimeManager::sync() {
	if ((millis() >= _lastSync + _syncInterval) || _lastSync == 0UL) {
		strDateTime dateTime = getNTPtime(_timeZone, _daylightSaving);
		if (dateTime.valid) {
			_lastSync = millis();
			return true;
		}
		return false;
	}
	return false;
}

unsigned long TimeManager::getLastSync() {
	return _lastSync;
}

strDateTime TimeManager::getLocalTime() {
	return convertUnixTimestamp(millis()/1000 + _difference);
}

String TimeManager::getDateAsString() {
	strDateTime dateTime = convertUnixTimestamp(millis()/1000 + _difference);
	String result = String(dateTime.day) + ". ";
	result += _monthDaysName[dateTime.month-1] + " " + String(dateTime.year);
	return result;
}

String TimeManager::getTimeAsString() {
	strDateTime dateTime = convertUnixTimestamp(millis()/1000 + _difference);
	String result = String(dateTime.hour);
	if (dateTime.hour < 10) {
		result = "0" + String(dateTime.hour);
	}
	result += ":";
	if (dateTime.minute < 10) {
		result += "0" + String(dateTime.minute);
	} else {
		result += String(dateTime.minute);
	}
	return result;
}

String TimeManager::getTimeStampAsString() {
	strDateTime dateTime = convertUnixTimestamp(millis()/1000 + _difference);
	String result = String(dateTime.day);
	if (dateTime.day < 10) {
		result = "0" + String(dateTime.day);
	}
	result += ".";
	if (dateTime.month < 10) {
		result += "0" + String(dateTime.month);
	} else {
		result += String(dateTime.month);
	}
	result += "." + String(dateTime.year);
	return result;
}

// Converts a unix time stamp to a strDateTime structure
strDateTime TimeManager::convertUnixTimestamp(unsigned long tempTimeStamp) {
	strDateTime tempDateTime;
	uint8_t year, month, monthLength;
	uint32_t time;
	unsigned long days;

	tempDateTime.timeInMillis = tempTimeStamp;
	time = (uint32_t) tempTimeStamp;
	tempDateTime.second = time % 60;
	time /= 60; // now it is minutes
	tempDateTime.minute = time % 60;
	time /= 60; // now it is hours
	tempDateTime.hour = time % 24;
	time /= 24; // now it is _days
	tempDateTime.dayofWeek = ((time + 4) % 7) + 1;  // Sunday is day 1

	year = 0;
	days = 0;
	while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
		year++;
	}
	tempDateTime.year = year; // year is offset from 1970

	days -= LEAP_YEAR(year) ? 366 : 365;
	time  -= days; // now it is days in this year, starting at 0

	days = 0;
	month = 0;
	monthLength = 0;
	for (month = 0; month < 12; month++) {
		if (month == 1) { // february
			if (LEAP_YEAR(year)) {
				monthLength = 29;
			} else {
				monthLength = 28;
			}
		} else {
			monthLength = _monthDays[month];
		}

		if (time >= monthLength) {
			time -= monthLength;
		} else {
			break;
		}
	}
	tempDateTime.month = month + 1;  // jan is month 1
	tempDateTime.day = time + 1;     // day of month
	tempDateTime.year += 1970;

	return tempDateTime;
}


//
// Summertime calculates the daylight saving time for middle Europe. Input: Unixtime in UTC
//
boolean TimeManager::summerTime(unsigned long timeStamp ) {

	strDateTime  tempDateTime;
	tempDateTime = convertUnixTimestamp(timeStamp);

	if (tempDateTime.month < 3 || tempDateTime.month > 10) {
		return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
	}
	if (tempDateTime.month > 3 && tempDateTime.month < 10) {
		return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
	}
	if (tempDateTime.month == 3 && (tempDateTime.hour + 24 * tempDateTime.day) >= (3 +  24 * (31 - (5 * tempDateTime.year / 4 + 4) % 7)) || tempDateTime.month == 10 && (tempDateTime.hour + 24 * tempDateTime.day) < (3 +  24 * (31 - (5 * tempDateTime.year / 4 + 1) % 7))) {
		return true;
	} else {
		return false;
	}
}

boolean TimeManager::daylightSavingTime(unsigned long timeStamp) {

	strDateTime  tempDateTime;
	tempDateTime = convertUnixTimestamp(timeStamp);

	if (tempDateTime.month < 3 || tempDateTime.month > 11) {
		return false;  //January, february, and december are out.
	}
	if (tempDateTime.month > 3 && tempDateTime.month < 11) {
		return true;   //April to October are in
	}
	int previousSunday = tempDateTime.day - (tempDateTime.dayofWeek - 1);  // dow Sunday input was 1,
	// need it to be Sunday = 0. If 1st of month = Sunday, previousSunday=1-0=1
	//int previousSunday = day - (dow-1);
	// -------------------- March ---------------------------------------
	//In march, we are DST if our previous Sunday was = to or after the 8th.
	if (tempDateTime.month == 3) {  // in march, if previous Sunday is after the 8th, is DST
		// unless Sunday and hour < 2am
		if (previousSunday >= 8) { // Sunday = 1
			// return true if day > 14 or (dow == 1 and hour >= 2)
			return ((tempDateTime.day > 14) || 
			((tempDateTime.dayofWeek == 1 && tempDateTime.hour >= 2) || tempDateTime.dayofWeek > 1));
		} // end if ( previousSunday >= 8 && _dateTime.dayofWeek > 0 )
		else {
			// previousSunday has to be < 8 to get here
			//return (previousSunday < 8 && (tempDateTime.dayofWeek - 1) = 0 && tempDateTime.hour >= 2)
			return false;
		} // end else
	} // end if (_tempDateTime.month == 3 )

	// ------------------------------- November -------------------------------

	// gets here only if month = November
	//In november we must be before the first Sunday to be dst.
	//That means the previous Sunday must be before the 2nd.
	if (previousSunday < 1) {
		// is not true for Sunday after 2am or any day after 1st Sunday any time
		return ((tempDateTime.dayofWeek == 1 && tempDateTime.hour < 2) || (tempDateTime.dayofWeek > 1));
		//return true;
	} // end if (previousSunday < 1)
	else {
		// return false unless after first wk and dow = Sunday and hour < 2
		return (tempDateTime.day <8 && tempDateTime.dayofWeek == 1 && tempDateTime.hour < 2);
	}  // end else
} // end boolean NTPtime::daylightSavingTime(unsigned long _timeStamp)


unsigned long TimeManager::adjustTimeZone(unsigned long timeStamp, float timeZone, byte DayLightSaving) {
	strDateTime tempDateTime;
	timeStamp += (unsigned long)(timeZone *  3600.0); // adjust timezone
	if (DayLightSaving ==1 && summerTime(timeStamp)) timeStamp += 3600; // European Summer time
	if (DayLightSaving ==2 && daylightSavingTime(timeStamp)) timeStamp += 3600; // US daylight time
	return timeStamp;
}

// time zone is the difference to UTC in hours
// if _isDayLightSaving is true, time will be adjusted accordingly
// Use returned time only after checking "ret.valid" flag

strDateTime TimeManager::getNTPtime(float timeZone, boolean DayLightSaving) {
	int cb;
	strDateTime dateTime;
	unsigned long unixTime = 0;
	dateTime.valid = false;
	unsigned long currentTimeStamp;
	unsigned long espTimeStamp;

	if (_sendPhase) {
		if (_sentTime && ((millis() - _sentTime) < _sendInterval)) {
			return dateTime;
		}

		_sendPhase = false;
		UDPNTPClient.begin(1337); // Port for NTP receive

		memset(_packetBuffer, 0, NTP_PACKET_SIZE);
		_packetBuffer[0] = 0b11100011; // LI, Version, Mode
		_packetBuffer[1] = 0;          // Stratum, or type of clock
		_packetBuffer[2] = 6;          // Polling Interval
		_packetBuffer[3] = 0xEC;       // Peer Clock Precision
		_packetBuffer[12] = 49;
		_packetBuffer[13] = 0x4E;
		_packetBuffer[14] = 49;
		_packetBuffer[15] = 52;
		UDPNTPClient.beginPacket(_NTPserver.c_str(), 123);
		UDPNTPClient.write(_packetBuffer, NTP_PACKET_SIZE);
		UDPNTPClient.endPacket();

		_sentTime = millis();
	} else {
		cb = UDPNTPClient.parsePacket();
		if (cb == 0) {
			if ((millis() - _sentTime) > _recvTimeout) {
				_sendPhase = true;
				_sentTime = 0;
			}
		} else {
			espTimeStamp = millis()/1000;
			UDPNTPClient.read(_packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
			unsigned long highWord = word(_packetBuffer[40], _packetBuffer[41]);
			unsigned long lowWord = word(_packetBuffer[42], _packetBuffer[43]);
			unsigned long secsSince1900 = highWord << 16 | lowWord;
			const unsigned long seventyYears = 2208988800UL;
			unixTime = secsSince1900 - seventyYears;
			if (unixTime > 0) {
				currentTimeStamp = adjustTimeZone(unixTime, timeZone, DayLightSaving);
				dateTime = convertUnixTimestamp(currentTimeStamp);
				dateTime.difference = currentTimeStamp - espTimeStamp;
				_difference = currentTimeStamp - espTimeStamp;
				dateTime.valid = true;
			} else {
				dateTime.valid = false;
			}
			_sendPhase = true;
		}
	}
	return dateTime;
}
