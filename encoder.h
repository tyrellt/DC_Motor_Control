#ifndef ENCODER_H_
#define ENCODER_H_

#define COUNTS_PER_REV 4733     // if this changes, change COUNTS_TO_DEG accordingly
#define COUNTS_TO_DEG 0.07606   // 360 / COUNTS_PER_REV. If this changes, change COUNTS_PER_REV

void encoderInit();
int readEncoderCounts();
float readEncoder();
void resetEncoder();

#endif