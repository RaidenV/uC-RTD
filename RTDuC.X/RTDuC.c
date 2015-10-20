#include <xc.h>
#include "Joystick.h"
#include "MotorControl.h"
#include "PID.h"
#include "ResolverToDigital.h"
#include "EEPROM.h"
#include "SPISlave.h"

#pragma config OSC = HSPLL
#pragma config WDT = OFF
#pragma config FCMEN = OFF

#define STATUSLED PORTAbits.RA3

void initialize(void);
void interrupt high_priority hISR(void);
void interrupt low_priority lISR(void);

void main(void)
{
    unsigned char temporary, x;

    initialize();

    SlaveReady = 0; //Start the slave in the ready condition;
    SSP2BUF = dummy_byte; //The dummy byte is defined as 0x00;

    while (1)
    {
        if (SPIflag == 1)
        {
            SPIflag = 0;
            if ((Command == 0x02) || (Command == 0x03) || (Command == 0x04) || (Command == 0x06) || (Command == 0x08))
            {
                if (Command == 0x02)
                    SPIDisassembleDouble(CurrentAngle);
                else if (Command == 0x03)
                    SPIDisassembleDouble(CurrentVelocity);
                else if (Command == 0x04)
                    SPIDisassembleDouble(Kp);
                else if (Command == 0x06)
                    SPIDisassembleDouble(Ki);
                else if (Command == 0x08)
                    SPIDisassembleDouble(Kd);
                SlaveReady = 0;
                for (x = 0; x < 4; x++) //Test sending multiple bytes;
                    SendSPI1(DoubleSPIS[x]);
                temporary = SSP1BUF;
            }
            else if ((Command == 0x01) || (Command == 0x05) || (Command == 0x07) || (Command == 0x09))
            {
                SlaveReady = 0;
                for (x = 0; x != 4; x++)
                    DoubleSPIS[x] = ReceiveSPI1();
                if (Command == 0x01)
                {
                    SetAngle = SPIReassembleDouble();
                    PIDEnableFlag = 0x03; //This flag sets two bits.  Bit 0 will be used by the main loop to determine whether or not the PID is active.  Bit 1 will be used to determine whether or not this is a new angle that is being sent;
                    JSEnableFlag = 0; //Turn off the joystick;
                }
                else if (Command == 0x05)
                {
                    Kp = SPIReassembleDouble();
                }
                else if (Command == 0x07)
                {
                    Ki = SPIReassembleDouble();
                }
                else if (Command == 0x09)
                {
                    Kd = SPIReassembleDouble();
                }
                temporary = SSP1BUF;
            }
            PIE1bits.SSP1IE = 1;
        }
        
        DetectJoystick();
        if(JSEnableFlag == 1)
        {
           ImplementJSMotion(DetectMovement()); //This function should guarantee that the PID loop is only stopped if the Joystick actually causes the motors to move;
        }
        
        else if((PIDEnableFlag & 0x02) == 0x02) //We don't need this repeating all the time, only when it's a new angle;
        {
            INTCONbits.TMR0IE = 1; //Enable the timer interrupt;
            JSEnableFlag = 0; //Disable the Joystick;
        }
    }

}

void initialize(void)
{
    while (OSCCONbits.OSTS == 0); //Wait here while the Oscillator stabilizes;

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
    if ((INTCONbits.TMR0IF == 1) && ((PIDEnableFlag | 0x01) == 0x01)) //If the TMR0 Interrupt is high, and the PID loop is enabled, run this;
    {
        TMR0Int();
    }

    if (INTCONbits.INT0IF == 1) //If the motor has failed, run this;
    {
        INT0Int();
    }

    if (PIR2bits.HLVDIF == 1) //If this unit is being powered down, run this;
    {
        MOTORFAILLED = 0; //If the system is powering down, quickly turn off all the LEDs;
        STATUSLED = 0;
        JOYSTICKLED = 0;
        HLVDInt();
    }

    if (PIR2bits.OSCFIF == 1) //If this Oscillator failed, run this;
    {
        RESET(); //If the Oscillator flag has been raised, there's a serious issue with the oscillator, so reset the device, understanding that the first part of the initialization routine checks the oscillator;
        /*
         * Understandably, the oscillator issue can be fixed by writing a bunch of alternate code which handles the event
         * that the external oscillator goes down.  The FCMEN monitors the quality of the external oscillator and allows
         * for such handling.  This would, however, require a substantial amount of code to be loaded onto the PIC and,
         * at this point, I'm not sure it's that important.  That, and the code would involve ifs, which will take up
         * loop time.
         * 
         * On second thought, after writing the body of the code, it would appear that, if all goes well in testing,
         * there is plenty of space left to write alternate code in the event that the Oscillator crashes.
         * This would more or less involve changing anything related to timing, whether that's baud rate or the timer
         * value, and figuring out a way to revert in the event that the oscillator comes back online;
         */
    }
}

void interrupt low_priority lISR(void)
{
    if (PIR1bits.SSP1IF == 1) //The SPI interface is of low priority;
    {
        SPIInt();
    }
}