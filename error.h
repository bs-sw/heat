#ifndef __error__
#define __error__

#include "heat.h"
#include <Arduino.h>

#define ERROR_OUTPUT_PIN 2

#define ERROR_FORWARD_TEMP_IMPLAUSIBLE    0x001
#define ERROR_OUTSIDE_TEMP_IMPLAUSIBLE    0x002
#define ERROR_INFLOW_TEMP_IMPLAUSIBLE     0x004
#define ERROR_RETURN_TEMP_IMPLAUSIBLE     0x008
#define ERROR_WATCHDOG_CYCLE_TIME         0x010
#define ERROR_DATE_TIME_COULDNT_BE_SET    0x020
#define ERROR_DATE_TIME_COULDNT_BE_READ   0x040
#define ERROR_SD_INIT                     0x080
#define ERROR_SD_FILE_WRITE               0x100

class Error {
private:
  static bool errorPin;
public:
  static unsigned long error;
  static unsigned int comErrorCount;
  static void init();
  static void setError(unsigned long err, unsigned long additionalValue);
  static void setError(unsigned long err);
  static void setErrorWithoutLogging(unsigned long err);
  static unsigned long getError();
  static void output();
  static void resetError(unsigned long err);
  static void incComErrorCount();
  static unsigned int getComErrorCount();
};

#endif
