#include"Arduino.h"
#include"SolarTracker.h"
#include"AccelStepper.h"



SolarTracker::SolarTracker(int ldrEast, int ldrWest,
                           int primaryEnablePin, int primaryDirectionPin, int primaryPulsePin,
                           int secondaryEnablePin, int secondaryDirectionPin, int secondaryPulsePin){
  _ldrEast = ldrEast;
  _ldrWest = ldrWest;
  
  _primaryEnablePin = primaryEnablePin;
  _primaryDirectionPin = primaryDirectionPin;
  _primaryPulsePin = primaryPulsePin;
  
  _secondaryEnablePin = secondaryEnablePin;
  _secondaryDirectionPin = secondaryDirectionPin;
  _secondaryPulsePin = secondaryPulsePin;
  
  _primaryAxisStepper = AccelStepper(1, primaryPulsePin, primaryDirectionPin);
  _primaryAxisStepper.setCurrentPosition(0);
  _primaryAxisStepper.setMaxSpeed(STEPS_PER_SECOND);
  _primaryAxisStepper.setAcceleration(STEPS_PER_SECOND_SQUARED);
  
  _secondaryAxisStepper = AccelStepper(1, secondaryPulsePin, secondaryDirectionPin);
  _secondaryAxisStepper.setCurrentPosition(0);
  _secondaryAxisStepper.setMaxSpeed(STEPS_PER_SECOND);
  _secondaryAxisStepper.setAcceleration(STEPS_PER_SECOND_SQUARED);
}

void SolarTracker::trackAndAdjust(){
  double sensorDifferences[3]; //Array where LDR measurements(differences) are initially stored.
  double averageArray[3]; // Where the averages are passed to.
  Serial.println("New loop.");

  for(int j = 0 ; j < 3 ; j++){
    for(int i = 0 ; i < 3 ; i++){
      sensorDifferences[i] = analogRead(_ldrEast)-analogRead(_ldrWest);
      Serial.println("Sensor difference: "+ String(sensorDifferences[i]));
    }
    averageArray[j] = getAverage(sensorDifferences);
  }
  
  int decision = decideDirection(averageArray);
  turnMotor(decision);
}

void SolarTracker::turnSecondaryAxis(){
  int targetPos = getSecondaryMotorPosition();
  digitalWrite(_secondaryEnablePin, HIGH);
  _secondaryAxisStepper.moveTo(targetPos);
  _secondaryAxisStepper.runToPosition();
  digitalWrite(_secondaryEnablePin, LOW);
}

double SolarTracker::getAverage(double arr[]){
  double avg = (arr[0] + arr[1] + arr[2])/3;
  return avg;
}

int SolarTracker::decideDirection(double arr[]) {
  int decisionCounter = 0;
  for(int i = 0 ; i < 3 ; i++){
    if(arr[i] > SENSITIVITY){ 
      decisionCounter++;
    } else if(arr[i] < SENSITIVITY*(-1)){
      decisionCounter--;
    }
  }
  return decisionCounter;
}

int SolarTracker::turnMotor(int decision) {
  String turningDirection = "HOLD";
  if(decision >= 1) {
    turningDirection = "EAST"; 
  } else if (decision <= -1) {
    turningDirection = "WEST";
  }

  if(turningDirection != "HOLD") {
    turnInDirection(turningDirection);
  }
}

void SolarTracker::turnInDirection(String dir) {
  String directionString = (dir == "EAST" ? "East" : "West");
    _primaryAxisStepper.moveTo(GEAR_RATIO*(dir == "EAST" ? EAST_BOUNDARY : WEST_BOUNDARY));
  
  Serial.println("Turning " + directionString + ", _ldrEast" + String(analogRead(_ldrEast))+ " _ldrWest "+ String(analogRead(_ldrWest)));
  while(_primaryAxisStepper.isRunning()){
    _primaryAxisStepper.run();
    
    int oppositeLdr = analogRead(dir == "EAST" ? _ldrWest : _ldrEast);
    int sameDirLdr = analogRead(dir == "EAST" ? _ldrEast : _ldrWest);
    // Since East LDR is less sensitive to light, we have to compensate
    oppositeLdr = (dir == "EAST" ? oppositeLdr + LDR_BIAS : oppositeLdr - LDR_BIAS);

    int ldrEastValue = analogRead(_ldrEast);
    int ldrWestValue = analogRead(_ldrWest);
    if(isSensorValueWithinRange(oppositeLdr, sameDirLdr)){
      //We need to bring the panel to a halt slowly, so we have to calculate the distance it would take to slow down at the deceleration rate
      /*int directionSign = (dir == "EAST" ? -1 : 1);
      float timeToSlowDown = STEPS_PER_SECOND / STEPS_PER_SECOND_SQUARED;
      float distanceToStopAtFullSpeed = STEPS_PER_SECOND * timeToSlowDown;
      float distanceToStopWithLinearDeceleration =  distanceToStopAtFullSpeed / 2.0;
      _primaryAxisStepper.move(distanceToStopWithLinearDeceleration * directionSign);
      
      _primaryAxisStepper.runToPosition();*/
      Serial.println("Was turning " + directionString + ", _ldrEast: " + String(ldrEastValue) + "LDRWest: " + String(ldrWestValue));
      break;
    }
  }
}

bool SolarTracker::isSensorValueWithinRange(int valueToCheck, int valueToCompareTo){
  return (valueToCheck-valueToCompareTo) > 0; 
}

bool SolarTracker::isPanelMoving() {
  return _primaryAxisStepper.isRunning();
}

int SolarTracker::getSecondaryMotorPosition() {
  float degreesToPosition = HIGHEST_SUN_POSITION_TODAY - ZERO_POSITION_ANGLE;
  float bigStepsToPosition = degreesToPosition / 1.8;
  float smallStepsToPosition = bigStepsToPosition * GEAR_RATIO;
  return -smallStepsToPosition;
}
