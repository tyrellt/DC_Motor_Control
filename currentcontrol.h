#ifndef CURRENTCONTROL_H_
#define CURRENTCONTROL_H_

#include "utilities.h"

int setPWM(int dutyCycle);
float setCurrentGains(float kp, float ki);
PIDInfo getCurrentGains();


#endif