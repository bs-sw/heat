#include "temperatureSensors.h"

byte TemperatureSensors::adr_out[8] = {0x28,0xAA,0xC6,0x5B,0x06,0x00,0x00,0xAE};
byte TemperatureSensors::adr_for[8] = {0x28,0xED,0x5F,0xAF,0x1A,0x13,0x01,0x73};
byte TemperatureSensors::adr_inf[8] = {0x28,0x85,0xD8,0xAA,0x1A,0x13,0x01,0x0A};
byte TemperatureSensors::adr_ret[8] = {0x28,0x91,0x33,0xB6,0x1A,0x13,0x01,0x0D};
OneWire TemperatureSensors::ds(ONEWIRE_PIN);
byte TemperatureSensors::data[12];
float TemperatureSensors::temperature_out;
float TemperatureSensors::temperature_for;
float TemperatureSensors::temperature_inf;
float TemperatureSensors::temperature_ret;
bool TemperatureSensors::outsideTemperatureRequested;

void TemperatureSensors::requestOutsideTemperature() {
  outsideTemperatureRequested = true;
}

void TemperatureSensors::init() {
  requestOutsideTemperature();
  setSensorResolution(adr_out);
  setSensorResolution(adr_for);
  setSensorResolution(adr_inf);
  setSensorResolution(adr_ret);
  startTemperatureConversion();
  delay(1000);
  refreshTemperatureValues();
}

float TemperatureSensors::getValueOut() {
#ifdef TEMPERATURES_SIMULATED
  return -10;
#else
  return temperature_out;
#endif
}

float TemperatureSensors::getValueFor() {
#ifdef TEMPERATURES_SIMULATED
  return 45;
#else
  return temperature_for;
#endif
}

float TemperatureSensors::getValueInf() {
#ifdef TEMPERATURES_SIMULATED
  return 75;
#else
  return temperature_inf;
#endif
}

float TemperatureSensors::getValueRet() {
#ifdef TEMPERATURES_SIMULATED
  return 60;
#else
  return temperature_ret;
#endif
}

void TemperatureSensors::startTemperatureConversion() {
  ds.reset();
  ds.write(0xCC);  //Skip ROM - all slaves should be requested
  ds.write(0x44);  //start conversion
}

void TemperatureSensors::refreshTemperatureValues() {
  if (outsideTemperatureRequested) {
    ds.reset();
    ds.select(adr_out);    
    ds.write(0xBE);
    for (int i = 0; i < 9; i++) {           // 9 bytes
      data[i] = ds.read();
    }
    if (OneWire::crc8(data, 8) == data[8]) temperature_out = ((data[1] << 8) + data[0])/16.0;
    if (temperature_out > 50 || temperature_out < -50) {
      temperature_out = -99;
      Error::setError(ERROR_OUTSIDE_TEMP_IMPLAUSIBLE);
    } else {
      outsideTemperatureRequested = false;
    }
  }

  ds.reset();
  ds.select(adr_for);    
  ds.write(0xBE);
  for (int i = 0; i < 9; i++) {           // 9 bytes
    data[i] = ds.read();
  }
  if (OneWire::crc8(data, 8) == data[8]) temperature_for = ((data[1] << 8) + data[0])/16.0;
  if (temperature_for > 110 || temperature_for < 10) {
    temperature_for = -9;
    Error::setError(ERROR_FORWARD_TEMP_IMPLAUSIBLE);
  }

  ds.reset();
  ds.select(adr_inf);    
  ds.write(0xBE);
  for (int i = 0; i < 9; i++) {           // 9 bytes
    data[i] = ds.read();
  }
  if (OneWire::crc8(data, 8) == data[8]) temperature_inf = ((data[1] << 8) + data[0])/16.0;
  if (temperature_inf > 110 || temperature_inf < 10) {
    temperature_inf = -9;
    Error::setError(ERROR_INFLOW_TEMP_IMPLAUSIBLE);
  }
  
  ds.reset();
  ds.select(adr_ret);    
  ds.write(0xBE);
  for (int i = 0; i < 9; i++) {           // 9 bytes
    data[i] = ds.read();
  }
  if (OneWire::crc8(data, 8) == data[8]) temperature_ret = ((data[1] << 8) + data[0])/16.0;
  if (temperature_ret > 110 || temperature_ret < 10) {
    temperature_ret = -9;
    Error::setError(ERROR_RETURN_TEMP_IMPLAUSIBLE);
  }
}

void TemperatureSensors::setSensorResolution(byte adress[]) {
  ds.reset();
  ds.select(adress); 
  ds.write(0x4E);
  ds.write(0);              //temperature alarm configuration TH
  ds.write(0);              //temperature alarm configuration TL
  ds.write(0x7F);           //resolution 12 Bit 0x7F

  ds.reset();
  ds.select(adress);
  ds.write(0x48, false);    //save resolution in EEPROM
  delay(20);                //Writing of EEPROM takes 10ms
  ds.reset();
}
