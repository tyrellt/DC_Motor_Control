#ifndef FIR_H_
#define FIR_H_

#define MAX_FILTER_ORDER 50

typedef struct {
    int p;  // filter order
    float b[MAX_FILTER_ORDER + 1];  // filter coefficients
    float sBuffer[MAX_FILTER_ORDER + 1]; // buffer of p+1 past samples, including current sample
} firFilter;

void firInit(firFilter *filter, float *coefficients, int order);
void shiftInSample(firFilter *filter, float sample);
float applyFIR(firFilter *filter, float sample);

#endif