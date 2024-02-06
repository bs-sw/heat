#include "realTimeClock.h"

byte RealTimeClock::year, RealTimeClock::month, RealTimeClock::day, RealTimeClock::hour, RealTimeClock::minute, RealTimeClock::second;
byte RealTimeClock::latestCalculationDay;
int RealTimeClock::sunset;
int RealTimeClock::sunrise;

void RealTimeClock::init() {
  Wire.begin();
  refresh();
}

byte RealTimeClock::getYear() {return year;}
byte RealTimeClock::getMonth() {return month;}
byte RealTimeClock::getDay() {return day;}
byte RealTimeClock::getHour() {return hour;}
byte RealTimeClock::getMinute() {return minute;}
byte RealTimeClock::getSecond() {return second;}

void RealTimeClock::getTimeString(char timeString[], bool getMESZ){
  byte hourOffset = 0;
  if (getMESZ) {if (isSummerTime(year, month, day, hour)) hourOffset = 1;}
  timeString[0] = ((hour + hourOffset)/ 10) + '0';  
  timeString[1] = ((hour + hourOffset)% 10) + '0';
  timeString[2] = ':';
  timeString[3] = (minute / 10) + '0';  
  timeString[4] = (minute % 10) + '0';
  timeString[5] = ':';
  timeString[6] = (second / 10) + '0';  
  timeString[7] = (second % 10) + '0';
  timeString[8] = '\0';
}

void RealTimeClock::getDateString(char dateString[]){
  dateString[0] = (day / 10) + '0'; 
  dateString[1] = (day % 10) + '0';
  dateString[2] = '.';
  dateString[3] = (month / 10) + '0'; 
  dateString[4] = (month % 10) + '0';
  dateString[5] = '.';
  dateString[6] = '2';
  dateString[7] = '0';
  dateString[8] = (year / 10) + '0';  
  dateString[9] = (year % 10) + '0';
  dateString[10] = '\0';
}

void RealTimeClock::getYearMonthString(char yearMonthString[]){
  yearMonthString[0] = '2';
  yearMonthString[1] = '0';
  yearMonthString[2] = (year / 10) + '0';  
  yearMonthString[3] = (year % 10) + '0';
  yearMonthString[4] = '-';
  yearMonthString[5] = (month / 10) + '0'; 
  yearMonthString[6] = (month % 10) + '0';
  yearMonthString[7] = '\0';
}

void RealTimeClock::getDateTimeString(char dateTimeString[], bool getMESZ){
  getDateString(dateTimeString);
  dateTimeString[10] = ';';
  getTimeString(dateTimeString + 11, getMESZ);
}

byte RealTimeClock::bcdToDec(byte val){
  return ( (val/16*10) + (val%16) );
}

byte RealTimeClock::decToBcd(byte val){
  return ( (val/10*16) + (val%10) );
}

void RealTimeClock::refresh(){
  Wire.beginTransmission(RTC_I2C_ADDRESS); //Start connection to address 0x68
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(RTC_I2C_ADDRESS, 7);
  if (Wire.available()) {
    second = bcdToDec(Wire.read() & 0x7f);
    minute = bcdToDec(Wire.read()); 
    hour = bcdToDec(Wire.read() & 0x3f); 
    Wire.read(); //week day not required
    day = bcdToDec(Wire.read());
    month = bcdToDec(Wire.read());
    year = bcdToDec(Wire.read());
  } else {
    Error::setError(ERROR_DATE_TIME_COULDNT_BE_READ);
    day = 15;
    month = 4;
    year = 20;
  }
}

bool RealTimeClock::isNight() {
  #if defined(IS_NIGHT)
  return false;
  #endif
  #if defined(IS_DAY)
  return false;
  #endif
  int time = ((int) hour) * 100 + minute;
  return (time > RTC_NIGHT_BEGIN || time < RTC_DAY_BEGIN);
}

int RealTimeClock::getMinutesSinceMidnight() {
  return ((int) hour) * 60 + minute;
}

bool RealTimeClock::laterOpeningTimeReached() {
  return hour >= LATER_OPENING_TIME_REACHED_HOURS;
}

void RealTimeClock::updateSunriseSunset(){
  if (true || day != latestCalculationDay) {
    float latitudeRad = LATITUDE * PI / 180;
    float dayNumber = (month - 1)*30 + month/3 + day;
    float timeEquation = -0.171 * sin(0.0337 * dayNumber + 0.465) - 0.1299 * sin(0.01787 * dayNumber - 0.168);
    float sunDeclination = 0.4095 * sin(0.016906 * (dayNumber - 80.086));
    float timeDifference = acos((sin(-0.0145) - sin(latitudeRad) * sin(sunDeclination)) / (cos(latitudeRad) * cos(sunDeclination))) * 12 / PI; 
    sunrise = (int) ((12 - timeDifference - timeEquation - LONGITUDE/15 + TIMEZONE) * 60);
    sunset = (int) ((12 + timeDifference - timeEquation - LONGITUDE/15 + TIMEZONE) * 60);
    latestCalculationDay = day;
  }
}

//returns the time in minutes from midnight to sunrise
int RealTimeClock::getSunrise() {
  updateSunriseSunset();
  return sunrise;
}

//returns the time in minutes from midnight to sunset
int RealTimeClock::getSunset() {
  updateSunriseSunset();
  return sunset;
}

int RealTimeClock::getDayLength() {
  return getSunset() - getSunrise();
}

bool RealTimeClock::isDayLight() {
  return (getMinutesSinceMidnight() > getSunrise() || getMinutesSinceMidnight() < getSunset());
}

void RealTimeClock::setTime(byte year, byte month, byte day, byte hour, byte minute) {
  byte second;
  bool fail = false;
  
  if (!(year <= 99)) fail = true; 
  if (!(month >= 1 && month <= 12)) fail = true;
  if (!(day >= 1 && day <= 31)) fail = true; 
  if (!(hour <= 23)) fail = true;
  if (!(minute <= 59)) fail = true; 
  second = 0;

  if(!fail) {
    Wire.beginTransmission(RTC_I2C_ADDRESS);
    Wire.write(0); // 0 activates the RTC module
    Wire.write(decToBcd(second));    
    Wire.write(decToBcd(minute));
    Wire.write(decToBcd(hour));                                  
    Wire.write(decToBcd(0)); //week day not required
    Wire.write(decToBcd(day));
    Wire.write(decToBcd(month));
    Wire.write(decToBcd(year));  
    Wire.endTransmission();  
  } else {
    Error::setError(ERROR_DATE_TIME_COULDNT_BE_SET);
  }
}

void RealTimeClock::addSecond() {
  second++;
  if(second >= 60)
  {
    second = 0;
    minute++;

    if(minute >= 60)
    {
      minute = 0;
      hour++;

      if(hour >= 24)
      {
        hour = 0;
        day++;

        if(day >= daysInMonth(month, year))
        {
          day = 1;
          month++;

          if(month > 12)
          {
            month = 1;
            year++;
          }
        }
      }
    }
  }
}

byte RealTimeClock::daysInMonth(byte mo, byte y) {
  byte days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  byte daysInMonth = days[mo];
  if(mo == 2 && isLeapyear(y))
  {
    return 29;
  } 
  return daysInMonth;
}

bool RealTimeClock::isLeapyear(byte y) {
  return (y & 0x03) == 0;
}

bool RealTimeClock::isSummerTime(byte y, byte mo, byte d, byte h)
{
 if (mo<3 || mo>10) return false;
 if (mo>3 && mo<10) return true;
 if ((mo==3 && (h + 24 * d) >= (2 + 24*(31 - (5 * y /4 + 4) % 7))) || (mo==10 && (h + 24 * d)<(2 + 24*(31 - (5 * y /4 + 1) % 7))))
   return true;
 else
   return false;
}
