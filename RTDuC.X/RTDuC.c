#include <xc.h>
#include "Joystick.h"
#include "MotorControl.h"
#include "PID.h"
#include "ResolverToDigital.h"
#include "EEPROM.h"
#include "SPISlave.h"

#pragma config OSC = HSPLL
#pragma config WDT = OFF

#define STATUSLED PORTAbits.RA3

void initialize(void);
void interrupt high_priority hISR(void);
void interrupt low_priority lISR(void);

void main(void)
{
    initialize();
    while(1)
    {
        
    }
    
}

void initialize(void)
{
    while(OSCCONbits.OSTS == 0); //Wait here while the Oscillator stabilizes;
    
    RTDInit(); //Initialize all modules;
    JoystickInit();
    MotorDriverInit();
    PIDInit();
    SPIInit();
    EEPROMInit();
    
    INTCONbits.GIE = 1; //Enable General Interrupts;
    INTCONbits.PEIE = 1; //Enable Peripheral Interrupts;
    RCONbits.IPEN = 1; //Enable Interrupt Priority;
    
    PIE2bits.OSCFIE = 1; //Enable the Oscillator Fail interrupt;
    IPR2bits.OSCFIP = 1; //High priority;
}

void interrupt high_priority hISR(void)
{
    if(INTCONbits.TMR0IF == 1)
    {
        TMR0Int();
    }
    
    if(INTCONbits.INT0IF == 1)
    {
        INT0Int();
    }
    
    if(PIR2bits.HLVDIF == 1)
    {
        MOTORFAILLED = 0;  //If the system is powering down, quickly turn off all the LEDs;
        STATUSLED = 0;
        JOYSTICKLED = 0;
        HLVDInt();
    }
    
    if(PIR2bits.OSCFIF == 1)
    {
        RESET(); //If the Oscillator flag has been raised, there's a serious issue with the oscillator, so reset the device, understanding that the first part of the initialization routine checks the oscillator;
        /*
         * Understandably, the oscillator issue can be fixed by writing a bunch of alternate code which handles the event
         * that the external oscillator goes down.  The FCMEN monitors the quality of the external oscillator and allows
         * for such handling.  This would, however, require a substantial amount of code to be loaded onto the PIC and,
         * at this point, I'm not sure it's that important.
         */
    }
}

void interrupt low_priority lISR(void)
{
    if(PIR1bits.SSP1IF == 1)
    {
        SPIInt();
    }
}