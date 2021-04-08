// Created on 21/04/2019
// by Didier Chauveaux

#ifndef _CALENDARUTILS_H
#define _CALENDARUTILS_H

#include "Arduino.h"

// calculate day of week from passed date, Sunday = 0, Monday = 1...
byte calcDayOfWeek(int yy, byte mm, byte dd);

// calculate week number from passed date
byte calcWeekNumber(int yy, byte mm, byte dd);

// calculate Daylight saving date for the passed year following EU rules, assuming Winter to Summer remains in March
byte calcWinterSummerDay(int yy);

// calculate Daylight saving date for the passed year following EU rules, assuming Summer to Winter remains in October
byte calcSummerWinterDay(int yy);


#endif
//
// END OF FILE
//
