#include "fir.h"

void firInit(firFilter *filter, float *coefficients, int order)
{
    filter->p = order;
    int i;
    for (i = 0; i < filter->p + 1; i++)
    {
        filter->b[i] = coefficients[i];
        filter->sBuffer[i] = 0.0;    // initialize all previous samples to 0
    }
    filter->isFirstSample = 1;

}

// Add new sample to buffer and shift out oldest sample
void shiftInSample(firFilter *filter, float sample)
{
    int i;

    for (i = filter->p; i > 0; i--)  // start at oldest sample and shift (bigger array index = older sample)
    {
        if (filter->isFirstSample)
        {
            filter->sBuffer[i] = 150;    // if this is the first sample, initialize entire buffer to match sample
        }
        else
        {
            filter->sBuffer[i] = filter->sBuffer[i - 1];
        }
    }
    filter->isFirstSample = 0;       // The next sample will not be the first sample
    filter->sBuffer[0] = sample;     // put in newest sample in posiiton 0
}

// Apply FIR summation using samples from buffer and filter coefficients
float applyFIR(firFilter *filter, float sample)
{
    shiftInSample(filter, sample);
    float output = 0.0;
    int i;
    for (i = 0; i < filter->p + 1; i++)
    {
        output += filter->b[i] * filter->sBuffer[i];
    }

    return output;
}