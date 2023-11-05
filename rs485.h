#ifndef __Rs485__
#define __Rs485__

#include <Arduino.h>
#include <avr/pgmspace.h>
#include "temperatureSensors.h"

//Serial Communication Settings
#define OUTPUT_SEND_ENABLE_PIN 3
#define INPUT_BUFFER_SIZE 64
#define OUTPUT_BUFFER_SIZE 64

//Slave specific function
#define SLAVE_NUMBER 1
#define FUNCTION_TEMPERATURE_OUT    'O'   //float
#define FUNCTION_TEMPERATURE_INF    'I'   //float
#define FUNCTION_TEMPERATURE_STP    'S'   //float
#define FUNCTION_TEMPERATURE_FOR    'F'   //float
#define FUNCTION_TEMPERATURE_RET    'R'   //float
#define FUNCTION_VALVE_POSITION     'V'   //float
#define FUNCTION_DATE_TIME          'T'   //5 x unsigned byte
#define FUNCTION_ERROR              'E'   //unsigned long
#define FUNCTION_COM_ERROR_COUNT    'C'   //unsigned int

//Buffer Positions
#define BUFFER_POSITION_SLAVE_NUMBER    1 
#define BUFFER_POSITION_PACKAGE_LENGTH  2 
#define BUFFER_POSITION_CRC_BYTE_0      3 
#define BUFFER_POSITION_CRC_BYTE_1      4 
#define BUFFER_POSITION_CRC_BYTE_2      5 
#define BUFFER_POSITION_CRC_BYTE_3      6 
#define BUFFER_POSITION_PAYLOAD         7 

const static PROGMEM uint32_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

class Rs485 {
private:
  static byte inputBuffer[];
  static byte inputBufferIndex;
  static byte outputBuffer[];
  static bool inputBufferReady;
  static bool packageReceived;
  static void initializeInputBuffer();
  static byte initializeOutputBuffer();
  static bool addToInputBuffer(byte data);
  static bool inputBufferCrcValid();
  static unsigned long crc_update(unsigned long crc, byte data);
  static unsigned long crc_string(byte *s, int len);
  static void executeDateTimeCommand(byte year, byte month, byte day, byte hour, byte minute);
  static void send();
  static bool receive();
public:
  static void init();
  static void communicate();
};

#endif
