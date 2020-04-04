#include "currentcontrol.h"

static PIDInfo currentGains;

int setPWM(int dutyCycle) {
    if (dutyCycle > 100 || dutyCycle < -100)
        return 0;   // failure
    else
        return 1;   // success
}
int setCurrentGains(int kp, int ki) { 
    PIDInfo gains;
    gains.kp = kp;
    gains.ki = ki;
    currentGains = gains; 
    return kp + ki;
}

PIDInfo getCurrentGains() { return currentGains; }