#include "currentcontrol.h"

// Variables needed in ISR, so volitile
static volatile float kp;
static volatile float ki;
//static volatile float kd;

int setPWM(int dutyCycle) {
    if (dutyCycle > 100 || dutyCycle < -100)
        return 0;   // failure
    else
        return 1;   // success
}
float setCurrentGains(float newKp, float newKi) 
{ 
    kp = newKp;
    ki = newKi;
    return kp + ki;   // successfully set
}

PIDInfo getCurrentGains() 
{ 
    PIDInfo currentGains;
    currentGains.kp = kp;
    currentGains.ki = ki;

    return currentGains;
}