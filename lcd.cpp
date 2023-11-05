#include "lcd.h"

LiquidCrystal_I2C Lcd::llcd(LCD_ADDRESS, 16, 4);

char Lcd::timeString[9];
char Lcd::dateString[11];
bool Lcd::toggle;
char Lcd::line[4][17];

void Lcd::init() {
  llcd.init();
}

void Lcd::backlight() {
  llcd.backlight();
}

void Lcd::toggleValues() {
  toggle = !toggle;
}

// ==================
// |21.01.2021 22:16|
// |O:-10.4  S: 50.1| 
// |R: 35.7  F: 40.5| 
// |I: 70.4  V:100.3|    toggle: |I: 70.4  E:  100|
// ==================

void Lcd::refresh() {
  RealTimeClock::getDateTimeString(line[0], true);
  line[0][10] = ' ';
  line[0][16] = '\0';

  line[1][ 0] = 'O';
  line[1][ 1] = ':';
  dtostrf(TemperatureSensors::getValueOut(), 5, 1, line[1] + 2);
  line[1][ 7] = ' ';
  line[1][ 8] = ' ';
  line[1][ 9] = 'S';
  line[1][10] = ':';
  dtostrf(Controller::getSetpointTemperature(), 5, 1, line[1] + 11);

  line[2][ 0] = 'R';
  line[2][ 1] = ':';
  dtostrf(TemperatureSensors::getValueRet(), 5, 1, line[2] + 2);
  line[2][ 7] = ' ';
  line[2][ 8] = ' ';
  line[2][ 9] = 'F';
  line[2][10] = ':';
  dtostrf(TemperatureSensors::getValueFor(), 5, 1, line[2] + 11);
  
  line[3][ 0] = 'I';
  line[3][ 1] = ':';
  dtostrf(TemperatureSensors::getValueInf(), 5, 1, line[3] + 2);
  line[3][ 7] = ' ';
  line[3][ 8] = ' ';

  if (Error::getError() && toggle) {
    line[3][ 9] = 'E';
    line[3][10] = ':';  
    sprintf_P(line[3] + 11, PSTR("%5lX"), Error::getError()); 
  } else {
    line[3][ 9] = 'V';
    line[3][10] = ':';
    dtostrf(Controller::getValvePosition(), 5, 1, line[3] +  11);
  }

  for (int i = 0; i < 4; i++) {
    llcd.setCursor(0, i);
    llcd.print(line[i]);
  }

#ifdef NO_PERIPHERY
  Serial.println("==================");
  for (int i = 0; i < 4; i++) {
    Serial.println(line[i]);
  }
  Serial.println("==================");
#endif
}
