#include "positioncontrol.h"

static PIDInfo posGains;
static int* trajectory;

int setPositionGains(int kp, int kd, int ki) 
{ 
    PIDInfo gains;
    gains.kp = kp;
    gains.kd = kd;
    gains.ki = ki;
    posGains = gains; 
    return kp + kd + ki;
}

PIDInfo getPositionGains() { return posGains; }

void setTrajSize(int size) {
    trajectory  = (int*)malloc(size * sizeof(int));
}
