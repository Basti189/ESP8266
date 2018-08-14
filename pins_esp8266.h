/*
  pins_esp8266.h - Pin definition functions for Nodemcu v2.0 (ESP-12E)
*/

#ifndef pins_esp8266_h
#define pins_esp8266_h

static const uint8_t D3 = 0;
static const uint8_t D4 = 2; //LED on ESP8266 Modul - Needed for upload BIn to flash
static const uint8_t RX = 3; //Serial console input, can be used but you can't send any command to the board
static const uint8_t D2 = 4;
static const uint8_t D1 = 5;
static const uint8_t SD2 = 9;
static const uint8_t SD3 = 10;
static const uint8_t D6 = 12;
static const uint8_t D7 = 13;
static const uint8_t D5 = 14;
static const uint8_t D8 = 15;
static const uint8_t D0 = 16; //LED_BUILTIN - LED on the Nodemcu Board
static const uint8_t A0 = 17; //analogRead() only, range 0.1V - 1.0V

#endif /* pins_esp8266_h */
