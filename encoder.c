#include "encoder.h"
#include <xc.h>

#define COUNTS_PER_REV 4733

static int encoderCommand(int read) { // send a command to the encoder chip
                                       // 0 = reset count to 32,768, 1 = return the count
  SPI4BUF = read;                      // send the command
  while (!SPI4STATbits.SPIRBF) { ; }   // wait for the response
  SPI4BUF;                             // garbage was transferred, ignore it
  SPI4BUF = 5;                         // write garbage, but the read will have the data
  while (!SPI4STATbits.SPIRBF) { ; }
  SPI4BUF;
  return SPI4BUF;
}

void encoderInit(void) {
  // SPI initialization for reading from the decoder chip
  SPI4CON = 0;              // stop and reset SPI4
  SPI4BUF;                  // read to clear the rx receive buffer
  SPI4BRG = 0x4;            // bit rate to 8 MHz, SPI4BRG = 80000000/(2*desired)-1
  SPI4STATbits.SPIROV = 0;  // clear the overflow
  SPI4CONbits.MSTEN = 1;    // master mode
  SPI4CONbits.MSSEN = 1;    // slave select enable
  SPI4CONbits.MODE16 = 1;   // 16 bit mode
  SPI4CONbits.MODE32 = 0; 
  SPI4CONbits.SMP = 1;      // sample at the end of the clock
  SPI4CONbits.ON = 1;       // turn SPI on
}

int readEncoderCounts() 
{ 
    // read counts from dsPIC through SPI
    encoderCommand(1);  // do an extra read because of bug in dsPIC
    return encoderCommand(1);
}

float readEncoder()
{
    int counts = readEncoderCounts();
    // convert counts to degrees and return

    return (counts - 32768) / (float)COUNTS_PER_REV * 360.0;  // 4 is for quadrature encoder
}

void resetEncoder()
{
    // send command to reset encoder
    encoderCommand(0);

}
