#include "positioncontrol.h"
#include <stdlib.h>

static PIDInfo posGains;
static int trajectory[2000]; // TODO: I feel like this is a waste of memory

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

int setTrajSize(int size) {
    //trajectory  = (int*)malloc(1 * sizeof(int));
    //if (trajectory == NULL) {
    //    return -1;
    //}
    return size;
}

int addTrajPoint(int point, int index)
{
    trajectory[index] = point;
    return trajectory[index];
}
