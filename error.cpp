#include "error.h"

unsigned long Error::error;
unsigned int Error::comErrorCount;
bool Error::errorPin;

void Error::init() {
   pinMode(ERROR_OUTPUT_PIN, OUTPUT);
   resetError(~0);
   output();
}

void Error::incComErrorCount() {
  comErrorCount++;
}

unsigned int Error::getComErrorCount() {
  return comErrorCount;
}

void Error::setErrorWithoutLogging(unsigned long err) {
  error |= err;
}

void Error::setError(unsigned long err, unsigned long additionalValue) {
  Logger::writeErrorToSD(err, additionalValue);
  error |= err;
}

void Error::setError(unsigned long err) {
  Logger::writeErrorToSD(err, 0);
  error |= err;
}

unsigned long Error::getError() {
  return error;
}

void Error::resetError(unsigned long err) {
  error &= ~err;
}

void Error::output() {
  if (error && errorPin) {
    digitalWrite(ERROR_OUTPUT_PIN, LOW);
    errorPin = false;
  } else {
    digitalWrite(ERROR_OUTPUT_PIN, HIGH);
    errorPin = true;
  }
}
