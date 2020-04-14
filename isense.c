#include "isense.h"
#include <xc.h>

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

float readCurrent()
{
    int total = 0;
    int i;
    int numSamples = 5;
    for (i = 0; i < numSamples; i++)
    {
        total += readCurrentCounts();
    }

    float average = (float)total / (float)numSamples;
    return 4.2286 * average - 2152.3;   // Equation from linear regression test
}

void iSenseInit() 
{
    AD1PCFGbits.PCFG9 = 0;                 // Set AN9 as an adc pin
    AD1CON3bits.ADCS = 2;                   // ADC clock period is Tad = 2*(ADCS+1)*Tpb =
                                          //                           2*3*12.5ns = 75ns
    AD1CON1bits.SSRC = 0b111;               // Automatic conversion
    AD1CON1bits.ASAM = 1;                   // Auto Sampling
    AD1CON1bits.ADON = 1;                   // turn on A/D converter
}
