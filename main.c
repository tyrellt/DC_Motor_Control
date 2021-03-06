#include "NU32.h"          // config bits, constants, funcs for startup and UART
#include <stdio.h>
#include "currentcontrol.h"
#include "positioncontrol.h"
#include "isense.h"
#include "encoder.h"
#include "utilities.h"

#define BUF_SIZE 200

int main() 
{
  char buffer[BUF_SIZE];
  NU32_Startup(); // cache on, min flash wait, interrupts on, LED/button init, UART init
  NU32_LED1 = 1;  // turn off the LEDs
  NU32_LED2 = 1;

  setMode(IDLE);

  __builtin_disable_interrupts();
  // initialize modules or peripherals
  encoderInit();
  positionCtrlInit();
  iSenseInit();
  currentControlInit();

  __builtin_enable_interrupts();

  while(1)
  {
    NU32_ReadUART3(buffer,BUF_SIZE); // we expect the next character to be a menu command
    NU32_LED2 = 1;                   // clear the error LED
    switch (buffer[0]) {
      case 'a':
      {
        // Read current sensor (ADC counts)
        sprintf(buffer,"%d\r\n", readCurrentCounts());
        NU32_WriteUART3(buffer);
        break;
      }

      case 'b':
      {
        // Read current sensor (mA)
        sprintf(buffer,"%f\r\n", getSampledCurrent());
        NU32_WriteUART3(buffer);
        break;
      }

      case 'c':
      {
        // Read encoder (counts)
        sprintf(buffer,"%d\r\n", readEncoderCounts());
        NU32_WriteUART3(buffer);
        break;
      }

      case 'd':
      {
        // Read encoder (deg)
        sprintf(buffer,"%f\r\n", readEncoder());
        NU32_WriteUART3(buffer);
        break;
      }

      case 'e':
      {
        // Reset encoder
        resetEncoder();
        // send counts to user to confirm reset
        sprintf(buffer,"%d\r\n", readEncoderCounts());
        NU32_WriteUART3(buffer);

        break;
      }

      case 'f':
      {
        // Set PWM (-100 to 100)
        int dutyCycle = 0;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%d", &dutyCycle);

        int status = setPWM(dutyCycle);

        sprintf(buffer,"%d\r\n", status);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'g':
      {
        // Set current gains
        float kp = 0;
        float ki = 0;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f %f", &kp, &ki);
        float sum = setCurrentGains(kp, ki); // send back sum to confirm they were set correctly
        sprintf(buffer,"%f\r\n", sum);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'h':
      {
        // Get current gains
        PIDInfo gains = getCurrentGains();
        sprintf(buffer, "%f %f\r\n", gains.kp, gains.ki);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'i':
      {
        // Set position gains
        float kp = 0;
        float kd = 0;
        float ki = 0;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f %f %f", &kp, &kd, &ki);

        float sum = setPositionGains(kp, kd, ki); // send back sum to confirm they were set correctly
        sprintf(buffer,"%f\r\n", sum);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'j':
      {
        // Get position gains
        PIDInfo gains = getPositionGains();
        sprintf(buffer, "%f %f %f\r\n", gains.kp, gains.kd, gains.ki);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'k':
      {
        // Test current control
        setMode(ITEST);
        
        while(getMode() == ITEST);  // wait for test to finish

        // send samples to client
        int i;
        float sample;
        float refSignal;
        sprintf(buffer, "%d\r\n", 100);   // ITEST is hardcoded for 100 samples (.02 seconds)
        NU32_WriteUART3(buffer);          // send number of samples to client
        for (i = 0; i < 100; i++)
        {
          getITestSample(i, &sample, &refSignal);
          sprintf(buffer, "%f %f\r\n", sample, refSignal);
          NU32_WriteUART3(buffer);
        }
        
        break;
      }

      case 'l':
      {
        // Go to angle (deg)
        float angle = 0.0;
        NU32_ReadUART3(buffer, BUF_SIZE);
        sscanf(buffer, "%f", &angle);
        
        resetEncoder();     // hold angle will be measured from current position
        setHoldAngle(angle);
        setMode(HOLD);

        sprintf(buffer, "%f\r\n", angle); //only for testing
        NU32_WriteUART3(buffer);
        break;
      }

      case 'm':
      case 'n':
      {
        // Load step or cubic trajectory
        int numSamples = 0;
        NU32_ReadUART3(buffer, BUF_SIZE);
        sscanf(buffer, "%d", &numSamples);
        if (numSamples > 0) {
          setTrajLength(numSamples);

          float sample = 0.0;
          int i;
          for (i = 0; i < numSamples; i++)
          {
            NU32_ReadUART3(buffer, BUF_SIZE);
            sscanf(buffer, "%f", &sample);
            addTrajPoint(sample, i);
          }
        }
        break;
      }

      case 'o':
      {
        // Execute trajectory
        resetEncoder();
        setMode(TRACK);

        while(getMode() == TRACK);  // wait for test to finish

        // send samples to client
        int i;
        float sample;
        float refSignal;
        int length = getTrajLength();
        sprintf(buffer, "%d\r\n", length);   // send number of samples to client
        NU32_WriteUART3(buffer);
        for (i = 0; i < length; i++)
        {
          getTrajSample(i, &sample, &refSignal);
          sprintf(buffer, "%f %f\r\n", sample, refSignal);
          NU32_WriteUART3(buffer);
        }


        break;
      }

      case 'p':
      {
        // Unpower the motor
        setMode(IDLE);
        break;
      }

      case 'q':
      {
        // handle q for quit
        setMode(IDLE);
        break;
      }
      
      case 'r':
      {
        // Get mode
        sprintf(buffer, "%d \r\n", getMode());
        NU32_WriteUART3(buffer);
        break;
      }

      case 'y': // another dummy command
      {
        int n1, n2 = 0;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%d", &n1);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%d", &n2);
        sprintf(buffer,"%d\r\n", n1 + n2); // return the two numbers added together
        NU32_WriteUART3(buffer);
        break;
      }
      
      default:
      {
        NU32_LED2 = 0;  // turn on LED2 to indicate an error
        break;
      }
    }
  }
  return 0;
}