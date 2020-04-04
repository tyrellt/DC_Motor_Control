#ifndef UTILITIES_H_
#define UTILITIES_H_

typedef struct {
    int kp;
    int kd;
    int ki;
} PIDInfo;

typedef enum {
    IDLE = 0,
    PWM = 1,
    ITEST = 2,
    HOLD = 3,
    TRACK = 4
} OpMode;

void setMode(OpMode mode);
OpMode getMode();

#endif