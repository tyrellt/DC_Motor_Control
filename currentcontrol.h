#ifndef CURRENTCONTROL_H_
#define CURRENTCONTROL_H_

#include "utilities.h"

int setPWM(int dutyCycle);
void PWMInit();
void currentCntlInit();
float setCurrentGains(float kp, float ki);
PIDInfo getCurrentGains();
void setRefCurrent(float newCurrent);

// returns filtered current sampled at 5kHz in current control loop
float getSampledCurrent();



#endif