#ifndef __controller__
#define __controller__

#define OUTPUT_PIN_RELAY_1 9
#define OUTPUT_PIN_RELAY_2 10

const float valveFullTravelTime = 96000.0;	    //Gesamter Weg des Ventils in Millisekunden
const float valveOverrun	= 10.0;				        //geduldete Ventilwegüberschreitung, ab der die Ausgabe abgeschaltet wird. (Ventil in Endlage)
const float valvePositionOpen = 90.0;			      //Ventilposition komplett geöffnet in Grad
const float valvePositionClose = 0.0;			      //Ventilposition komplett geschlossen in Grad
const float outputCycleTime = 200.0;			      //Minimale Pulszeit in Millisekunden - muss BASE_CLOCK entsprechen
const float controllerCycleTime = 2000.0;       //Reglerzyklus - muss LEVEL2_DIVIDER*BASE_CLOCK entsprechen

const float hcInclination = 80.0;               //einstellen (bei hcInclination=80 und hcDayOffset=20 wird bei 0C->60C und bei 20C->20C ausgegeben)
const float hcDayOffset = 20.0;                 //einstellen
const float hcNightOffset = 18.0;               //einstellen

const float kP = 0.1;
const float kD = 3.0;

const float ta =  controllerCycleTime / 1000.0;
const float maxSetpointSlope = 0.25 * ta;

#include "heat.h"
#include <Arduino.h>
#include "temperatureSensors.h"
#include "realTimeClock.h"
#include "error.h"

class Controller {
private:
  static float currentOffset;
  static float err;
  static float errOld;
  static float errSum;
  static float y;
  static float feedForwardControl;
  static long outputPulse;
  static bool outputInProgress;
  static bool opening;
  static bool moving;
  static bool referenceMode;
  static bool keepOp;
  static void openValve();
  static void closeValve();
  static void stopValve();
  static int cycleCount;
  static unsigned int pulses;
  static float controllerCycle;
public:
  static float valvePosition;
  static float setpointTemperature;
  static float nextSetpointTemperature;
  static void init();
  static void output();
  static void compute();
  static float getSetpointTemperature();
  static float getValvePosition();
  static long getOutputPulse();
  static unsigned int getPulses();
  static void resetPulses();
};

#endif
