#include "MotorControl.h"

unsigned char PIDEnableFlag;
int DeadbandLow = -200; //Joystick deadband variables;
int DeadbandHigh = 200;

void MotorDriverInit(void)
{
    TRISGbits.RG0 = 0; //ECCP3 Enhanced PWM output Channel: A, AHI (MOSFET Driver Chip)
    TRISEbits.RE4 = 0; //ECCP3 Enhanced PWM output Channel: B, BHI (MOSFET Driver Chip)
    TRISEbits.RE3 = 0; //ECCP3 Enhanced PWM output Channel: C, ALO (MOSFET Driver Chip)
    TRISGbits.RG3 = 0; //ECCP3 Enhanced PWM output Channel: D, BLO (MOSFET Driver Chip)

    TRISBbits.RB0 = 1; //FAULT Pin Falling-edge Interrupt (MOSFET Dr)

    TRISAbits.RA4 = 0; //Motor Fault LED;

    CCPR3L = 0x00; //Start with the motors off (set all values to zero);
    CCP3CONbits.DC3B0 = 0;
    CCP3CONbits.DC3B1 = 0;
    CCP3CON = 0x4C; //Set Full-bridge mode, all outputs active high;
    PR2 = 0xFF; //With a prescaler of 1:1 on Timer2, this yields a PWM frequency of 39.06 kHz;
    T3CON = 0x00; //Set Timer1 and Timer2 as the clock sources for all PWM activities;
    T2CON = 0x04; //Timer2 is set with neither prescaler nor postscaler;             

}
 
/*--------------------------------------------------------\
| KillMotors                                               |
|     													   |
| Self-explanatory; stops the motors if an error is detect-|
| ed;                                                      |
\---------------------------------------------------------*/ 
void KillMotors(void)
{
    MOTORFAILLED = 1; //Turn the Motor Failed LED;
    CCP3CON = CCP3CON & 0xF0; //Shut the PWM module down;
}

/*--------------------------------------------------------\
| StartMotors                                              |
|     													   |
| Allows the motors to be (re)started;                     |
\---------------------------------------------------------*/ 
void StartMotors(void)
{
    MOTORFAILLED = 0;
    CCP3CON = CCP3CON = 0x4C; //Set Full-bridge mode, all outputs active high;
}

/*--------------------------------------------------------\
| ImplementPIDMotion                                       |
|     													   |
| This module is fused to the PID loop algorithm in the    | 
| PID.h/.c library; it uses the output of the PID algorithm|
| to move the motors;                                      |
\---------------------------------------------------------*/ 
void ImplementPIDMotion(int PIDValue)
{
    if (PIDValue > 255) //Set the limits so that input is not greater than the possible motor input;
        PIDValue = 255;
    else if (PIDValue < -255)
        PIDValue = -255;

    if (PIDValue < 0)
        CCP3CONbits.P3M1 = 1; //If the PID loop is negative, turn counter clockwise;
    else if (PIDValue > 0)
        CCP3CONbits.P3M1 = 0; //If the PID loop is positive, turn the counter in the clockwise direction;

    PIDValue = abs(PIDValue); //Now that the direction is set, we no longer care about whether this is positive or negative;
    CCPR3L = (PIDValue >> 2) & 0xFF;
    CCP3CONbits.DC3B = (PIDValue & 0x03);

}

/*--------------------------------------------------------\
| ImplementJSMotion                                        |
|     													   |
| This module moves the motors in accordance with the      |
| output of the joystick;                                  |
\---------------------------------------------------------*/ 
void ImplementJSMotion(int JoystickValue)
{
    unsigned int CCPinput;
    if (JoystickValue < DeadbandLow) //If the joystick value is less than DeadbandLow
    {
        CCP3CONbits.P3M1 = 1; //Set the Full-bridge output REVERSE
        PIDEnableFlag = 0; //Make damn sure that the PID loop is not being implemented;
    }
    else if (JoystickValue > DeadbandHigh) //If the joystick value is greater than DeadbandHigh
    {
        CCP3CONbits.P3M1 = 0; //Set the Full-bridge output FORWARD
        PIDEnableFlag = 0; //Make damn sure that the PID loop is not being implemented;
    }
    else if ((JoystickValue <= DeadbandHigh) && (JoystickValue >= DeadbandLow) && (PIDEnableFlag == 0)) //If the Joystick Module is activated, but it's not being moved, set the movement to 0;
    {
        JoystickValue = 0;
        CCPR3L = 0x00;
        CCP3CONbits.DC3B1 = 0;
        CCP3CONbits.DC3B0 = 0;
    }

    if ((JoystickValue < DeadbandLow) || (JoystickValue > DeadbandHigh))
    {
        JoystickValue = abs(JoystickValue); //As we already know the direction, we discard the sign;
        CCPinput = JoystickValue * 2; //As there are 10 bits available to us in the PWM duty cycle, we multiply the maximum/minimum joystick value (512) by 2;
        CCPR3L = (CCPinput >> 2) & 0xFF;
        CCP3CONbits.DC3B = (CCPinput & 0x03);
    }
}

