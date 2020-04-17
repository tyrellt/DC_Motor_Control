#include "currentcontrol.h"
#include "utilities.h"
#include "NU32.h"
#include "isense.h"
#include <math.h>

#define PERIOD_5HZ 62499    // PR = (desiredPeriod / prescaler * 80 MHz) - 1, prescaler 256
#define PERIOD_5KHZ 1999    // PR = (desiredPeriod / prescaler * 80 MHz) - 1, prescaler 8
#define PERIOD_20KHZ 3999   // PR = (desiredPeriod / prescaler * 80 MHz) - 1

#define REVERSE_BIT LATDbits.LATD1
#define MAX_U 100.0


// Variables needed in ISR, so volitile
static volatile float kp;
static volatile float ki;
//static volatile float kd;
static volatile float refCurrent = 0.0;
static volatile float actualCurrent = 0.0;

// for sample testing
static float testSamples[MAX_TEST_SAMPLES];
static float testRef[MAX_TEST_SAMPLES];
static volatile int numTestSamples = 0;


int setPWM(int dutyCycle) {
    int isReverse = 0;  // default is forward

    if (dutyCycle > 100 || dutyCycle < -100)
        return 0;   // failure
    
    if (dutyCycle < 0)
    {
        isReverse = 1;      // negative direction
        dutyCycle *= -1;    // make duty cycle positive
    }

    REVERSE_BIT = isReverse;
    
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
    REVERSE_BIT = 0;     // positive direction. reverse is false

    // Initialize H-Bridge mode bit
    TRISDbits.TRISD2 = 0;   // set RD2 as digital output
    LATDbits.LATD2 = 1;     // set it high, because H-Bridge mode should always be 1
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
                break;
            }
            
            else if (count % 25 == 0)    // execute conditional when count is at 25, 50, and 75
            {
                refCurrent *= -1.0;
            }

            // PI control
            static float eInt = 0;       // initial error integral

            float e = refCurrent - actualCurrent;    // error in mA
            eInt = eInt + e;                  // calculate new error integral
            float u = kp*e + ki*eInt;     // PI control signal
            
            int isReverse = 0;
            if (u < 0.0)
            {
                isReverse = 1;
                u *= -1;    // make u positive
            }

            if (u > MAX_U)
            {
                u = MAX_U;    // saturate at 1.0;
            }

            REVERSE_BIT = isReverse;
            OC1RS = u/MAX_U*(PERIOD_20KHZ+1);    // OC1RS = (duty cycle)*(PR2+1)

            // store actual and reference data
            testSamples[count] = actualCurrent;
            testRef[count] = refCurrent;

            count++;

            break;
        }

        case SAMPLE:
        {
            static int iCurrentSample = 0;
            if (iCurrentSample == 5000)
            {
                OC1RS = 1*(PERIOD_20KHZ+1);
                REVERSE_BIT = 0;
            }
            // else if (iCurrentSample == 3000)
            // {
            //     OC1RS = 0.5*(PERIOD_20KHZ+1);
            //     REVERSE_BIT = 1;
            // }
            // else if (iCurrentSample == 4000)
            // {
            //     OC1RS = 0.07*(PERIOD_20KHZ+1);
            //     REVERSE_BIT = 1;
            // }
            // else if (iCurrentSample == 5000)
            // {
            //     OC1RS = 0.07*(PERIOD_20KHZ+1);
            //     REVERSE_BIT = 0;
            // }
            // else if (iCurrentSample == 6000)
            // {
            //     OC1RS = 0.2*(PERIOD_20KHZ+1);
            //     REVERSE_BIT = 0;
            // }
            // else if (iCurrentSample == 7000)
            // {
            //     OC1RS = 0.7*(PERIOD_20KHZ+1);
            //     REVERSE_BIT = 0;
            // }
            // else if (iCurrentSample == 7500)
            // {
            //     OC1RS = 0.8*(PERIOD_20KHZ+1);
            //     REVERSE_BIT = 0;
            // }
            // else if (iCurrentSample == 8000)
            // {
            //     OC1RS = 0.95*(PERIOD_20KHZ+1);
            //     REVERSE_BIT = 0;
            // }
            // else if (iCurrentSample == 8500)
            // {
            //     OC1RS = 0.99*(PERIOD_20KHZ+1);
            //     REVERSE_BIT = 0;
            // }
            // else if (iCurrentSample == 9000)
            // {
            //     OC1RS = 1*(PERIOD_20KHZ+1);
            //     REVERSE_BIT = 0;
            // }

            if (iCurrentSample < numTestSamples)
            {
                testSamples[iCurrentSample] = actualCurrent;
                iCurrentSample++;
            }
            else
            {
                setMode(IDLE);
                iCurrentSample = 0;
            }
            
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

void setNumTestSamples(int n)
{
    numTestSamples = n;
}

void getTestSample(int index, float *sample, float *ref)
{
    *sample = testSamples[index];
    *ref = testRef[index];
}
