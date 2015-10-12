#include <pic18f8722.h>
#include <stdlib.h>
#include "Joystick.h"
#include "ResolverToDigital.h"

unsigned char PIDEnableFlag = 0;
unsigned char JSEnableFlag = 0;

void JoystickInit(void)
{
    TRISD = 0;
    TRISAbits.RA2 = 0; //Set the Joystick Connected LED pin as an output
    TRISBbits.RB1 = 1;
    if (PORTBbits.RB1 == 1) //If the Joystick is connected (power is applied to RB1)
    {
        JOYSTICKLED = 1; //Turn on the Joystick Connected LED
        JSEnableFlag = 1; //If the Joystick is enabled, allow its routine to run in the main body of code;
    }
    else if (PORTBbits.RB1 == 0) //If the Joystick is not connected (power is not applied to RB1)
    {
        JOYSTICKLED = 0; //Turn off the Joystick Connected LED
        JSEnableFlag = 0; //If the Joystick is disabled, disallow its routine from running in the main body of code;
    }

    ADCON0bits.ADON = 1; //Turn on the ADC module;
    ADCON0 = ADCON0 & 0x01; //Set the Analog Channel to 0 (AN0), don't touch the ADON bit;
    ADCON0 = ADCON0 | 0x04;
    ADCON1 = 0x0D; //Set the Analog Channel to 0, every other input is enabled as a digital value, use internal reference voltage);
    ADCON2 = 0xB4; //Set register: Right Justified, 16 Tad, Fosc/4;
}

void DetectJoystick(void)
{
    if (PORTBbits.RB1 == 0) //If this was triggered by a falling edge interrupt;
    {
        JOYSTICKLED = 0; //Turn off the Joystick Connected LED
        JSEnableFlag = 0; //If the Joystick is disabled, disallow its routine from running in the main body of code;
    }

    else if (PORTBbits.RB1 == 1) //If this was triggered by a rising edge interrupt;
    {
        JOYSTICKLED = 1; //Turn on the Joystick Connected LED
        JSEnableFlag = 1; //If the Joystick is enabled, allow its routine to run in the main body of code;
    }
    
    INTCON3bits.INT1IF == 0;
}

int DetectMovement(void)
{
    unsigned int ADCresult;
    int JoystickResult;

    ADCON0bits.GODONE = 1; //Start the ADC conversion;
    while (ADCON0bits.GO_NOT_DONE == 1); //Wait until the conversion is done;

    ADCresult = ADRESH; //When the conversion is done, bring out the high byte of the ADC;
    ADCresult = ADCresult << 8; //Shift the high byte left;
    ADCresult = ADCresult | ADRESL; //Add the low byte of the ADC;
    ADCresult = ADCresult + 225;
    JoystickResult = (ADCresult - JSOFFSET); //Subtract the median offset from the ADC value, yielding a value which is positive or negative;
    JoystickResult = JoystickResult * 14.28571428571428571429;
    
    if ((JoystickResult > DeadbandHigh) || (JoystickResult < DeadbandLow))  //If the joystick has been moved a sufficient distance (we don't want very slight variations causing the motor to move)
    { 
        PIDEnableFlag = 0; //Disable the PID loop; This will prevent the PID from trying to drag the motor back to its setpoint;
        return JoystickResult; //Return the result of the analog read minus the offset;
    }
    else
        return 0;
}
