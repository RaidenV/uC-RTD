#include "MotorControl.h"

unsigned char PIDEnableFlag;
unsigned char MotorFailFlag = 0;

void MotorDriverInit(void)
{
    TRISGbits.RG0 = 0; //ECCP3 Enhanced PWM output Channel: A, AHA (MOSFET Driver Chip)
    TRISGbits.RG3 = 0; //ECCP3 Enhanced PWM output Channel: D, ALB (MOSFET Driver Chip)
    TRISEbits.RE3 = 0; //ECCP3 Enhanced PWM output Channel: C, ALA (MOSFET DRIVER CHIP)
    TRISEbits.RE4 = 0; //ECCP3 Enhanced PWM output Channel: B, AHB (MOSFET Driver Chip)
    TRISBbits.RB0 = 1; //FAULT Pin Falling-edge Interrupt (MOSFET Dr)

    TRISAbits.RA4 = 0; //Motor Fault LED;

    INTCONbits.INT0IE = 1; //Enable the interrupt for the motor fault;
    INTCON2bits.INTEDG0 = 0; //This interrupt is a falling edge interrupt;
    //INT0 is always high priority and therefore does not require a bit to be set;
    
    CCP3CON = 0x4C; //Set Full-bridge mode, all outputs active high;
    PR2 = 0xFF; //With a prescaler of 1:1 on Timer2, this yields a PWM frequency of 39.06 kHz;
    T3CON = 0x00; //Set Timer1 and Timer2 as the clock sources for all PWM activities;
    T2CON = 0x04; //Timer2 is set with neither prescaler nor postscaler;            

}

void KillMotors(void)
{
    MOTORFAILLED = 1; //Turn the Motor Failed LED;
    CCP3CON = CCP3CON & 0xF0; //Shut the PWM module down;
}

void ImplementPIDMotion(int PIDValue)
{
    if (PIDValue < 0)
        CCP3CONbits.P3M1 = 1; //If the PID loop is negative, turn counter clockwise;
    else if (PIDValue > 0)
        CCP3CONbits.P3M1 = 0; //If the PID loop is positive, turn the counter in the clockwise direction;
    
    PIDValue = abs(PIDValue); //Now that the direction is set, we no longer care about whether this is positive or negative;
    CCPR3L = (PIDValue >> 2) & 0xFF;
    CCP3CONbits.DC3B = (PIDValue & 0x03);
}

void ImplementJSMotion(int JoystickValue)
{
    unsigned int CCPinput;
    PIDEnableFlag = 0; //Make damn sure that the PID loop is not being implemented;
    if (JoystickValue < DeadbandLow) //If the joystick value is less than DeadbandLow
    {
        CCP3CONbits.P3M1 = 1; //Set the Full-bridge output REVERSE
    }
    else if (JoystickValue > DeadbandHigh) //If the joystick value is greater than DeadbandHigh
    {
        CCP3CONbits.P3M1 = 0; //Set the Full-bridge output FORWARD
    }
    else if (JoystickValue <= DeadbandHigh && JoystickValue >= DeadbandLow) //If the Joystick Module is activated, but it's not being moved, set the movement to 0;
    {
        JoystickValue = 0;
    }

    JoystickValue = abs(JoystickValue);
    CCPinput = JoystickValue * 2;
    CCPR3L = (CCPinput >> 2) & 0xFF;
    //  PORTD = CCPinput & 0xFF; //This was used to show the resulting PWM input on the LEDs attached to PORTD;
    CCP3CONbits.DC3B = (CCPinput & 0x03);
}

void INT0Int(void)
{
    KillMotors();
    INTCONbits.INT0IF = 0;
}

