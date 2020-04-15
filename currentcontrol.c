#include "currentcontrol.h"
#include "utilities.h"
#include "NU32.h"
#include "isense.h"

#define PERIOD_5HZ 62499    // PR = (desiredPeriod / prescaler * 80 MHz) - 1, prescaler 256
#define PERIOD_5KHZ 1999    // PR = (desiredPeriod / prescaler * 80 MHz) - 1, prescaler 8
#define PERIOD_20KHZ 3999   // PR = (desiredPeriod / prescaler * 80 MHz) - 1


// Variables needed in ISR, so volitile
static volatile float kp;
static volatile float ki;
//static volatile float kd;
static volatile float refCurrent = 0.0;
static volatile float actualCurrent = 0.0;


int setPWM(int dutyCycle) {
    int reverse = 0;  // default is forward

    if (dutyCycle > 100 || dutyCycle < -100)
        return 0;   // failure
    
    if (dutyCycle < 0)
    {
        reverse = 1;      // negative direction
        dutyCycle *= -1;    // make duty cycle positive
    }

    LATDbits.LATD1 = reverse;
    
    OC1RS = (int)((float)dutyCycle/100.0*(float)(PERIOD_20KHZ+1));
    setMode(PWM);

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
    LATDbits.LATD1 = 0;     // positive direction. reverse is false
}

void currentControlInit()
{
    // Timer1 setup

    PR1 = PERIOD_5KHZ;              //              set period register
    TMR1 = 0;                       //              initialize count to 0
    T1CONbits.ON = 1;               //              turn on Timer1
    T1CONbits.TCKPS = 0b01;          // 1:8 prescaler select
    //T1CONbits.TCKPS = 0b11;        // 256 prescaler. ONLY FOR TESTING!!

    IPC1bits.T1IP = 6;              // INT step 4: priority
    IPC1bits.T1IS = 0;              //             subpriority
    IFS0bits.T1IF = 0;              // INT step 5: clear interrupt flag
    IEC0bits.T1IE = 1;              // Enable interrupt
}

void __ISR(_TIMER_1_VECTOR, IPL6SOFT) currentCtrlLoop(void) 
{
    actualCurrent = readCurrent();  // get filtered current reading
    switch(getMode())
    {
        case IDLE:
        {
            OC1RS = 0;  // set duty cycle to 0
            break;
        }

        case PWM:
        {
            // do nothing. The setPWM function sets the duty cycle
            break;
        }

        case ITEST:
        {
            static int count = 0;
            if (count == 0)
            {
                refCurrent = 200.0;
            }
            else if (count >= 99)
            {
                setMode(IDLE);
                count = 0;
                //sendDataToMatlab()
            }
            
            else if (count % 25 == 0)    // execute conditional when count is at 25, 50, and 75
            {
                refCurrent *= -1.0;
            }

            // PI control
            static float ePrev = 0;      // error in previous iteration
            static float eInt = 0;       // initial error integral

            float e = refCurrent - actualCurrent;    // error in mA
            eInt = eInt + e;                  // calculate new error integral
            float u = kp*e + ki*eInt;     // PI control signal
            
            
            //savaDataToArray()

            //OC1RS = u/maxu*(PERIOD_20KHZ+1);    // OC1RS = (duty cycle)*(PR2+1)

            count++;

            break;
        }

        default:
        {

            break;
        }

    }

    IFS0bits.T1IF = 0;  // clear interrupt flag
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

void setRefCurrent(float newCurrent) 
{
    refCurrent = newCurrent;
}

float getSampledCurrent() {
    return actualCurrent;
}
