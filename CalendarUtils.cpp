#include "Arduino.h"

// calculate day of week from passed date, Sunday = 0, Monday = 1...
byte calcDayOfWeek(int yy, byte mm, byte dd)
{
  int dow = 0;
  if (mm >= 3)
  {
    int z = yy;
    dow = (((23 * mm) / 9) + dd + 4 + yy + (z / 4) - (z / 100) + (z / 400) - 2) % 7;
  }
  else
  {
    int z = yy - 1;
    dow = (((23 * mm) / 9) + dd + 4 + yy + (z / 4) - (z / 100) + (z / 400)) % 7;
  }
  return (byte)dow;
}

// calculate week number from passed date
byte calcWeekNumber(int yy, byte mm, byte dd)
{
  int a = (mm < 3) ? yy - 1 : yy;
  int b = (a / 4) - (a / 100) + (a / 400);
  int c = ((a - 1) / 4) - ((a - 1) / 100) + ((a - 1) / 400);
  int s = b - c;
  int e = (mm < 3) ? 0 : (s + 1);
  int f = (mm < 3) ? dd - 1 + (31 * (mm - 1)) : dd + (((153 * (mm - 3)) + 2) / 5) + 58 + s;
  int g = (a + b) % 7;
  int d = (f + g - e) % 7;
  int n = f + 3 - d;
  int res = 0;
  if (n < 0)
  {
    res = 53 - ((g - s) / 5);
  }
  else if (n > (364 + s))
  {
    res = 1;
  }
  else
  {
    res = (n / 7) + 1;
  }
  return (byte)res;
}

// calculate Daylight saving date for the passed year following EU rules, assuming Winter to Summer remains in March
byte calcWinterSummerDay(int yy)
{
  int r = 31 - calcDayOfWeek(yy, 03, 31);
  return (byte)(r);
}

// calculate Daylight saving date for the passed year following EU rules, assuming Summer to Winter remains in October
byte calcSummerWinterDay(int yy)
{
  int r = 31 - calcDayOfWeek(yy, 10, 31);
  return (byte)(r);
}
//
// END OF FILE
//
