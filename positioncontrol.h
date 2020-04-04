#ifndef POSITIONCONTROL_H_
#define POSITIONCONTROL_H_
#include "utilities.h"

int setPositionGains(int kp, int kd, int ki);
PIDInfo getPositionGains();
int loadTrajectory(float trajectory[]);

#endif