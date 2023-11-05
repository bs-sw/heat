#include "rs485.h"

byte Rs485::inputBuffer[INPUT_BUFFER_SIZE];
byte Rs485::outputBuffer[OUTPUT_BUFFER_SIZE];
byte Rs485::inputBufferIndex;
bool Rs485::inputBufferReady;
bool Rs485::packageReceived;

void Rs485::init() {
  digitalWrite(OUTPUT_SEND_ENABLE_PIN, LOW);
  pinMode(OUTPUT_SEND_ENABLE_PIN, OUTPUT);
  Serial.begin(19200);
}

unsigned long Rs485::crc_update(unsigned long crc, byte data) {
    byte tbl_idx;
    tbl_idx = crc ^ (data >> (0 * 4));
    crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
    tbl_idx = crc ^ (data >> (1 * 4));
    crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
    return crc;
}

unsigned long Rs485::crc_string(byte *s, int len) {
  unsigned long crc = ~0L;
  int i;
  for (i=0; i<len; i++) {
    crc = crc_update(crc, *s++);
  }
  crc = ~crc;
  return crc;
}

byte Rs485::initializeOutputBuffer() {
  outputBuffer[0] = '{';
  outputBuffer[BUFFER_POSITION_SLAVE_NUMBER] = SLAVE_NUMBER;
  outputBuffer[BUFFER_POSITION_PACKAGE_LENGTH] = 0;
  outputBuffer[BUFFER_POSITION_CRC_BYTE_0] = 0;
  outputBuffer[BUFFER_POSITION_CRC_BYTE_1] = 0;
  outputBuffer[BUFFER_POSITION_CRC_BYTE_2] = 0;
  outputBuffer[BUFFER_POSITION_CRC_BYTE_3] = 0;
  return BUFFER_POSITION_PAYLOAD;
}

bool Rs485::addToInputBuffer(byte data) {
  inputBuffer[inputBufferIndex] = data;
  inputBufferIndex++;
  if (inputBufferIndex > INPUT_BUFFER_SIZE) return false;
  return true;
}

bool Rs485::inputBufferCrcValid() {
  unsigned long transmittedCrc;
  memcpy(&transmittedCrc, &inputBuffer[BUFFER_POSITION_CRC_BYTE_0], sizeof(transmittedCrc));
  inputBuffer[BUFFER_POSITION_CRC_BYTE_0] = 0;
  inputBuffer[BUFFER_POSITION_CRC_BYTE_1] = 0;  
  inputBuffer[BUFFER_POSITION_CRC_BYTE_2] = 0;
  inputBuffer[BUFFER_POSITION_CRC_BYTE_3] = 0;
  unsigned long calculatedCrc = crc_string(inputBuffer, inputBufferIndex);
  return (calculatedCrc == transmittedCrc);
}

void Rs485::initializeInputBuffer() {
  inputBufferIndex = 0;
}

void Rs485::send() {
  byte outputBufferIndex = initializeOutputBuffer();
  //----
  outputBuffer[outputBufferIndex] = FUNCTION_TEMPERATURE_STP;
  memcpy(&outputBuffer[outputBufferIndex + 1], &Controller::setpointTemperature, sizeof(Controller::setpointTemperature));
  outputBufferIndex += 1 + sizeof(Controller::setpointTemperature);
  //----
  outputBuffer[outputBufferIndex] = FUNCTION_TEMPERATURE_OUT;
  memcpy(&outputBuffer[outputBufferIndex + 1], &TemperatureSensors::temperature_out, sizeof(TemperatureSensors::temperature_out));
  outputBufferIndex += 1 + sizeof(TemperatureSensors::temperature_out); 
  //----
  outputBuffer[outputBufferIndex] = FUNCTION_TEMPERATURE_INF;
  memcpy(&outputBuffer[outputBufferIndex + 1], &TemperatureSensors::temperature_inf, sizeof(TemperatureSensors::temperature_inf));
  outputBufferIndex += 1 + sizeof(TemperatureSensors::temperature_inf);
  //----
  outputBuffer[outputBufferIndex] = FUNCTION_TEMPERATURE_FOR;
  memcpy(&outputBuffer[outputBufferIndex + 1], &TemperatureSensors::temperature_for, sizeof(TemperatureSensors::temperature_for));
  outputBufferIndex += 1 + sizeof(TemperatureSensors::temperature_for);
  //----
  outputBuffer[outputBufferIndex] = FUNCTION_TEMPERATURE_RET;
  memcpy(&outputBuffer[outputBufferIndex + 1], &TemperatureSensors::temperature_ret, sizeof(TemperatureSensors::temperature_ret));
  outputBufferIndex += 1 + sizeof(TemperatureSensors::temperature_ret);
  //----
  outputBuffer[outputBufferIndex] = FUNCTION_VALVE_POSITION;
  memcpy(&outputBuffer[outputBufferIndex + 1], &Controller::valvePosition, sizeof(Controller::valvePosition));
  outputBufferIndex += 1 + sizeof(Controller::valvePosition);
  //----
  outputBuffer[outputBufferIndex] = FUNCTION_DATE_TIME;
  outputBuffer[outputBufferIndex + 1] = RealTimeClock::getYear();
  outputBuffer[outputBufferIndex + 2] = RealTimeClock::getMonth();
  outputBuffer[outputBufferIndex + 3] = RealTimeClock::getDay();
  outputBuffer[outputBufferIndex + 4] = RealTimeClock::getHour();
  outputBuffer[outputBufferIndex + 5] = RealTimeClock::getMinute();
  outputBufferIndex += 1 + 5;
  //----
  outputBuffer[outputBufferIndex] = FUNCTION_ERROR;
  memcpy(&outputBuffer[outputBufferIndex + 1], &Error::error, sizeof(Error::error));
  outputBufferIndex += 1 + sizeof(Error::error);
  //----
  outputBuffer[outputBufferIndex] = FUNCTION_COM_ERROR_COUNT;
  memcpy(&outputBuffer[outputBufferIndex + 1], &Error::comErrorCount, sizeof(Error::comErrorCount));
  outputBufferIndex += 1 + sizeof(Error::comErrorCount);
  //----
  outputBuffer[outputBufferIndex] = '}';
  outputBufferIndex += 1;
  //----
  outputBuffer[BUFFER_POSITION_PACKAGE_LENGTH] = outputBufferIndex;
  //----
  unsigned long crc = crc_string(outputBuffer, outputBufferIndex);
  memcpy(&outputBuffer[BUFFER_POSITION_CRC_BYTE_0], &crc, sizeof(crc));
  //----
  digitalWrite(OUTPUT_SEND_ENABLE_PIN, HIGH); //enable MAX485 driver
  Serial.write(outputBuffer, outputBufferIndex);
  Serial.flush();
  digitalWrite(OUTPUT_SEND_ENABLE_PIN, LOW); //enable MAX485 driver
  //----
  if (outputBufferIndex > OUTPUT_BUFFER_SIZE) {
    while(true) {
      Serial.println(F("Overflow"));
    }
  }
}
//{.  sn  le  crc........   func.  .}
//7B  01  08  F7 D6 88 F0          7D   //call without command

bool Rs485::receive() {
	  //Paket in Puffer einlesen
  while(Serial.available()) {
    byte input = Serial.read();
    #ifdef DEBUG 
      Serial.println(input);
    #endif
    if (!inputBufferReady) {
      if (input == '{') {
        initializeInputBuffer();
        inputBufferReady = true;
        #ifdef DEBUG 
          Serial.println(F("input buffer initialized"));
        #endif
      } else {
        Error::incComErrorCount();
      }
    }
    if (inputBufferReady) {
      if (!addToInputBuffer(input)) {
        Error::incComErrorCount();
        inputBufferReady = false;
        #ifdef DEBUG 
          Serial.println(F("input buffer overflow"));
        #endif
      }
      if (inputBufferReady && input == '}') {
        packageReceived = true;
        inputBufferReady = false;
        #ifdef DEBUG 
          Serial.println(F("package received"));
        #endif
        break;
      } 
    }
  }

  //Process received package
  if (packageReceived) {
    packageReceived = false;

    byte packageLen = inputBuffer[BUFFER_POSITION_PACKAGE_LENGTH];
    if (packageLen != inputBufferIndex || packageLen > INPUT_BUFFER_SIZE) {
      Error::incComErrorCount();
      #ifdef DEBUG 
        Serial.println(F("invalid length"));
      #endif
      return false;
    }
    
    if(!inputBufferCrcValid()) {
      Error::incComErrorCount();
      #ifdef DEBUG 
        Serial.println(F("invalid crc"));
      #endif
      return false;
    }

    #ifdef DEBUG 
      Serial.print(F("Slave number "));
      Serial.println(inputBuffer[BUFFER_POSITION_SLAVE_NUMBER]);
    #endif
      
    if(inputBuffer[BUFFER_POSITION_SLAVE_NUMBER] == SLAVE_NUMBER) {
      #ifdef DEBUG 
        Serial.println(F("Slave number matching"));
      #endif
      inputBufferIndex = BUFFER_POSITION_PAYLOAD;
      while(inputBufferIndex < packageLen - 1) {
        byte function = inputBuffer[inputBufferIndex];
        inputBufferIndex += 1;
        switch(function) {
          case FUNCTION_DATE_TIME: 
            executeDateTimeCommand(inputBuffer[inputBufferIndex], inputBuffer[inputBufferIndex+1], inputBuffer[inputBufferIndex+2],
              inputBuffer[inputBufferIndex+3], inputBuffer[inputBufferIndex+4]);
            inputBufferIndex += 5;
            break;
          default: 
            Error::incComErrorCount();
            #ifdef DEBUG 
              Serial.println(F("Unknown function"));
            #endif
            return false;
        }
        if (inputBufferIndex > packageLen - 1) {
          Error::incComErrorCount();
          #ifdef DEBUG 
              Serial.println(F("invalid length"));
          #endif
          return false;
        }
      }
	  return true; //sendResponse();
    }
  }
  return false;
}

void Rs485::communicate() {
  //if (receive()) send();    //----------------------temporary unused
  send();
  
}

void Rs485::executeDateTimeCommand(byte year, byte month, byte day, byte hour, byte minute) {
  RealTimeClock::setTime(year, month, day, hour, minute);
}
