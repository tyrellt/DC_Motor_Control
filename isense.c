#include "isense.h"
#include <xc.h>
#include "fir.h"

#define FILTER_ORDER 2
#define COUNTS_AT_0MA 507
#define MA_PER_COUNT 4.7433

static firFilter filter;


unsigned int readCurrentCounts()
{
    // assume manual sampling, automatic conversion
    AD1CHSbits.CH0SA = 9; // connect chosen pin to MUXA for sampling
    AD1CON1bits.SAMP = 1; // start sampling

    while (!AD1CON1bits.DONE) {
        ; // wait for the conversion process to finish
    }
    return ADC1BUF0; // read the buffer with the result
}

// returns current in mA
float readCurrent(int shouldFilter)
{
    float filteredAvg;
    if (shouldFilter)
    {
        filteredAvg = applyFIR(&filter, readCurrentCounts());     // filter reading with 200 Hz cutoff
    }
    else
    {
        filteredAvg = readCurrentCounts();
    }
    
    return (filteredAvg - COUNTS_AT_0MA) * MA_PER_COUNT;   // Equation from linear regression test
}

void iSenseInit() 
{
    AD1PCFGbits.PCFG9 = 0;                  // Set AN9 as an adc pin
    AD1CON3bits.ADCS = 2;                   // ADC clock period is Tad = 2*(ADCS+1)*Tpb =
                                            //                           2*3*12.5ns = 75ns
    AD1CON1bits.SSRC = 0b111;               // Automatic conversion
    AD1CON1bits.ASAM = 0;                   // Manual Sampling
    AD1CON1bits.ADON = 1;                   // turn on A/D converter

    float b[FILTER_ORDER + 1] = {0.0683, 0.8633, 0.0683};   // coefficients of 2nd order low pass with 200 Hz cutoff
        
    firInit(&filter, b, FILTER_ORDER);
}
