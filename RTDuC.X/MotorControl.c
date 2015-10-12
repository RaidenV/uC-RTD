#include <pic18f8722.h>

void MotorDriverInit(void)
{
    TRISGbits.RG0 = 0; //ECCP3 Enhanced PWM output Channel: A, AHA (MOSFET Driver Chip)
    TRISGbits.RG3 = 0; //ECCP3 Enhanced PWM output Channel: D, ALB (MOSFET Driver Chip)
    TRISEbits.RE3 = 0; //ECCP3 Enhanced PWM output Channel: C, ALA (MOSFET DRIVER CHIP)
    TRISEbits.RE4 = 0; //ECCP3 Enhanced PWM output Channel: B, AHB (MOSFET Driver Chip)
    TRISBbits.RB0 = 1; //FAULT Pin Falling-edge Interrupt (MOSFET Dr)

    CCP3CON = 0x4C; //Set Full-bridge mode, all outputs active high;
    PR2 = 0xFF; //With a prescaler of 1:1 on Timer2, this yields a PWM frequency of 39.06 kHz;
    T3CON = 0x00; //Set Timer1 and Timer2 as the clock sources for all PWM activities;
    T2CON = 0x04; //Timer2 is set with neither prescaler nor postscaler;            

}

void ImplementPIDMotion(int);
