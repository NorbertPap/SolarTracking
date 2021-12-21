#ifndef MPPT_h
  #define MPPT_h

  #include"Arduino.h"
  class MPPT {
    public:
      MPPT(int mosfetGatePin, int panelVoltageDividerPin, int currentSensorPin);
      float findMaximumPowerPoint(int maxIterations);
    private:
      float _pwmRatio;
      int _mosfetGatePin;
      int _panelVoltageDividerPin;
      int _currentSensorPin;
  
      float measureVoltage();
      float measureCurrent();
      float calculatePower();
      float findPwmRatioLowerBound();
      float findPwmRatioUpperBound();
      float pwmRatioMidPoint(float lower, float upper);
      void setPwmRatioToMosfetGate(float pwmRatio);
  
      uint16_t icr;
      void setupPWM13();
      void analogWrite13(uint8_t pin, uint16_t val);
  };
   
#endif
