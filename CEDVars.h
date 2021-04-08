// Created on 21/04/2019
// by Didier Chauveaux

#ifndef _CEDVARS_H
#define _CEDVARS_H

#include "Arduino.h"
#include "EEPROM_I2C.h"
#include "DS3231.h"
#include "PCF8574.h"
#include <Ethernet.h>

PROGMEM const word calNameOffset=32;
PROGMEM const byte keyUP = 0;
PROGMEM const byte keyDOWN = 1;
PROGMEM const byte keyLEFT = 2;
PROGMEM const byte keyRIGHT = 3;
PROGMEM const byte keyMIDDLE = 4;
PROGMEM const byte keySET = 5;
PROGMEM const byte keyRESET = 6;
PROGMEM const byte keyIR = 7;
PROGMEM const word calendarOffset = 256;
PROGMEM const word calendarDaySize = 6;
PROGMEM const word calendarWeekSize = 42;
PROGMEM const word calendarYearSize = 2226;

const byte reqSize[10]={1,22,1,4,4,4,4,4,1,18};
const byte inputPin[8]={2,3,4,5,6,7,8,9};
const char inputSym[8]={'U','D','L','R','M','S','r','I'};
const word calNameAdd[8]={calNameOffset+0,calNameOffset+16,calNameOffset+32,calNameOffset+48,calNameOffset+64,calNameOffset+80,calNameOffset+96,calNameOffset+112};


// vars for day light saving management
static byte dlsWS_Day=0;
static byte dlsWS_Month=0;
static byte dlsWS_Hour=0;
static byte dlsWS_Minute=0;
static bool dlsWS_Active=0;
static byte dlsSW_Day=0;
static byte dlsSW_Month=0; 
static byte dlsSW_Hour=0;
static byte dlsSW_Minute=0;
static bool dlsSW_Active=0;



// global vars
static byte second;
static byte minute;
static byte hour;
static byte dayOfWeek;
static byte dayOfMonth;
static byte month;
static byte year;
static byte DoW;
static byte weekNumber;
static byte lcdActivePage=0;
static byte lcdActiveLine=0;
static bool lcdCursor=false;
static bool lcdDone=false;

// calendars related vars
static char calName[16];
//static calName calNames[8];
static bool calBypassed[8];
static bool calBypassVal[8];
static bool calCurrentVal[8];

static byte dOutput01=0;
static bool outState[8];
static byte dInput01=0;
static bool inpState[8];
static bool minpState[8];
static float curTemp=0.0;
static unsigned long cycleCurrent=0;
static unsigned long cycleMini=10000000;
static unsigned long cycleMaxi=0;


static bool demoOutputs=false;

static unsigned long reqCounter=0;
static unsigned long reqCtrWrite=0;
static byte calBuffer[136];
static byte reqAnswerLen;

// Initialize eeprom
static EEPROM_I2C eeprom;
//Initialize extended IO PCF8574
static PCF8574 PCF_01(0x20);
// Initialize RTC DS3231
static DS3231 rtc(DS3231_I2C_ADDRESS);
// Initialize net communication server
//static CEDNetServer cednet;


static byte mac[]={0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
static IPAddress ip(192, 168, 1, 177);
static EthernetServer server(3088);
static EthernetClient clients[8]; // W5500 permits up to 8 clients

    static byte clientInProgress; // the client whose request is being managed
    static bool clientProcessRequest;


#endif

//
// End of File
//
