#ifndef CURRENTCONTROL_H_
#define CURRENTCONTROL_H_

#include "utilities.h"

#define MAX_TEST_SAMPLES 1000

int setPWM(int dutyCycle);
void PWMInit();
void currentCntlInit();
float setCurrentGains(float newKp, float newKi);
PIDInfo getCurrentGains();
void setRefCurrent(float newCurrent);

// returns filtered current sampled at 5kHz in current control loop
float getSampledCurrent();

// functions for sample testing
void setNumTestSamples(int n);
void getTestSample(int index, float *sample, float *ref);
void getFullTestSample(int index, float *sample, float *ref, float *u, float *e);



#endif