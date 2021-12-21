#include"QuickMedianLib.h"
#include"MPPT.h"
#include"SolarTracker.h"

int mosfetGatePin = 9;
float pwmRatio = 0.0;
int increment = 1000;
int currentSensorPin = A2;
int panelVoltageDividerPin = A3;

int ldrEast = A0;
int ldrWest = A1;
int primaryEnablePin = 1;
int primaryDirectionPin = 2;
int primaryPulsePin = 3;
int secondaryEnablePin = 4;
int secondaryDirectionPin = 5;
int secondaryPulsePin = 6;

bool doDualAxisTracking = true;

long panelVoltageDividerR2 = 100000;
long panelVoltageDividerR1 = 470000;
float panelVoltageDividerRatio = (panelVoltageDividerR2 + panelVoltageDividerR1 + 0.0) / (panelVoltageDividerR2 + 0.0);

MPPT mppt(mosfetGatePin, panelVoltageDividerPin, currentSensorPin);
SolarTracker tracker(ldrEast, ldrWest,
                      primaryEnablePin, primaryDirectionPin, primaryPulsePin,
                      secondaryEnablePin, secondaryDirectionPin, secondaryPulsePin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(mosfetGatePin, OUTPUT);
  pinMode(panelVoltageDividerPin, INPUT);
  pinMode(currentSensorPin, INPUT);
  setupPwm13();
  
  pinMode(primaryEnablePin, OUTPUT);
  pinMode(primaryDirectionPin, OUTPUT);
  pinMode(primaryPulsePin, OUTPUT);
  pinMode(secondaryEnablePin, OUTPUT);
  pinMode(secondaryDirectionPin, OUTPUT);
  pinMode(secondaryPulsePin, OUTPUT);

  delay(3000);
  if(doDualAxisTracking) {
    tracker.turnSecondaryAxis();
  }
  delay(1000);
}

void loop() {
  
  /*setPwmRatioToMosfetGate();
  listenAndChangePwmRatio();
  float panelVoltage = measurePanelVoltage();
  float panelCurrent = measurePanelCurrent();
  Serial.println("Voltage measurement " + String(panelVoltage, 4) + ", current measurement: " + String(panelCurrent, 4));*/
  
  Serial.println("Primary axis started");
  tracker.trackAndAdjust();
}

void listenAndChangePwmRatio() {
  if(Serial.available() > 0){
    int input = Serial.parseInt();
    pwmRatio = max(min((pwmRatio + input * (1.0/increment)), 1.0), 0.0);
    Serial.println(pwmRatio, 4);
  }
}

float measurePanelVoltage() {
  int panelVoltageDividerAnalogValue = analogRead(panelVoltageDividerPin);
  float panelVoltageDividerOut = 5.0 * ((float)(panelVoltageDividerAnalogValue) / 1023.0);
  float panelVoltage = panelVoltageDividerOut * panelVoltageDividerRatio;
  return panelVoltage;
}

float measurePanelCurrent() {
  int currentSensorValue = analogRead(currentSensorPin);
  float currentSensorVoltage = (currentSensorValue - 513) * 5.0 / 1023.0;
  float panelCurrent = currentSensorVoltage / 0.066;
  return panelCurrent;
}

void setPwmRatioToMosfetGate() {
  analogWrite13(mosfetGatePin, (uint16_t)(8191*pwmRatio));
}

uint16_t icr = 0x1fff;

void setupPwm13() {
  DDRB |= _BV(PB1) | _BV(PB2); //Set pins as outputs
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) //Non-Inv pwm
           | _BV(WGM11); // Mode 14: Fast pwm, TOP=ICR1
  TCCR1B = _BV(WGM13) | _BV(WGM12)
           | _BV(CS10); // Prescaler 1
  ICR1 = icr; // TOP counter value (Relieving OCR1A*)
}
//* 16-bit version of analogWrite(). Only for D9 & D10
void analogWrite13(uint8_t pin, uint16_t val) {
  switch (pin) {
    case 9: OCR1A = val; break;
    case 10: OCR1B = val; break;
  }
}
