#ifndef SolarTracker_h
  #define SolarTracker_h

  #include"Arduino.h"
  #include"AccelStepper.h"
  class SolarTracker {
    public:
      SolarTracker(int ldrEast, int ldrWest, int primaryEnablePin, int primaryDirectionPin, int primaryPulsePin, int secondaryEnablePin, int secondaryDirectionPin, int secondaryPulsePin);
      void trackAndAdjust();
      void turnSecondaryAxis();
      bool isPanelMoving();
    private:
      const int SENSITIVITY = 5; // The minimum difference in LDR values that the algorithm is allowed to see as change.
      const int LDR_BIAS = 0; //Inherent difference between LDR's
      const int STEPS_PER_SECOND = 100;
      const int STEPS_PER_SECOND_SQUARED = 50;
      const int EAST_BOUNDARY = -75; //North-East direction: earlieast sunrise
      const int WEST_BOUNDARY = 75; //North-West direction: latest sunset
      const int GEAR_RATIO = 47*38/16;
      const int DAY_OF_YEAR = 342;
      const float ZERO_POSITION_ANGLE = 35.0;
      const float HIGHEST_SUN_POSITION_TODAY = 11.42;
    
      // LDRs connected to analog pins
      int _ldrEast; 
      int _ldrWest;
      // Driver connected to 1-3 digital pins. PUL needs to be connected to PWM digital pin.
      int _primaryEnablePin;
      int _primaryDirectionPin;
      int _primaryPulsePin;
      int _secondaryEnablePin;
      int _secondaryDirectionPin;
      int _secondaryPulsePin;
      AccelStepper _primaryAxisStepper;
      AccelStepper _secondaryAxisStepper;

      double getAverage(double arr[]);
      int decideDirection(double arr[]);
      int turnMotor(int decision);
      void turnInDirection(String dir);
      bool isSensorValueWithinRange(int valueToCheck, int valueToCompareTo);
      int getSecondaryMotorPosition();
  };
   
#endif
