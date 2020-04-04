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
  // in future, initialize modules or peripherals here
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
        sprintf(buffer,"%f\r\n", readCurrent());
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
        int kp = 0;
        int ki = 0;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%d %d", &kp, &ki);
        int status = setCurrentGains(kp, ki);
        sprintf(buffer,"%d\r\n", status);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'h':
      {
        // Get current gains
        PIDInfo gains = getCurrentGains();
        sprintf(buffer, "%d %d\r\n", gains.kp, gains.ki);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'i':
      {
        // Set position gains
        int kp = 0;
        int kd = 0;
        int ki = 0;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%d %d %d", &kp, &kd, &ki);
        int status = setPositionGains(kp, kd, ki);
        sprintf(buffer,"%d\r\n", status);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'j':
      {
        // Get position gains
        PIDInfo gains = getPositionGains();
        sprintf(buffer, "%d %d %d\r\n", gains.kp, gains.kd, gains.ki);
        NU32_WriteUART3(buffer);
        break;
      }

      case 'k':
      {
        // Test current control
        setMode(ITEST);
        
        break;
      }

      case 'l':
      {
        // Go to angle (deg)
        setMode(HOLD);
        float angle = 0.0;
        NU32_ReadUART3(buffer, BUF_SIZE);
        sscanf(buffer, "%f", &angle);
        // Call some setAngle() function
        sprintf(buffer, "%f\r\n", angle); //only for testing
        NU32_WriteUART3(buffer);
        break;
      }

      case 'm':
      {
        // Load step trajectory
        
        break;
      }

      case 'n':
      {
        // Load cubic trajectory
        break;
      }

      case 'o':
      {
        // Execute trajectory
        setMode(TRACK);
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
        sscanf(buffer, "%d %d", &n1, &n2);
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