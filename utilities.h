#ifndef UTILITIES_H_
#define UTILITIES_H_

#define MOTOR_SERVO_RATE 200
#define MAX_TRAJ_SAMPLES = 10 * MOTOR_SERVO_RATE    // Max trajectory duration is 10 seconds
 
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