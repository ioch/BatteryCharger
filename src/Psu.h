#ifndef _PSU_H
#define _PSU_H

#include "Arduino.h"

#define MODE_CC 0
#define MODE_CV 1

#define MIN_VOLTAGE 0
#define MAX_VOLTAGE 16300
#define MIN_CURRENT 50
#define MAX_CURRENT 10150
#define V_ERROR -240				// this have to be measured experimentaly

typedef struct _PID {
    float P;
    float I;
    float D; } PID;

class Psu {
    public:
        Psu(uint8_t dacOutPin, uint8_t isensePin, uint8_t vsensePin, PID *ipid, PID *vpid);
        void setConstantVoltage(uint16_t voltage);
        void setConstantCurrent(uint16_t current);
        void servo();
        uint16_t getCurrent();
        uint16_t getVoltage();
        int16_t getControllSignal();
        uint8_t getMode();
        void off();
        void controll(int value);
    private:
        void pidIteration(float processVariable, PID *coeffs);
        void setVoltage(unsigned int newSetPoint);
        void setCurrent(unsigned int newSetPoint);

        uint8_t dacOutPin;
        uint8_t isensePin;
        uint8_t vsensePin; 
        uint8_t mode;

        unsigned int currentSetPoint;
        unsigned int voltageSetPoint;

        PID *ipid;
        PID *vpid;
        float errIntegral;
        float setPoint;
        int controlSignal;
        
        uint16_t current;
        uint16_t voltage;
};
#endif
