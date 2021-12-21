#include"Arduino.h"
#include"MPPT.h"

MPPT::MPPT(int mosfetGatePin, 
          int panelVoltageDividerPin,
          int currentSensorPin) {
  _mosfetGatePin = mosfetGatePin;
  _panelVoltageDividerPin = panelVoltageDividerPin;
  _currentSensorPin = currentSensorPin;
  setPwmRatioToMosfetGate(0.0);
}

float MPPT::findMaximumPowerPoint(int maxIterations=7){
  float pwmRatioLowerBound = findPwmRatioLowerBound();
  float pwmRatioUpperBound = findPwmRatioUpperBound();
  if(pwmRatioLowerBound > pwmRatioUpperBound) {
    return 0.0;
  }

  int left = pwmRatioLowerBound;
  int right = pwmRatioUpperBound;
  int middle = pwmRatioMidPoint(left, right);

  int i = 0;
  while(i < maxIterations && (left<right)){
    setPwmRatioToMosfetGate(middle);
    float currentPower = calculatePower();
    setPwmRatioToMosfetGate(middle + 0.001);
    float currentPowerRight = calculatePower();
    if(currentPower>currentPowerRight){
      setPwmRatioToMosfetGate(middle - 0.001);
      float currentPowerLeft = calculatePower();
      if (currentPower>currentPowerLeft){
        return currentPower;
      }
      right = pwmRatioMidPoint(left, middle);
      left = left;
    } else {
      left = pwmRatioMidPoint(middle, right);
      right = right;
    }
    middle = pwmRatioMidPoint(left, right);
    i++;
  }
  Serial.println("Terminated");
  return calculatePower();
}

float MPPT::measureVoltage(){
  long panelVoltageDividerR2 = 100000;
  long panelVoltageDividerR1 = 470000;
  float panelVoltageDividerRatio = (panelVoltageDividerR2 + panelVoltageDividerR1 + 0.0) / (panelVoltageDividerR2 + 0.0);
  int panelVoltageDividerAnalogValue = analogRead(_panelVoltageDividerPin);
  float panelVoltageDividerOut = 5.0 * (panelVoltageDividerAnalogValue / 1023.0);
  float panelVoltage = panelVoltageDividerOut * panelVoltageDividerRatio;
  return panelVoltage;
}

float MPPT::measureCurrent(){
  int currentSensorValue = analogRead(_currentSensorPin);
  float currentSensorVoltage = (currentSensorValue - 513) * 5.0 / 1023.0;
  float panelCurrent = currentSensorVoltage / 0.066;
  return panelCurrent;
}

float MPPT::calculatePower(){
  return measureCurrent()*measureVoltage();
}

float MPPT::findPwmRatioLowerBound() {
  float originalPwmRatio = _pwmRatio;
  setPwmRatioToMosfetGate(0.0);
  delay(10);
  float openCircuitVoltage = measureVoltage();
  for(int i = 1; i < 1000; i++) {
    setPwmRatioToMosfetGate((float)i / 1000.0);
    float voltage = measureVoltage();
    if(openCircuitVoltage - voltage > 0.250) {
      return _pwmRatio;
    }
  }
  setPwmRatioToMosfetGate(originalPwmRatio);
  return 1.0;
}

float MPPT::findPwmRatioUpperBound() {
  float originalPwmRatio = _pwmRatio;
  setPwmRatioToMosfetGate(1.0);
  delay(10);
  float shortCircuitCurrent = measureCurrent();
  for(int i = 1000; i > 0; i--) {
    setPwmRatioToMosfetGate((float)i / 1000.0);
    float current = measureCurrent();
    if(shortCircuitCurrent - current > 0.150) {
      return _pwmRatio;
    }
  }
  setPwmRatioToMosfetGate(originalPwmRatio);
  return 0.0;
}

float MPPT::pwmRatioMidPoint(float lower, float upper) {
  return (float)(((int)(lower * 1000) + (int)(upper * 1000)) / 2) / 1000;
}

void MPPT::setPwmRatioToMosfetGate(float pwmRatio) {
  _pwmRatio = max(min(pwmRatio, 1.0), 0.0);
  analogWrite13(_mosfetGatePin, (uint16_t)(8191*_pwmRatio));
}

void MPPT::setupPWM13() {
  DDRB |= _BV(PB1) | _BV(PB2); //Set pins as outputs
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) //Non-Inv PWM
      | _BV(WGM11); // Mode 14: Fast PWM, TOP=ICR1
  TCCR1B = _BV(WGM13) | _BV(WGM12)
      | _BV(CS10); // Prescaler 1
  ICR1 = icr; // TOP counter value (Relieving OCR1A*)
}

//* 13-bit version of analogWrite(). Only for D9 & D10
void MPPT::analogWrite13(uint8_t pin, uint16_t val) {
  switch (pin) {
    case 9: OCR1A = val; break;
    case 10: OCR1B = val; break;
  }
}
