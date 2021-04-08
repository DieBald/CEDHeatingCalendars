#include "DS3231.h"
#include <Wire.h>
//#include "CEDVars.h"

DS3231::DS3231(const byte deviceAddress)
{
    _address = deviceAddress;
}

float DS3231::readDS3231Temp() {
 
  Wire.beginTransmission(_address);
  Wire.write(0x11); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom((int)_address, (int)2);
  // request 2 bytes of data from DS3231 starting from register 11h
  byte TempHiR = Wire.read();             // 11
  byte TempLoR = Wire.read();             // 12
  float Temp = TempHiR;
  switch (TempLoR) {
  case 0x40: Temp = Temp + 0.25; break;
  case 0x80: Temp = Temp + 0.5; break;
  case 0xc0: Temp = Temp + 0.75; break;
  }
  return Temp;
}

void DS3231::setDS3231datetime(byte second, byte minute, byte hour, byte dayOfWeek, byte
  dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(_address);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}

void DS3231::setDS3231time(byte second, byte minute, byte hour)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(_address);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.endTransmission();
}

void DS3231::setDS3231DoW(byte dayOfWeek)
{
  // sets day of week data to DS3231
  Wire.beginTransmission(_address);
  Wire.write(3); // set next input to start at the DaoyOfWeek register
  byte adayOfWeek=decToBcd(dayOfWeek);
  Wire.write(adayOfWeek);
  Wire.endTransmission();
}

void DS3231::setDS3231Hour(byte hour)
{
  // sets hour data to DS3231
  Wire.beginTransmission(_address);
  Wire.write(2); // set next input to start at the Hour register
  byte ahour=decToBcd(hour);
  Wire.write(ahour);
  Wire.endTransmission();
}

void DS3231::setDS3231Minute(byte minute)
{
    // sets hour data to DS3231
    Wire.beginTransmission(_address);
    Wire.write(1); // set next input to start at the Minute register
    byte aminute = decToBcd(minute);
    Wire.write(aminute);
    Wire.endTransmission();
}
void DS3231::setDS3231Second(byte second)
{
    // sets hour data to DS3231
    Wire.beginTransmission(_address);
    Wire.write(0); // set next input to start at the Second register
    byte asecond = decToBcd(second);
    Wire.write(asecond);
    Wire.endTransmission();
}

void DS3231::setDS3231DoM(byte dayOfMonth)
{
    // sets hour data to DS3231
    Wire.beginTransmission(_address);
    Wire.write(4); // set next input to start at the Hour register
    byte adayOfMonth = decToBcd(dayOfMonth);
    Wire.write(adayOfMonth);
    Wire.endTransmission();
}

void DS3231::setDS3231Month(byte month)
{
    // sets hour data to DS3231
    Wire.beginTransmission(_address);
    Wire.write(5); // set next input to start at the Hour register
    byte amonth = decToBcd(month);
    Wire.write(month);
    Wire.endTransmission();
}

void DS3231::setDS3231Year(byte year)
{
    // sets hour data to DS3231
    Wire.beginTransmission(_address);
    Wire.write(6); // set next input to start at the Hour register
    byte ayear = decToBcd(year);
    Wire.write(ayear);
    Wire.endTransmission();
}

// Convert normal decimal numbers to binary coded decimal
byte DS3231::decToBcd(byte val)
{
  return((val / 10 * 16) + (val % 10));
}
// Convert binary coded decimal to normal decimal numbers
byte DS3231::bcdToDec(byte val)
{
  return((val / 16 * 10) + (val % 16));
}

void DS3231::readDS3231time(byte * second,
  byte * minute,
  byte * hour,
  byte * dayOfWeek,
  byte * dayOfMonth,
  byte * month,
  byte * year)
{
  Wire.beginTransmission(_address);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom((int)_address, (int)7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f); // 00
  *minute = bcdToDec(Wire.read());        // 01
  *hour = bcdToDec(Wire.read() & 0x3f);   // 02
  *dayOfWeek = bcdToDec(Wire.read());     // 03
  *dayOfMonth = bcdToDec(Wire.read());    // 04
  *month = bcdToDec(Wire.read());         // 05
  *year = bcdToDec(Wire.read());          // 06
}
//
// END OF FILE
//
