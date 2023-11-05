#ifndef __temperatureSensors__
#define __temperatureSensors__

#include "heat.h"
#include "error.h"
#include <Arduino.h>
#include <OneWire.h>

#define ONEWIRE_PIN 6

class TemperatureSensors {
private:
  static OneWire ds;
	static byte adr_out[]; //Adresse Aussensensor
	static byte adr_for[]; //Adresse Vorlaufsensor
  static byte adr_inf[];
  static byte adr_ret[];
  static byte data[12];
  static void setSensorResolution(byte adress[]);
  static bool outsideTemperatureRequested;
public:
  static float temperature_out;
  static float temperature_for;
  static float temperature_inf;
  static float temperature_ret;
  static void refreshTemperatureValues();
  static void startTemperatureConversion();
  static void init();
	static float getValueOut();
	static float getValueFor();
  static float getValueInf();
  static float getValueRet();
  static void requestOutsideTemperature();
};

#endif
