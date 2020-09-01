# DC_Motor_Control

This project was done as part of the requirements of ME EN 6240 Advanced Mechatronics at the University of Utah.

The purpose of the project is to perform position control on a brushed DC motor using a PIC32 microcontroller. Bootloader code for PIC provided by Northwestern Robotics from Northwestern University (see NU32.org). 

Control hardware consists of a rotary encoder, a current sensor, and an H-bridge for driving the motor.

Closed loop PI current control is performed in a 5 kHz ISR using the current sensor. The current reading is read by the PIC as an analog signal.
Closed loop PID position control is performed at 200 Hz using the encoder feedback. The encoder value is read from the encoder chip using SPI. 
A MATLAB client with a simple UI is used to talk to the PIC via UART. The client also handles trajectory generation and plotting of data.
