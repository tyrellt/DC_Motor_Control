#include "currentcontrol.h"
#include "utilities.h"
#include "NU32.h"
#include "isense.h"

#define PERIOD_5HZ 62499    // PR = (desiredPeriod / prescaler * 80 MHz) - 1, prescaler 256
#define PERIOD_5KHZ 1999    // PR = (desiredPeriod / prescaler * 80 MHz) - 1, prescaler 8
#define PERIOD_20KHZ 3999   // PR = (desiredPeriod / prescaler * 80 MHz) - 1

#define REVERSE_BIT LATDbits.LATD1
#define MAX_U 100.0
#define EINT_MAX 3333   // MAX_U / ki where ki = 0.03 (default)


// Variables needed in ISR, so volitile
static volatile float kp;
static volatile float ki;
//static volatile float kd;
static volatile float refCurrent = 0.0;
static volatile float actualCurrent = 0.0;
static volatile float rawCurrent = 0.0;

// for sample testing
static volatile float testSamples[MAX_TEST_SAMPLES];
static volatile float testRef[MAX_TEST_SAMPLES];
static volatile int numTestSamples = 0;

static void PICalculation(float error)
{
    static float eInt = 0.0;       // initial error integral
    eInt += error;                  // calculate new error integral
    if (eInt > EINT_MAX)
    {
        eInt = EINT_MAX;
    }
    else if (eInt < -EINT_MAX)
    {
        eInt = -EINT_MAX;
    }
    
    float u = kp*error + ki*eInt;     // PI control signal

    int isReverse = 0;
    if (u < 0.0)
    {
        isReverse = 1;
        u *= -1;    // make u positive
    }

    if (u > MAX_U)
    {
        u = MAX_U;    // saturate at MAX_U
    }

    REVERSE_BIT = isReverse;

    // Update duty cycle
    OC1RS = (u/MAX_U)*(PERIOD_20KHZ+1);    // OC1RS = (duty cycle)*(PR2+1)
}


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


    // Output Compare setup
    OC1CONbits.OCM = 0b110;  // PWM mode without fault pin; other OC1CON bits are defaults
    OC1CONbits.OCTSEL = 0;   // Timer2 selected as clock source
    OC1RS = 0.0*(PERIOD_20KHZ+1);            // OC1RS = (duty cycle)*(PR2+1)
    OC1R = 0.0*(PERIOD_20KHZ+1);              // initialize before turning OC1 on; afterward it is read-only
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

    IPC1bits.T1IP = 6;              // INT step 4: priority
    IPC1bits.T1IS = 0;              //             subpriority
    IFS0bits.T1IF = 0;              // INT step 5: clear interrupt flag
    IEC0bits.T1IE = 1;              // Enable interrupt
    
    // Decent default values for a decent tracking.
    kp = 0.3;
    ki = 0.03;

    PWMInit();
}

void __ISR(_TIMER_1_VECTOR, IPL6SOFT) currentCtrlLoop(void) 
{
    actualCurrent = readCurrent(1);  // get filtered current reading
    //rawCurrent = readCurrent(0);
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
                REVERSE_BIT = 0;
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
            
            float e = refCurrent - actualCurrent;    // error in mA
            // store actual and reference data
            testSamples[count] = actualCurrent;
            testRef[count] = refCurrent;

            PICalculation(e);

            count++;

            break;
        }

        case HOLD:
        case TRACK:     // behavior is identical for HOLD and TRACK modes
        {
            float e = refCurrent - actualCurrent;
            PICalculation(e);
            
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

void getITestSample(int index, float *sample, float *ref)
{
    *sample = testSamples[index];
    *ref = testRef[index];
}
