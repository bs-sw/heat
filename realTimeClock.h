#ifndef __realTimeClock__
#define __realTimeClock__

#define RTC_DAY_BEGIN 630
#define RTC_NIGHT_BEGIN 2100
#define RTC_I2C_ADDRESS 0x68 // I2C Adresse des RTC  DS3231

#define LATER_OPENING_TIME_REACHED_HOURS 12
#define LATITUDE 48.7
#define LONGITUDE 9.2
#define TIMEZONE 1

#include "heat.h"
#include <Arduino.h>
#include <Wire.h>
#include "error.h"

class RealTimeClock {
private:
  static byte bcdToDec(byte val);
  static byte decToBcd(byte val);
  static byte year, month, day, hour, minute, second;
  static byte latestCalculationDay;
  static byte daysInMonth(byte mo, byte y);
  static bool isLeapyear(byte y);
  static bool isSummerTime(byte y, byte mo, byte d, byte h);
  static int sunset;
  static int sunrise;

public:
  static void init();
  static void getTimeString(char timeString[], bool getMESZ);
  static void getDateString(char dateString[]);
  static void getDateTimeString(char dateTimeString[], bool getMESZ);
  static void getYearMonthString(char yearMonthString[]);
  static void refresh();
  static void setTime(byte year, byte month, byte day, byte hour, byte minute);
  static void addSecond();
  static bool isDayLight();
  static bool isNight();
  static int getDayLength();
  static int getMinutesSinceMidnight();
  static int getSunset();
  static int getSunrise();
  static void updateSunriseSunset();
  static bool laterOpeningTimeReached();
  static byte getYear();
  static byte getMonth();
  static byte getDay();
  static byte getHour();
  static byte getMinute();
  static byte getSecond();
};

#endif
