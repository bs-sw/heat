#ifndef __logger__
#define __logger__

#define SPI_CHIP_SELECT_SD 8

#include "heat.h"
#include <SPI.h>
#include <SD.h>
#include "error.h"
#include "temperatureSensors.h"
#include "controller.h"
#include "realTimeClock.h"

class Logger {
private:
public:
  static void init();
  static void writeToSD();
  static void writeErrorToSD(unsigned long err, unsigned long additionalValue);
};

#endif
