#include "Psu.h"
#include "Arduino.h"

Psu::Psu(uint8_t _dacOutPin, uint8_t _isensePin, uint8_t _vsensePin, PID *_ipid, PID *_vpid):
        dacOutPin(_dacOutPin),
        isensePin(_isensePin),
        vsensePin(_vsensePin),
        ipid(_ipid),
        vpid(_vpid)
{
    //~ C_P = 2.1;
    //~ C_I = 0.05;
    //~ 
    //~ P = 7.1;
    //~ I = 1.5;
}

void Psu::setConstantVoltage(uint16_t voltage){
    mode = MODE_CV;
    setPoint = (float) voltage / (float) MAX_VOLTAGE;
}

void Psu::setConstantCurrent(uint16_t current) {
    mode = MODE_CC;
    setPoint = (float) current / (float) MAX_CURRENT;
}

void Psu::servo(){
    uint16_t currentLSB = analogRead(isensePin);
    current = map(currentLSB, 0, 1023, MIN_CURRENT, MAX_CURRENT);//milliamps

    uint16_t voltageLSB = analogRead(vsensePin);
    voltage = map(voltageLSB, 0, 1023, MIN_VOLTAGE, MAX_VOLTAGE) -V_ERROR;//millivolts

    if(MODE_CC == mode) {
        pidIteration((float) currentLSB / 1023.0, ipid);
    } else if(MODE_CV == mode) {
        pidIteration((float) voltageLSB / 1023.0, vpid);
    }
}

uint16_t Psu::getCurrent() {
    return current;
}

uint16_t Psu::getVoltage() {
    return voltage;
}

int16_t Psu::getControllSignal() {
    return controlSignal;
}

uint8_t Psu::getMode() {
    return mode;
}

/* Private methods */

void Psu::pidIteration(float processVariable, PID *gains) {
    float error = setPoint - processVariable;
    float ctrl = error * gains->P + errIntegral;

    controlSignal = ctrl * 255;

    //~ Serial.print(" sp:");
    //~ Serial.print(setPoint * 10000);
    //~ Serial.print(" pv:");
    //~ Serial.print(processVariable * 10000);
    //~ Serial.print(" er:");
    //~ Serial.print(error * 10000);
    //~ Serial.print(" ctrl:");
    //~ Serial.print(ctrl * 10000);
    //~ Serial.print(" ctrl:");
    //~ Serial.println(controlSignal);
    
    if(controlSignal < 255) {
        errIntegral += error * gains->I;
    }

    controll(controlSignal);
}

void Psu::off() {
    controll(0);
}

void Psu::controll(int value) {
    value = constrain(value, 0, 255);
    analogWrite(dacOutPin, value);
}
