#ifndef POSITIONCONTROL_H_
#define POSITIONCONTROL_H_
#include "utilities.h"

// Initializes timer for 200 Hz interrupt,
int positionCtrlInit();

float setPositionGains(float kp, float kd, float ki);
PIDInfo getPositionGains();
float addTrajPoint(float point, int index);
void setHoldAngle(float angle);



#endif
