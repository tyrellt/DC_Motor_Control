#ifndef CURRENTCONTROL_H_
#define CURRENTCONTROL_H_

#include "utilities.h"

int setPWM(int dutyCycle);
int setCurrentGains(int kp, int ki);
PIDInfo getCurrentGains();


#endif