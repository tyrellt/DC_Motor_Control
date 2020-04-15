#include "positioncontrol.h"

#include <stdlib.h>
#include <xc.h>
#include <sys/attribs.h>            // __ISR macro

#include "encoder.h"

#define PERIOD_200HZ 399999 // PR = (desiredPeriod / prescaler * 80 MHz) - 1
                            //  desired period is 1/200Hz = 0.005, prescaler = 1

// Variables needed in ISR, so volitile
static volatile float kp;
static volatile float ki;
static volatile float kd;
static volatile float holdAngle;      // desired hold angle (set by user)

static int trajectory[2000]; // TODO: I feel like this is a waste of memory

int positionCtrlInit()
{
    // Timer45 setup. Config for combined timer is done in Timer4 registers,
    //  but interrupt is triggered by Timer5 overflow

    T4CONbits.T32 = 1;              //              Chain timers 4 and 5 together
    PR4 = PERIOD_200HZ;             //              set period register
    TMR4 = 0;                       //              initialize count to 0
    T4CONbits.ON = 1;               //              turn on Timer4
    
    IPC5bits.T5IP = 5;              // INT step 4: priority
    IPC5bits.T5IS = 0;              //             subpriority
    IFS0bits.T5IF = 0;              // INT step 5: clear interrupt flag
    IEC0bits.T5IE = 1;              // Enable interrupt
}

void __ISR(_TIMER_5_VECTOR, IPL5SOFT) positionCtrlLoop(void) 
{
    static float ePrev = 0;      // error in previous iteration
    static float eInt = 0;       // initial error integral
    // TODO: reset these values after a test is done...

    if (getMode() == HOLD)
    {
        // PID control
        float e = holdAngle - readEncoder();    // error in degrees
        float edot = e - ePrev;                 // calculate derivative
                                                // TODO: Add filtering on derivative term
        eInt = eInt + e;                  // calculate new error sum
        float u = kp*e + ki*eInt + kd*edot;     // PID control signal
        
        // some setCurrent(u) function

        ePrev = e;
    }

    IFS0bits.T5IF = 0;  // clear interrupt flag

}

float setPositionGains(float newKp, float newKd, float newKi) 
{ 
    kp = newKp;
    kd = newKd;
    ki = newKi;
    return kp + ki + kd;   // successfully set
}

PIDInfo getPositionGains() 
{ 
    PIDInfo posGains;
    posGains.kp = kp;
    posGains.ki = ki;
    posGains.kd = kd;

    return posGains;
}

int addTrajPoint(int point, int index)
{
    trajectory[index] = point;
    return trajectory[index];
}

void setHoldAngle(float angle)  //angle parameter is in degrees
{
    holdAngle = angle;
}
