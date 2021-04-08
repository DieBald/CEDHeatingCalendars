/*
  C3D

  Controls electric heating through Pilot Wire
  Permits 8 areas through 8 calendars
  Integrates EEPROM memory for calendar storage, RTC for maintaining time in case of power outage,
  a network card for communication with computer (PC or Android device), a LCD screen and joystick
  for local display and simple changes. Uses IO expansion for digital outpouts

  Circuit:
   I2C EEPROM AT24C256
   I2C DS3231 RTC Clock
   I2C Digital IO expansion shield (PCF8574)
   I2C LCD2004 using MCP23017 shield
   SPI Ethernet shield (W5500) attached to pins 10, 11, 12, 13

  created 20 Apr 2019
  by Didier Chauveaux

*/

#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>
#include "DS3231.h"
#include "PCF8574.h"
#include "CalendarUtils.h"
#include "CEDVars.h"
#include "EEPROM_I2C.h"

// TODO: manage reinit of network card through output

// define the speed of I2C bus
#define I2C_SPEED 400000L // Higher speed does not improve performance most likely because of LCD
//Initialise the LCD
#define I2C_ADDR          0x27        //Define I2C Address where the PCF8574A is
LiquidCrystal_I2C lcd(I2C_ADDR, 20, 4);


byte curDayOfWeek = 0xFF;
byte mdOutput01 = dOutput01;
byte mdInput01 = dInput01;
// for display
float mTemperature;
byte mDayOfWeek;
unsigned long cycleVal = 0;
bool firstLoop = true;
bool clientReused = false;

// vars for day light saving management
bool dlsWS_ToDo, dlsSW_ToDo, dlsWS_Done, dlsSW_Done;


// test counters: dubug only, remove for definitive version
word tstCtr01 = 0;
word tstCtr02 = 0;
word tstCtr03 = 0;
word tstCtr04 = 0;

byte prgB[8];
word maddr[8];

void getConfigFromEeprom() {
  eeprom.request(0, 10);
  dlsWS_Day = Wire.read();
  dlsWS_Month = Wire.read();
  dlsWS_Hour = Wire.read();
  dlsWS_Minute = Wire.read();
  dlsSW_Day = Wire.read();
  dlsSW_Month = Wire.read();
  dlsSW_Hour = Wire.read();
  dlsSW_Minute = Wire.read();

  dlsWS_Active = Wire.read();
  dlsSW_Active = Wire.read();
  /*
    // read calendar names
    for (byte n=0;n<8;n++){
     eeprom.request(32+(n*16), 16);
     for (word c=0;c<16;c++) {
      calNames[n][c]=Wire.read();
     }
    }
  */

}


// display Daylight Saving data
void displayDaylightSaving() {
  lcd.setCursor(0, 0);
  lcd.print("HE:");
  lcd.print((dlsWS_Active) ? 'A' : '_');
  if (dlsWS_Day < 10) lcd.print("0");
  lcd.print(dlsWS_Day, DEC);
  lcd.print("/");
  if (dlsWS_Month < 10) lcd.print("0");
  lcd.print(dlsWS_Month, DEC);
  lcd.print(" EH:");
  lcd.print((dlsSW_Active) ? 'A' : '_');
  if (dlsSW_Day < 10) lcd.print("0");
  lcd.print(dlsSW_Day, DEC);
  lcd.print("/");
  if (dlsSW_Month < 10) lcd.print("0");
  lcd.print(dlsSW_Month, DEC);
}

// calculate day light saving dates for this year
void manageNewYear() {
  dlsWS_Day = calcWinterSummerDay(2000 + year);
  dlsWS_Month = 3;
  dlsWS_Hour = 3;
  dlsWS_Minute = 0;
  dlsSW_Day = calcSummerWinterDay(2000 + year);
  dlsSW_Month = 10;
  dlsSW_Hour = 4;
  dlsSW_Minute = 0;
  tstCtr01++;
}

// manage possible changes on new day
void manageNewDay() {
  if ((month == dlsWS_Month) && (dayOfMonth == dlsWS_Day) && (dlsWS_Active) && (!dlsWS_ToDo)) {
    // it's day to advance hour of 1
    dlsWS_ToDo = true;
    dlsWS_Done = false;
  }
  else if ((month == dlsSW_Month) && (dayOfMonth == dlsSW_Day) && (dlsWS_Active) && (!dlsWS_ToDo)) {
    // it's day to delay hour of 1
    dlsSW_ToDo = true;
    dlsSW_Done = false;
  }
  else { // Today, nothing to do
    dlsWS_ToDo = false;
    dlsSW_ToDo = false;
    dlsWS_Done = false;
    dlsSW_Done = false;
  }
  tstCtr02++;
}

// manage possible changes on new day
void manageDayLightSaving() {
  if ((hour == dlsWS_Hour) && (minute == dlsWS_Minute) && (dlsWS_ToDo) && (!dlsWS_Done) && (dlsWS_Active)) {
    // it's time to advance hour of 1
    rtc.setDS3231Hour(hour + 1);
    dlsWS_Done = true;
  }
  if ((hour == dlsSW_Hour) && (minute == dlsSW_Minute) && (dlsSW_ToDo) && (!dlsSW_Done) && (dlsSW_Active)) {
    // it's time to delay hour of 1
    rtc.setDS3231Hour(hour - 1);
    dlsSW_Done = true;
  }
}

void lcdDisplayDateTime() {
  lcd.setCursor(0, 3);
  if (dayOfMonth < 10)
  {
    lcd.print("0");
  }
  lcd.print(dayOfMonth, DEC);
  lcd.print("/");
  if (month < 10)
  {
    lcd.print("0");
  }
  lcd.print(month, DEC);
  lcd.print("/20");
  lcd.print(year, DEC);
  // display time
  lcd.print("  ");
  if (hour < 10)
  {
    lcd.print("0");
  }
  lcd.print(hour, DEC);
  // convert the byte variable to a decimal number when displayed
  lcd.print(":");
  if (minute < 10)
  {
    lcd.print("0");
  }
  lcd.print(minute, DEC);
  lcd.print(":");
  if (second < 10)
  {
    lcd.print("0");
  }
  lcd.print(second, DEC);
}

void lcdDisplayNetRequestCounter() {
  lcd.setCursor(0, 1);
  lcd.print("Net Req.:");
  if (reqCounter < 10) lcd.print("         ");
  else if (reqCounter < 100) lcd.print("        ");
  else if (reqCounter < 1000) lcd.print("       ");
  else if (reqCounter < 10000) lcd.print("      ");
  else if (reqCounter < 100000) lcd.print("     ");
  else if (reqCounter < 1000000) lcd.print("    ");
  else if (reqCounter < 10000000) lcd.print("   ");
  else if (reqCounter < 100000000) lcd.print("  ");
  else if (reqCounter < 1000000000) lcd.print(" ");
  lcd.print(reqCounter, DEC);
  lcd.setCursor(0, 2);
  // display if a client was reused
  if (clientReused) {
    lcd.print('R');
  }
  else {
    lcd.print('-');
  }
  lcd.print( "  Write:");
  if (reqCtrWrite < 10) lcd.print("         ");
  else if (reqCtrWrite < 100) lcd.print("        ");
  else if (reqCtrWrite < 1000) lcd.print("       ");
  else if (reqCtrWrite < 100000) lcd.print("     ");
  else if (reqCtrWrite < 1000000) lcd.print("    ");
  else if (reqCtrWrite < 10000000) lcd.print("   ");
  else if (reqCtrWrite < 100000000) lcd.print("  ");
  else if (reqCtrWrite < 1000000000) lcd.print(" ");
  lcd.print(reqCtrWrite, DEC);

}

void lcdDisplayTestCounter() {
  lcd.setCursor(0, 2);
  lcd.print(tstCtr01, HEX);
  lcd.print(" ");
  lcd.print(tstCtr02, HEX);
  lcd.print(" ");
  lcd.print(tstCtr03, HEX);
  lcd.print(" ");
  lcd.print(tstCtr04, HEX);
  lcd.print("  ");
}

void lcdDisplayCycleDuration() {
  lcd.setCursor(0, 0);
  lcd.print("Cycle curr:");
  if (cycleCurrent < 10) lcd.print("   ");
  else if (cycleCurrent < 100) lcd.print("  ");
  else if (cycleCurrent < 1000) lcd.print(" ");
  lcd.print(cycleCurrent, DEC);
  lcd.setCursor(0, 1);
  lcd.print("Cycle mini:");
  if (cycleMini < 10) lcd.print("   ");
  else if (cycleMini < 100) lcd.print("  ");
  else if (cycleMini < 1000) lcd.print(" ");
  lcd.print(cycleMini, DEC);
  lcd.setCursor(0, 2);
  lcd.print("Cycle maxi:");
  if (cycleMaxi < 10) lcd.print("   ");
  else if (cycleMaxi < 100) lcd.print("  ");
  else if (cycleMaxi < 1000) lcd.print(" ");
  lcd.print(cycleMaxi, DEC);
}
void lcdDisplayCalendar() {
  lcd.setCursor(0, 0);
  lcd.print("Cycle curr:");

}

void lcdDisplayOutputMode() {
  lcd.setCursor(0, 0);
  lcd.print("Output demo mode:");
  lcd.print((demoOutputs) ? " On" : "Off");
}
// update datetime global vars
void updateDateTime() {
  // retrieve data from DS3231
  byte asecond, aminute, ahour, adayOfWeek, adayOfMonth, amonth, ayear;
  rtc.readDS3231time(&asecond, &aminute, &ahour, &adayOfWeek, &adayOfMonth, &amonth, &ayear);
  if (adayOfMonth != dayOfMonth) {
    weekNumber = calcWeekNumber(ayear + 2000, amonth, adayOfMonth);
    byte aDoW = calcDayOfWeek(ayear + 2000, amonth, adayOfMonth) + 1;
    if (aDoW != dayOfWeek) {
      dayOfWeek = aDoW;
      rtc.setDS3231DoW(dayOfWeek);
    }
  }
  bool aNewYear = ((adayOfMonth == 1) || (amonth == 1) || (ayear != year));
  bool aNewDay = ((adayOfMonth != dayOfMonth) || (amonth != month) || (ayear != year));

  if ((asecond != second) || (aminute != minute) || (ahour != hour) || (adayOfWeek != dayOfWeek) || (adayOfMonth != dayOfMonth) || (amonth != month) || (ayear != year)) {
    second = asecond;
    minute = aminute;
    hour = ahour;
    dayOfWeek = adayOfWeek;
    dayOfMonth = adayOfMonth;
    month = amonth;
    year = ayear;
    DoW = ((dayOfWeek != 1) ? (dayOfWeek - 1) : 7) - 1; // Sunday is 1 for RTC, we want to start the week on Monday=0
  }
  else return;
  if (aNewYear) manageNewYear();
  if (aNewDay) manageNewDay();
  manageDayLightSaving();
}

void updateTemperature() {
  curTemp = rtc.readDS3231Temp();
  //if (temp==curTemp) return;
}

void updatePage1() {
  //lcd.clear();
  lcdDisplayDateTime();
  //if(curDayOfWeek==dayOfWeek) return;
  curDayOfWeek = dayOfWeek;
  if ((lcdDone) || (mDayOfWeek != dayOfWeek)) {
    mDayOfWeek = dayOfWeek;
    lcd.setCursor(0, 0);
    switch (dayOfWeek) {
      case 1:
        lcd.print("Dimanche");
        break;
      case 2:
        lcd.print("Lundi   ");
        break;
      case 3:
        lcd.print("Mardi   ");
        break;
      case 4:
        lcd.print("Mercredi");
        break;
      case 5:
        lcd.print("Jeudi   ");
        break;
      case 6:
        lcd.print("Vendredi");
        break;
      case 7:
        lcd.print("Samedi  ");
        break;
    }
    lcd.print(" S");
    if (weekNumber < 10) lcd.print("0");
    lcd.print(weekNumber, DEC);
  }
  if ((lcdDone) || (mTemperature != curTemp)) {
    mTemperature = curTemp;
    lcd.setCursor(13, 0);
    lcd.print(curTemp);
    lcd.print(char(223));
    lcd.print("C");
  }
  //if ((lcdDone)||(??))
  {
    lcd.setCursor(0, 1);
    for (int i = 0; i < 8; i++) {
      if (i == 4) {
        lcd.setCursor(0, 2);
        //break;
      }

      lcd.print("C");
      lcd.print(i + 1, DEC);
      /*
        if (prgB[i]<16) lcd.print('0');

        lcd.print(prgB[i],HEX);
        lcd.print(' ');
      */
      lcd.print((calBypassed[i]) ? "F" : "_");
      lcd.print((calCurrentVal[i]) ? "M " : "a ");

    }
    /*
      lcd.setCursor(0,2);
      for (int i=0;i<4;i++){
        lcd.print(maddr[i],DEC);
        lcd.print(' ');

      }
    */
  }
  lcdDone = false;
}

void updatePage2() {
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("I/O num 01234567");
  lcd.setCursor(0, 1);
  lcd.print("Outputs ");
  //lcd.print(dOutput01,BIN);
  for (int i = 7; i >= 0; i--) lcd.print((outState[i]) ? '1' : '_');
  lcd.print(" ");
  if (dOutput01 < 16)lcd.print("0");
  lcd.print(dOutput01, HEX);
  lcd.setCursor(0, 2);
  lcd.print(" Inputs ");
  for (int i = 7; i >= 0; i--) lcd.print((inpState[i]) ? '1' : '_');
  lcd.print(" ");
  if (dInput01 < 16)lcd.print("0");
  lcd.print(dInput01, HEX);
  //for (int i=7;i>=0;i--) lcd.print((inpState[i])?'1':'_');
  lcd.setCursor(8, 3);
  for (int i = 7; i >= 0; i--) lcd.print((inpState[i]) ? inputSym[i] : '_');
}

void updatePage3() {
  //lcd.clear();
  lcdDisplayDateTime();
  displayDaylightSaving();
  lcdDisplayNetRequestCounter();
}

void updatePage4() {
  //lcd.clear();
  lcdDisplayDateTime();
  //displayDaylightSaving();
  lcdDisplayCycleDuration();
}


void keyboardPage3() {
  if (lcdActivePage != 2) return;
  if ((!minpState[keyRESET]) && (inpState[keyRESET])) {
    //page 3 displays network connection, RESEET is doing soft reset of network
    cednetStart();
  }
}

void keyboardPage4() {
  if (lcdActivePage != 3) return;
  if ((!minpState[keyRESET]) && (inpState[keyRESET])) {
    //page 3 display cycle duration
    cycleMini = 9999;
    cycleMaxi = 0;
  }
}

void updatePage5() {
  //lcd.clear();
  lcdDisplayDateTime();
  //displayDaylightSaving();
  lcdDisplayOutputMode();
  switch (lcdActiveLine) {
    case 0 : {
        lcd.setCursor(17, 0);
        //lcd.cursor();
        lcdCursor = true;
        break;
      }
  }
}

void keyboardPage5() {
  if (lcdActivePage != 4) return;
  if ((!minpState[keyRESET]) && (inpState[keyRESET])) {
    //page 4 swap demoOutputs
    demoOutputs = false;
  }
  if ((!minpState[keySET]) && (inpState[keySET])) {
    //page 4 swap demoOutputs
    demoOutputs = true;
  }
}

void updatePage6() {

  for (int ln = 0; ln < 3; ln++) {
    lcd.setCursor(0, ln);
    lcd.print(lcdActiveLine + ln + 1);
    lcd.print(' ');
    eeprom.request(calNameAdd[lcdActiveLine + ln], 16);
    for (int i = 0; i < 16; i++) calName[i] = Wire.read();
    for (int i = 0; i < 16; i++) lcd.print(calName[i]);
    lcd.print(' ');
    lcd.print((calCurrentVal[lcdActiveLine + ln]) ? 'M' : '_');
  }
  lcdDisplayDateTime();
}

void keyboardPage6() {
  if (lcdActivePage != 5) return;
  if ((!minpState[keyUP]) && (inpState[keyUP])) {
    if (lcdActiveLine == 0) lcdActiveLine = 5;
    else lcdActiveLine--;
  }
  else if ((!minpState[keyDOWN]) && (inpState[keyDOWN])) {
    if (lcdActiveLine == 5) lcdActiveLine = 0;
    else lcdActiveLine++;
  }
}

void updatePage7() {
  lcdDisplayTestCounter();
}

void keyboardPage7() {
  if (lcdActivePage != 6) return;
  if ((!minpState[keyRESET]) && (inpState[keyRESET])) {
    //page 6 display test counters
    tstCtr01 = 0;
    tstCtr02 = 0;
    tstCtr03 = 0;
    tstCtr04 = 0;
  }
}

void updateInformation() {
  switch (lcdActivePage) {
    case 0 :
      updatePage1();
      break;
    case 1 :
      updatePage2();
      break;
    case 2 :
      updatePage3();
      break;
    case 3 :
      updatePage4();
      break;
    case 4 :
      updatePage5();
      break;
    case 5 :
      updatePage6();
      break;
    case 6 :
      updatePage7();
      break;
    default :
      updatePage1();
  }
}

void manageKeyboard() {
  if ((!minpState[keyLEFT]) && (inpState[keyLEFT])) {
    if (lcdActivePage == 0) lcdActivePage = 5;
    else lcdActivePage--;
    lcdActiveLine = 0;
    //if (lcdCursor) lcd.noCursor();
    lcdDone = true;
    lcd.clear();
    return;
  }
  if ((!minpState[keyRIGHT]) && (inpState[keyRIGHT])) {
    if (lcdActivePage == 5) lcdActivePage = 0;
    else lcdActivePage++;
    lcdActiveLine = 0;
    //if (lcdCursor) lcd.noCursor();
    lcdDone = true;
    lcd.clear();
    return;
  }
  if (lcdActivePage == 2) keyboardPage3();
  if (lcdActivePage == 3) keyboardPage4();
  if (lcdActivePage == 4) keyboardPage5();
  if (lcdActivePage == 5) keyboardPage6();
  if (lcdActivePage == 6) keyboardPage7();
}

void updateOutputs() {
  if (dOutput01 == mdOutput01) return;
  mdOutput01 = dOutput01;
  PCF_01.write8(dOutput01 ^ 0xFF);
  for (int i = 0; i < 8; i++) {
    outState[i] = bitRead(dOutput01, i);
  }
}
void updateInputs() {
  byte asyn = 0;
  for (int i = 0; i < 8; i++) {
    inpState[i] = !digitalRead(inputPin[i]); // inputs are high by default
    if (inpState[i]) asyn |= B00000001 << i;
  }
  dInput01 = asyn;
}



unsigned long next100Millis;
unsigned long nextSecond;

void testDisplay(int p) {
  if (p == 0) {
    lcd.setCursor(0, 0);
    for (int i = 32; i < 52; i++) {
      lcd.print(char(i));
    }
    lcd.setCursor(0, 1);
    for (int i = 52; i < 72; i++) {
      lcd.print(char(i));
    }
    lcd.setCursor(0, 2);
    for (int i = 72; i < 92; i++) {
      lcd.print(char(i));
    }
    lcd.setCursor(0, 3);
    for (int i = 92; i < 112; i++) {
      lcd.print(char(i));
    }
  }
  if (p == 1) {
    lcd.setCursor(0, 0);
    for (int i = 112; i < 132; i++) {
      lcd.print(char(i));
    }
    lcd.setCursor(0, 1);
    for (int i = 132; i < 152; i++) {
      lcd.print(char(i));
    }
    lcd.setCursor(0, 2);
    for (int i = 152; i < 172; i++) {
      lcd.print(char(i));
    }
    lcd.setCursor(0, 3);
    for (int i = 172; i < 192; i++) {
      lcd.print(char(i));
    }
  }
  if (p == 2) {
    lcd.setCursor(0, 0);
    for (int i = 192; i < 212; i++) {
      lcd.print(char(i));
    }
    lcd.setCursor(0, 1);
    for (int i = 212; i < 232; i++) {
      lcd.print(char(i));
    }
    lcd.setCursor(0, 2);
    for (int i = 232; i < 252; i++) {
      lcd.print(char(i));
    }
    lcd.setCursor(0, 3);
    for (int i = 252; i < 256; i++) {
      lcd.print(char(i));
    }
  }

}

void setup() {
  //Serial.begin(115200);
  cednetStart();
  next100Millis = millis() + 100;
  nextSecond = next100Millis;
  dayOfWeek = 0xFF;
  Wire.begin();
  Wire.setClock(I2C_SPEED); // check if OK in future DS3231 is given for 400kHz
  lcd.init();
  lcd.clear();
  lcd.setBacklight(HIGH);
  //lcd.blink();
  // initialize inputs
  for (int i = 0; i < 8; i++) pinMode(inputPin[i], INPUT_PULLUP);
  getConfigFromEeprom();

  //PCF_01.begin();
  //testDisplay(1);
}

bool ledDir = true;
bool seqTic = false;
byte ledTic = 0;

void demoOutput() {
  if (!seqTic) {
    if (ledDir) {
      dOutput01 = dOutput01 << 1;
    }
    else {
      dOutput01 = dOutput01 >> 1;
    }
  }
  if (!dOutput01) {
    if (!seqTic) ledTic = 0; // on démarre
    seqTic = true;
  }
  if (seqTic) {
    if (!dOutput01) {
      if (ledDir) {
        dOutput01 = B10000000;
      }
      else {
        dOutput01 = B00000001;
      }
    }
    else dOutput01 = 0;
    ledTic++;
    if (ledTic > 6) {
      ledDir = !ledDir;
      seqTic = false;
      ledTic = 0;
    }
  }
}

void manageCalendars() {
  // calculate relative byte address function of the date and time
  // apply week
  word aaddr = ((weekNumber - 1) * calendarWeekSize);
  // apply day of week
  aaddr = aaddr + (DoW * calendarDaySize);
  // apply time: one byte is 4 hours (2 half an hour per hour)
  aaddr = aaddr + (hour / 4);
  // calculate bit position within retrieved byte
  byte theBit = (((hour % 4)) * 2) + (minute / 30);
  byte theMask = B00000001 << theBit;
  // now apply offset in eeprom
  aaddr = aaddr + calendarOffset;

  byte prgByte;
  bool heat;
  for (int i = 0; i < 8; i++) {
    maddr[i] = aaddr;
    prgByte = eeprom.get(aaddr);
    prgB[i] = prgByte;
    aaddr = aaddr + calendarYearSize; // set for next calendar
    heat = (prgByte & theMask) != 0;

    if (calBypassed[i]) {
      if (calBypassVal[i] == heat) {
        calBypassed[i] = false;
      }
      else {
        heat = calBypassVal[i];
      }
    }
    calCurrentVal[i] = heat;
  }
  // apply calculated values to outputs if not in demo mode :)
  if (!demoOutputs) {
    dOutput01 = 0;
    for (int i = 0; i < 8; i++) {
      outState[i] = !calCurrentVal[i]; // to give heat, pilot wire is 0
      dOutput01 = dOutput01 | ((outState[i]) ? B00000001 << i : 0);
    }
  }
}


void loop() {
  if (firstLoop) {
    cycleVal = millis();
  }

  // Update inputs
  updateInputs();
  // check every 1000ms
  if (nextSecond <= millis()) {
    nextSecond = millis() + 1000;
  }
  cednetListen();
  // check every 100ms
  if (next100Millis <= millis()) {
    next100Millis = millis() + 50; // changed from 100 to 50 since there's nothing better to do
    updateDateTime();
    updateTemperature();

    manageKeyboard();

    updateInformation();
    if (demoOutputs) demoOutput();
    manageCalendars();

    for (int i = 0; i < 8; i++) minpState[i] = inpState[i];
  }


  updateOutputs();
  cycleCurrent = millis() - cycleVal;
  cycleVal = millis();
  cycleMini = min(cycleCurrent, cycleMini);
  cycleMaxi = max(cycleCurrent, cycleMaxi);
  firstLoop = false;
}


void cednetStart() {


  Ethernet.init(10);  // Most Arduino shields
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  // Check for Ethernet hardware present
  if (EthernetHardwareStatus() == EthernetNoHardware) return;
  // if (LinkStatus() == LinkOFF) // Ethernet cable is not connected
  // reasonably all clients are dead
  for (byte i = 0; i < 8; i++) if (clients[i]) clients[i].stop();
  clientInProgress = 0;
  clientProcessRequest = false;
  // start the server
  server.begin();
}

void cednetReset(EthernetClient client) {
  clientProcessRequest = false;
  clientInProgress = 0;
  client.stop();
}

void cednetListen() {
  //    Serial.print(" H");
  //  Serial.println(hour,DEC);

  // check status and give a chance to reconnect.
  // it seems (with W5500) that not cable connected is bringing EthernetNoHardware
  if (Ethernet.hardwareStatus() == EthernetNoHardware) cednetStart();
  if (Ethernet.hardwareStatus() == EthernetNoHardware) return; // well, nothing eventually

  // listen for incoming clients
  EthernetClient client = server.accept(); // does not require client to send anything
  if (client) {
    //Serial.println("Client connected");
    for (byte i = 0; i < 8; i++) {
      if (!clients[i]) {
        clients[i] = client;
        break;
      }

    }
  }

  // check for incoming data from next client
  if (!clientProcessRequest) {
    // check from next after clientInProgress, gives a chance to next before coming back to first
    for (byte i = clientInProgress + 1; i < 8; i++) {
      if (clients[i] && clients[i].available() > 0) {
        // read incoming data from the client
        calBuffer[0] = clients[i].read();
        clientInProgress = i;
        clientProcessRequest = true;
        break;
      }
    }
    // alright, let's see for otherones
    if (!clientProcessRequest) {
      clientInProgress = 0xFF;
      for (byte i = 0; i < 8; i++) {
        if (clients[i] && clients[i].available() > 0) {
          // read incoming data from the client
          calBuffer[0] = clients[i].read();
          clientInProgress = i;
          clientProcessRequest = true;
          break;
        } // if
      } // for
    } // if
  } // if not clientrequest
  if (clientProcessRequest) {
    //Serial.print("A request is available, nbr ");
    //Serial.println(calBuffer[0],DEC);
    cednetProcessClientRequest(); // there is something to do
  }
  // stop any clients which disconnected
  for (byte i = 0; i < 8; i++) {
    if (clients[i])
      if (!clients[i].connected()) {
        // WARNING check clients[i] is coming back to null after stop
        clients[i].stop();
        // clients[i] = (EthernetClient)(0);
        // it might be client instance can be reused by server!!??
      }
  }
  return;// 0;
}


bool cednetpopulateBuffer(const byte aLen) {
  byte aPos = 1; // start from 1 since 0 is already there
  while (clients[clientInProgress].available() > 0) {
    calBuffer[aPos] = clients[clientInProgress].read();
    aPos++;
  }
  if (calBuffer[0] == 7) {
    byte l = (byte)((1 + calBuffer[3] - calBuffer[2]) * calendarWeekSize);
    return (aPos == (aLen + l));
  }
  return (aPos == aLen);
}

bool cednetanswerClient() {
  if (reqAnswerLen == 0) return true;
  return (clients[clientInProgress].write(calBuffer, reqAnswerLen) == reqAnswerLen);
}

void cednetProcessClientRequest() {
  // check function number that was read
  if (calBuffer[0] >= sizeof(reqSize)) {
    // client made a mistake, max supported function number is 7
    cednetReset(clients[clientInProgress]);  // assume it is unwilled connection
  }
  // manage operation dependent on function number
  // expected client request length depends on function number
  // populate calBuffer with client data
  if (cednetpopulateBuffer(reqSize[calBuffer[0]])) {
    //Serial.println("buffer was populated successfully");
    switch (calBuffer[0]) {
      case 0 :
        // prepare answer
        calBuffer[1] = 8; // number of available calendars
        calBuffer[2] = byte(sizeof(calBuffer));
        calBuffer[3] = 0; // unused
        calBuffer[4] = hour;
        calBuffer[5] = minute;
        calBuffer[6] = second;
        calBuffer[7] = dayOfWeek;
        calBuffer[8] = weekNumber;
        calBuffer[9] = dayOfMonth;
        calBuffer[10] = month;
        calBuffer[11] = year;
        calBuffer[12] = dlsWS_Active;
        calBuffer[13] = dlsWS_Hour;
        calBuffer[14] = dlsWS_Minute;
        calBuffer[15] = dlsWS_Day;
        calBuffer[16] = dlsWS_Month;
        calBuffer[17] = dlsSW_Active;
        calBuffer[18] = dlsSW_Hour;
        calBuffer[19] = dlsSW_Minute;
        calBuffer[20] = dlsSW_Day;
        calBuffer[21] = dlsSW_Month;
        reqCounter++;
        reqAnswerLen = 22;
        clientProcessRequest = false;
        break;
      case 1 :
        // write request: handle demand
        rtc.setDS3231datetime(calBuffer[6], calBuffer[5], calBuffer[4], calcDayOfWeek(calBuffer[11], calBuffer[10], calBuffer[9]), calBuffer[9], calBuffer[10], calBuffer[11]);
        // write daylight saving active to eeprom
        dlsWS_Active = calBuffer[12];
        dlsSW_Active = calBuffer[17];
        calBuffer[13] = dlsSW_Active; // reuse...
        eeprom.writePage(8, calBuffer + 12, 2);
        // prepare answer
        calBuffer[1] = 0xFF; // OK, few reason not to succeed.
        reqCounter++;
        reqCtrWrite++;
        reqAnswerLen = 2;
        clientProcessRequest = false;
        break;
      case 2 :
        // prepare answer
        calBuffer[1] = 8; // number of available calendars
        for (int i = 0; i < 8; i++) {
          calBuffer[2 + i * 3] = calBypassed[i];
          calBuffer[3 + i * 3] = calBypassVal[i];
          calBuffer[4 + i * 3] = calCurrentVal[i];
        }
        reqCounter++;
        reqAnswerLen = 26;
        clientProcessRequest = false;
        break;
      case 3 : {
          // write request: handle demand
          byte acalNumber = calBuffer[1];
          bool acalBypassed = calBuffer[2];
          bool acalBypassVal = calBuffer[3];
          byte aRes;
          aRes = 0xFF;
          if (acalNumber > 8) {
            aRes = 0;
          }
          else {
            calBypassed[acalNumber] = acalBypassed;
            calBypassVal[acalNumber] = acalBypassVal;
          }
          // prepare answer
          calBuffer[1] = aRes; // result
          reqCounter++;
          reqCtrWrite++;
          reqAnswerLen = 2;
          clientProcessRequest = false;
        }
        break;
      case 4 : {
          // prepare answer
          byte asubFunc = calBuffer[1];
          byte anbIO = calBuffer[2];
          byte anbByte = calBuffer[3];
          //byte aRnbByte=0;
          if (asubFunc == 0) {
            calBuffer[4 + 0] = dOutput01;
            reqAnswerLen = 5;
          }
          if (asubFunc == 1) {
            calBuffer[4 + 0] = dInput01;
            reqAnswerLen = 5;
          }
          if (asubFunc == 3) {
            float* f = &curTemp;
            void* v = f;
            unsigned long* l;
            l = v;
            calBuffer[4 + 0] = (byte)(*l >> 24);
            calBuffer[5 + 0] = (byte)((*l >> 16) & 0xFF);
            calBuffer[6 + 0] = (byte)((*l >> 8) & 0xFF);
            calBuffer[7 + 0] = (byte)(*l & 0xFF);
            reqAnswerLen = 8;
          }
          /*
            anbByte=max(4,anbByte);
            for (int i=aRnbByte;i<anbByte;i++){
            calBuffer[4+i]=0;
            }
          */
          reqCounter++;

          clientProcessRequest = false;
        }
        break;
      case 5 : {
          // prepare answer
          byte asubFunc = calBuffer[1];
          byte anumIO = calBuffer[2];
          byte avalIO = calBuffer[3];
          if ((asubFunc == 0) && (anumIO < 8)) {
            if (avalIO) bitSet(dOutput01, anumIO);
            else bitClear(dOutput01, anumIO);
          }
          calBuffer[1] = 0xFF;
          reqCounter++;
          reqCtrWrite++;
          reqAnswerLen = 2;
          clientProcessRequest = false;
        }
        break;
      case 6 : {
          // prepare answer
          byte acalNbr = calBuffer[1];
          byte afirstWeek = calBuffer[2];
          byte alastWeek = calBuffer[3];
          byte aRes = 0xFF;
          if (alastWeek < afirstWeek) {
            // not managed
            aRes = 0;
          }
          else if ((alastWeek - afirstWeek) >= 3) {
            // not managed because of buffer size
            aRes = 0;
          }
          else {
            // it is genuine request
            word aaddr = (calendarYearSize * acalNbr) + ((afirstWeek - 1) * calendarWeekSize);
            word asize = ((1 + alastWeek - afirstWeek) * calendarWeekSize);
            // apply offset in eeprom
            aaddr = aaddr + calendarOffset;
            eeprom.readPage(aaddr, (byte *)&calBuffer[4], asize);
            reqCounter++;
            reqAnswerLen = 4 + asize;
            clientProcessRequest = false;
          }
        }
        break;
      case 7 : {
          // prepare answer
          byte acalNbr = calBuffer[1];
          byte afirstWeek = calBuffer[2];
          byte alastWeek = calBuffer[3];
          byte aRes = 0xFF;
          if (alastWeek < afirstWeek) {
            // not managed
            aRes = 0;
          }
          else if ((alastWeek - afirstWeek) >= 3) {
            // not managed because of buffer size
            aRes = 0;
          }
          else {
            // it is genuine request
            word aaddr = (calendarYearSize * acalNbr) + ((afirstWeek - 1) * calendarWeekSize);
            word asize = ((1 + alastWeek - afirstWeek) * calendarWeekSize);
            // apply offset in eeprom
            aaddr = aaddr + calendarOffset;
            tstCtr02 = asize;
            eeprom.writePage(aaddr, (byte *)&calBuffer[4], asize);
            reqCounter++;
            reqCtrWrite++;
            reqAnswerLen = 2;
            clientProcessRequest = false;
          }
        }
        break;
      case 8 : {
          // prepare answer
          calBuffer[1] = 8;
          eeprom.readPage(calNameOffset, (byte *)&calBuffer[2], 16 * 8);
          reqCounter++;
          reqAnswerLen = 2 + 128;
          clientProcessRequest = false;
        }
        break;
      case 9 : {
          // prepare answer
          byte acalNbr = calBuffer[1];
          for (int i = 0; i < 16; i++) calName[i] = calBuffer[2 + i];
          eeprom.writePage(calNameOffset + acalNbr * 16, (byte *)&calName, 16);
          reqCounter++;
          reqCtrWrite++;
          reqAnswerLen = 2;
          clientProcessRequest = false;
        }
        break;

    }
    cednetanswerClient();
  }
  else {
    cednetReset(clients[clientInProgress]); // assume it is unwilled connection
  }

}

//
// END OF FILE
//
