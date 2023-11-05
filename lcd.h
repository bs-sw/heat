#ifndef __lcd__
#define __lcd__

#include "heat.h"
#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "temperatureSensors.h"
#include "realTimeClock.h"
#include "error.h"
#include "controller.h"

#define LCD_ADDRESS 0x27

class Lcd {
private:
  static LiquidCrystal_I2C llcd;
  static char timeString[];
  static char dateString[];
  static bool toggle;
  static char line[][17];
public:
  static void init();
  static void backlight();
  static void refresh();
  static void toggleValues();
};

#endif
