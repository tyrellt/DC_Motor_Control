#include "currentcontrol.h"
#include <xc.h>

#define PERIOD_5HZ 62499    // PR = (desiredPeriod / prescaler * 80 MHz) - 1

#define PERIOD_20KHZ 3999   // PR = (desiredPeriod / prescaler * 80 MHz) - 1


// Variables needed in ISR, so volitile
static volatile float kp;
static volatile float ki;
//static volatile float kd;

int setPWM(int dutyCycle) {
    int direction = 1;  // default direction

    if (dutyCycle > 100 || dutyCycle < -100)
        return 0;   // failure
    
    if (dutyCycle < 0)
    {
        direction = 0;      // negative direction
        dutyCycle *= -1;    // make duty cycle positive
    }

    LATDbits.LATD1 = direction;
    
    OC1RS = (int)((float)dutyCycle/100.0*(float)(PERIOD_20KHZ+1));
    return 1;   // success
}

void PWMInit()
{
    // Timer2 setup
    PR2 = PERIOD_20KHZ;               //              set period register
    TMR2 = 0;                       //              initialize count to 0
    T2CONbits.ON = 1;               //              turn on Timer2
    //T2CONbits.TCKPS = 0b111;        // 256 prescaler. ONLY FOR TESTING!!

    // Output Compare setup
    OC1CONbits.OCM = 0b110;  // PWM mode without fault pin; other OC1CON bits are defaults
    OC1CONbits.OCTSEL = 0;   // Timer2 selected as clock source
    OC1RS = .5*(PERIOD_20KHZ+1);            // OC1RS = (duty cycle)*(PR2+1)
    OC1R = .5*(PERIOD_20KHZ+1);              // initialize before turning OC1 on; afterward it is read-only
    OC1CONbits.ON = 1;       // turn on OC1

    // Initialize motor direction bit
    TRISDbits.TRISD1 = 0;   // set RD0 as digital output
    LATDbits.LATD1 = 1;     // positive direction
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
