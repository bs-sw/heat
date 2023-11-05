#include "controller.h"

float Controller::setpointTemperature;
float Controller::nextSetpointTemperature;
float Controller::valvePosition;
float Controller::currentOffset;
float Controller::err;
float Controller::errOld;
float Controller::y;
long Controller::outputPulse;
bool Controller::outputInProgress;
bool Controller::opening;
bool Controller::moving;
bool Controller::referenceMode = true;
bool Controller::keepOp = false;
int Controller::cycleCount;
unsigned int Controller::pulses;

void Controller::init() {
  digitalWrite(OUTPUT_PIN_RELAY_1, HIGH);
  digitalWrite(OUTPUT_PIN_RELAY_2, HIGH);
  pinMode(OUTPUT_PIN_RELAY_1, OUTPUT);   
  pinMode(OUTPUT_PIN_RELAY_2, OUTPUT);
}

void Controller::compute(){
  if (referenceMode && (cycleCount * controllerCycleTime < valveFullTravelTime)) {
    cycleCount++;
    return;                                                
  } else {
    referenceMode = false;  
  }
  
  if (RealTimeClock::isNight())  currentOffset = hcNightOffset;
    else currentOffset = hcDayOffset;
    
  nextSetpointTemperature = currentOffset + ((hcInclination - 20)*(currentOffset - TemperatureSensors::getValueOut()))/(20 - (TemperatureSensors::getValueOut() - currentOffset) / 2);
  setpointTemperature = setpointTemperature + ((nextSetpointTemperature - setpointTemperature) > maxSetpointSlope ?  maxSetpointSlope : 0);
  setpointTemperature = setpointTemperature + ((nextSetpointTemperature - setpointTemperature) < maxSetpointSlope ? -maxSetpointSlope : 0);
  if (abs(nextSetpointTemperature - setpointTemperature) <= maxSetpointSlope) setpointTemperature = nextSetpointTemperature;
  
  err = setpointTemperature - TemperatureSensors::getValueFor();

  y = kP*err + kD*(err - errOld)/ta;
  errOld = err;

  if (keepOp) outputPulse += y * controllerCycleTime;
  else outputPulse = y * controllerCycleTime;

  if (abs(outputPulse) < outputCycleTime) keepOp = true; 
  else keepOp = false;
}

void Controller::output() {
  if (referenceMode) {
    closeValve();
    valvePosition = 0;
    return;                                               
  }
  if (outputInProgress) {
    valvePosition += (opening ? 1 : -1) * outputCycleTime * (valvePositionOpen - valvePositionClose) / valveFullTravelTime;
  }
  if (abs(outputPulse) >= outputCycleTime) {
    outputInProgress = false;   //Ausgabe beenden (falls kein Endanschlag erreicht ist, wird das Signal gleich wieder gesetzt)
    if(outputPulse > 0 && valvePosition < (valvePositionOpen + valveOverrun)) {
      openValve();
      outputInProgress = true;
      outputPulse -= outputCycleTime;
      if (valvePosition < valvePositionClose) valvePosition = valvePositionClose;
    } 
    if(outputPulse < 0 && valvePosition > (valvePositionClose - valveOverrun)) {
      closeValve();
      outputInProgress = true;
      outputPulse += outputCycleTime;
      if (valvePosition > valvePositionOpen) valvePosition = valvePositionOpen;
    }
  } else {
    outputInProgress = false;  //Ausgabe beenden, da kein ganzer Puls mehr vorhanden
  }
  if (!outputInProgress) {
    stopValve();
  }
}

float Controller::getSetpointTemperature(){
  return setpointTemperature;
}

float Controller::getValvePosition(){
  return valvePosition;
}

void Controller::openValve() {
  #if !defined(RECORD_STEP_RESPONSE)
  digitalWrite(OUTPUT_PIN_RELAY_2, HIGH);
  digitalWrite(OUTPUT_PIN_RELAY_1, LOW);
  #endif
  if (!moving || (moving && !opening)) pulses++;
  moving = true;
  opening = true;
}

void Controller::closeValve() {
  #if !defined(RECORD_STEP_RESPONSE)
  digitalWrite(OUTPUT_PIN_RELAY_1, HIGH);
  digitalWrite(OUTPUT_PIN_RELAY_2, LOW);
  #endif
  if (!moving || (moving && opening)) pulses++;
  opening = false;
  moving = true;
}

void Controller::stopValve() {
  #if !defined(RECORD_STEP_RESPONSE)
  digitalWrite(OUTPUT_PIN_RELAY_1, HIGH);
  digitalWrite(OUTPUT_PIN_RELAY_2, HIGH); 
  #endif
  moving = false;
}

long Controller::getOutputPulse() {
  return outputPulse;
}

unsigned int Controller::getPulses() {
  return pulses;
}

void Controller::resetPulses() {
  pulses = 0;
}
