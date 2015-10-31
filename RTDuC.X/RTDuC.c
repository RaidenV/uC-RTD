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
void interrupt ISR(void);
void INT0Int(void); //Motor failed interrupt, attached to External Interrupt 0 (RB0);
void InitializeInterrupts(void);
void ZeroMotors(void);

void main(void)
{
    unsigned char temporary, x = 0;

    initialize();

    SlaveReady = 0; //Start the slave in the ready condition;
    SSP1BUF = dummy_byte; //The dummy byte is defined as 0x00;

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
                INTCONbits.GIE = 0; //Turn interrupts off during transmission;  This is somewhat of a last-minute design idea.  The idea is that the transmission time between master and slave will prove insignificant to the 30 ms PID loop time;
                SlaveReady = 0;
                for (x = 0; x < 4; x++) //Test sending multiple bytes;
                    SendSPI1(DoubleSPIS[x]);
                temporary = SSP1BUF;
                INTCONbits.GIE = 1; //Turn interrupts back on;
            }
            else if ((Command == 0x01) || (Command == 0x05) || (Command == 0x07) || (Command == 0x09))
            {
                INTCONbits.GIE = 0; //Turn interrupts off during transmission;
                SlaveReady = 0;
                for (x = 0; x != 4; x++)
                    DoubleSPIS[x] = ReceiveSPI1();
                INTCONbits.GIE = 1; //Turn interrupts back on;
                if (Command == 0x01)
                {
                    SetAngle = SPIReassembleDouble();
                    PIDEnableFlag = 3; //This flag sets two bits.  Bit 0 will be used by the main loop to determine whether or not the PID is active.  Bit 1 will be used to determine whether or not this is a new angle that is being sent;
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
        if (JSEnableFlag == 1)
        {
            ImplementJSMotion(DetectMovement()); //This function should guarantee that the PID loop is only stopped if the Joystick actually causes the motors to move;
        }

        if (PIDEnableFlag == 1 && TMR0Flag == 1) //This is the option which will run more frequently, therefore it should come first to avoid an instruction cycle of testing the PIDEnableFlag for the less likely value of '3';
        {
            SlaveReady = 1; //Disallow master transmission;
            INTCONbits.GIE = 0; //Disable interrupts while the PID loop runs;
            CurrentAngle = RTD2Angle(ReadRTDpos());
            calculatePID(CurrentAngle, SetAngle);
            ImplementPIDMotion(motorInput);
            TMR0Flag = 0; //Lower the timer flag so that this doesn't repeat before the timer has expired;
            INTCONbits.GIE = 1; //Enable interrupts;
            SlaveReady = 0; //Allow master to transmit;
        }

        else if (PIDEnableFlag == 3) //Tests if the bit has been set by the StrippedKey = 0x01 in the KeyValue code;
        {
            SlaveReady = 1; //Disallow master transmission;
            INTCONbits.GIE = 0; //Disable interrupts while the PID loop runs;
            TMR0H = timerHigh;
            TMR0L = timerLow;
            CurrentAngle = RTD2Angle(ReadRTDpos());
            calculatePID(CurrentAngle, SetAngle);
            ImplementPIDMotion(motorInput);
            INTCONbits.GIE = 1;

            T0CONbits.TMR0ON = 1;
            SlaveReady = 0; // Allow master to transmit;
        }

        else if (TMR0Flag == 1)
        {
            INTCONbits.GIE = 0; //Disable interrupts while the PID loop runs;
            CurrentAngle = RTD2Angle(ReadRTDpos());
            INTCONbits.GIE = 1; //Enable interrupts;
            TMR0Flag = 0;
        }
    }
}

void initialize(void)
{
    while (OSCCONbits.OSTS == 0); //Wait here while the Oscillator stabilizes;

    RTDInit(); //Initialize all modules;
    SPIInit();
    JoystickInit();
    MotorDriverInit();
    PIDInit();
    EEPROMInit();
    ZeroMotors();

    InitializeInterrupts();

    STATUSLED = 1;
}

void interrupt ISR(void)
{
    SlaveReady = 1; //Set the slave in the Not Ready State so that the master is no longer allowed to transmit;
    if (PIR1bits.SSP1IF == 1) //The SPI interface is of low priority;
    {
        SPIInt();
    }

    if (INTCONbits.TMR0IF == 1) //If the TMR0 Interrupt is high, and the PID loop is enabled, run this;
    {
        TMR0Int();
        SlaveReady = 0; //Take it out of the Not Ready State to allow for SPI transmission;  This is the only high priority interrupt where the slave returns to normal operating conditions afterwards;
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
        HLVDInt(); //Run the save routine;
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

void INT0Int(void)
{
    KillMotors();
    STATUSLED = 0; //Shut the status light off, indicating failure;
    T0CONbits.TMR0ON = 1; //Turn the timer on;
    INTCONbits.GIE = 0; //Turn interrupts off;
    MOTORFAILLED = 1;
    while (1)
    {
        TMR0H = 0x00; //Give the timer a long delay;
        TMR0L = 0x00;
        while (!INTCONbits.TMR0IF);
        INTCONbits.TMR0IF = 0; //Return the interrupt flag to 0;
        ~MOTORFAILLED; //Switch on/off;
    }
}

void InitializeInterrupts(void)
{
    INTCONbits.GIE = 1; //Enable General Interrupts;
    INTCONbits.PEIE = 1; //Enable Peripheral Interrupts;

    INTCONbits.TMR0IE = 1; //If so, enable the PID loop;
    T0CONbits.TMR0ON = 1;

    PIE2bits.OSCFIE = 1; //Enable the Oscillator Fail interrupt;
    IPR2bits.OSCFIP = 1; //High priority;
}

void ZeroMotors(void)
{
    CurrentAngle = 2;
    Ki = 1;
    Kp = 2;
    Kd = 0.05;
    TMR0H = timerHigh;
    TMR0L = timerLow;
    T0CONbits.TMR0ON = 1;
    PIDEnableFlag = 3;
    SetAngle = 0;
    do
    {
        CurrentAngle = RTD2Angle(ReadRTDpos());
        calculatePID(CurrentAngle, SetAngle);
        ImplementPIDMotion(motorInput);
        while (INTCONbits.TMR0IF == 0);
        INTCONbits.TMR0IF = 0;
        TMR0H = timerHigh;
        TMR0L = timerLow;
    }
    while (abs(error) > 1);

    Ki = 0;
    Kp = 0;
    Kd = 0;
}