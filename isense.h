#ifndef ISENSE_H_
#define ISENSE_H_

unsigned int readCurrentCounts();

// Returns current in mA
float readCurrent(int shouldFilter);
void iSenseInit();

#endif
