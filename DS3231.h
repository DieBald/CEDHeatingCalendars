//
//    FILE: DS3231.H
//  AUTHOR: Didier Chauveaux
//    DATE: 21/04/2019
// VERSION: 0.1.0
// PURPOSE: I2C DS3231 library for Arduino
//     URL: 
//
// HISTORY:
// see DS3231.cpp file
//

#ifndef _DS3231_H
#define _DS3231_H

#include <Wire.h>
#include "Arduino.h"
#define DS3231_I2C_ADDRESS 0x68

class DS3231
{
public:
  explicit DS3231(const byte deviceAddress);
  float readDS3231Temp();
  void setDS3231datetime(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year);
  void setDS3231time(byte second, byte minute, byte hour);
  void setDS3231DoW(byte dayOfWeek);
  void setDS3231Hour(byte hour);
  void readDS3231time(byte * second,byte * minute,byte * hour,byte * dayOfWeek,byte * dayOfMonth,byte * month,byte * year);
private:
  byte decToBcd(byte val);
  byte bcdToDec(byte val);
  byte _address;
};

#endif
//
// END OF FILE
//
