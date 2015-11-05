#include <xc.h>
#include <delays.h>
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
    unsigned char trash, x = 0;
    unsigned char dtime = 10;

    initialize();

    SSP1BUF = dummy_byte; //The dummy byte is defined as 0x00;
    SlaveReady = 0; //Start the slave in the ready condition;

    while (1)
    {
        if (SPIflag == 1)
        {
            SPIflag = 0;
            INTCONbits.GIE = 0; //Turn interrupts off during transmission;
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
                trash = SSP1BUF;
                PIR1bits.SSP1IF = 0;
                SlaveReady = 1;
            }
            else if ((Command == 0x01) || (Command == 0x05) || (Command == 0x07) || (Command == 0x09))
            {
                SlaveReady = 0;
                for (x = 0; x != 4; x++)
                    DoubleSPIS[x] = ReceiveSPI1();

                SlaveReady = 1;

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
                trash = SSP1BUF;
                PIR1bits.SSP1IF = 0;
                SaveAll();
            }
            else //If the command was not understood...;
            {
                trash = SSP1BUF; //Clear the buffer;
                PIR1bits.SSP1IF = 0; //Lower the flag to prepare for a fresh transfer;
            }

            INTCONbits.GIE = 1; //Turn interrupts back on;
            PIE1bits.SSP1IE = 1;
            SlaveReady = 0;
            Delay10TCYx(dtime); //I've decided to add slight delays (100 Tcy (may need to be longer)) to try to avoid the case where the master polls the slave directly after the slave is ready, but the slave has already entered into another routine which prohibits the master sending data; 
        }
        SlaveReady = 1;
        DetectJoystick();
        SlaveReady = 0;
        Delay10TCYx(10);
        if (JSEnableFlag == 1)
        {
            SlaveReady = 1; //Disallow master transmission;
            INTCONbits.GIE = 0; //Disable interrupts while the PID loop runs;
            ImplementJSMotion(DetectMovement()); //This function should guarantee that the PID loop is only stopped if the Joystick actually causes the motors to move;
            INTCONbits.GIE = 1; //Enable interrupts;
            SlaveReady = 0; //Alow the master to transmit;
            Delay10TCYx(dtime);
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
            Delay10TCYx(dtime);
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
            Delay10TCYx(dtime);
        }

        else if (TMR0Flag == 1)
        {
            SlaveReady = 1;
            INTCONbits.GIE = 0; //Disable interrupts while the PID loop runs;
            CurrentAngle = RTD2Angle(ReadRTDpos());
            INTCONbits.GIE = 1; //Enable interrupts;
            TMR0Flag = 0;
            SlaveReady = 0;
            Delay10TCYx(dtime);
        }
    }
}

void initialize(void)
{
    while (OSCCONbits.OSTS == 0); //Wait here while the Oscillator stabilizes;
    SlaveReady = 1;
    
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
        SaveAll(); //Run the save routine;
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

    INTCONbits.TMR0IE = 1; //Enable the Timer 0 Interrupt;
    T0CONbits.TMR0ON = 1;

    PIE2bits.OSCFIE = 1; //Enable the Oscillator Fail interrupt;
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