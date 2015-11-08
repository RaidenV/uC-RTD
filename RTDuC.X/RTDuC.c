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
#define timer3High 0xF6 //This will provide a 5 ms delay;
#define timer3Low 0x3C // 65536 - (0.005 / (8/10e6)) = 59286 = 0xE796;

void initialize(void);
void interrupt ISR(void);
void INT0Int(void); //Motor failed interrupt, attached to External Interrupt 0 (RB0);
void RecTmrInit(void); //Initialize Timer 3 for recording data;
void InitializeInterrupts(void);
void ZeroMotors(void);
const unsigned int DataLodeSize = 603;
const unsigned int TransmitLodeSize = 1809;
double DataLode[603];
unsigned char TransmitLode[1809];

void main(void)
{
    const unsigned char dtime = 10;
    unsigned char trash, x = 0;
    unsigned int counter = 0;

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
                SPIRestart();
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
                SPIRestart();
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

        if (RECFlag == 1)
        {
            SlaveReady = 1;
            double saveKp = Kp, //Save the current Kp, Ki, and Kd values as the ZeroMotors() function overwrites it;
                    saveKi = Ki,
                    saveKd = Kd,
                    saveSP = SetAngle;

            INTCONbits.GIE = 0; //Disable interrupts;
            INTCONbits.TMR0IE = 0; //This is notable: in order to utilize the timer in any other situation other than interrupt, the interrupt specific to this timer must be disabled;
            T0CONbits.TMR0ON = 0; //Turn the timer off in case it was running prior to this;

            ZeroMotors(); //Start with the motors zeroed;
            Kp = saveKp; //Reload the Kp, Ki, and Kd values to be tested;
            Ki = saveKi;
            Kd = saveKd;

            TMR0H = timerHigh;
            TMR0L = timerLow;

            PIDEnableFlag = 3; //Set the PIDEnable flag, letting the loop know that there is a new angle entered;
            SetAngle = 60; //60 degrees is an arbitrary angle used to test the PID loop.  This could be anything, but as I've seen with servo amplifiers such as those from AMC, a set degree difference is used to repeatedly test the success of the PID loop;
            counter = 0; //Set the event counter to 0;

            T0CONbits.TMR0ON = 1; //Start the timers;
            T3CONbits.TMR3ON = 1;
            
            STATUSLED = 0;

            while (counter < DataLodeSize)
            {
                CurrentAngle = RTD2Angle(ReadRTDpos()); //Read the current position;
                calculatePID(CurrentAngle, SetAngle); //Calculate the loop output which generates the motorInput variable;
                ImplementPIDMotion(motorInput); //Implement the PID motion;
                while (INTCONbits.TMR0IF == 0) //I'm using the outer timer loop to replicate the PID loop;
                {
                    STATUSLED = 1;
                    TMR3H = timer3High;
                    TMR3L = timer3Low;
                    DataLode[counter] = RTD2Angle(ReadRTDpos());
                    STATUSLED = 0;
                    while (PIR2bits.TMR3IF == 0); //I'm using this inner loop to record data more frequently than every PID update.  This is primarily due to our god-awful gear ratio of 4:1 which gives a terrible mechanical constant on the order of milliseconds rather than seconds :( ;
                    PIR2bits.TMR3IF = 0;
                    if (counter < DataLodeSize) //We're going to be counting this number of events.  This is to prevent the counter from overrunning and writing angles into locations not included in the DataLode[] array;
                        counter++;
                }
                INTCONbits.TMR0IF = 0;
                TMR0H = timerHigh;
                TMR0L = timerLow;
            }
            SPIDisassembleLode(DataLode, TransmitLode); //Prepare the data for transfer;
            SPIRestart(); //Restart the module to clear any errors;
            SlaveReady = 0; //Slave is ready to transmit;
            for (counter = 0; counter != TransmitLodeSize; counter++) //Send all gathered data;
                SendSPI1(TransmitLode[counter]);
            SlaveReady = 1;
            SPIRestart(); //Clear the module;
            RECFlag = 0;
            INTCONbits.TMR0IE = 1;
            INTCONbits.PEIE = 1;
            INTCONbits.GIE = 1;
            SetAngle = saveSP;
            PIDEnableFlag = 3;


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
    ZeroMotors();
    EEPROMInit();
    RecTmrInit();

    InitializeInterrupts();
    TRISAbits.RA3 = 0;
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

void RecTmrInit(void)
{
    T3CONbits.T3CKPS = 0x3; //Prescaler of 1:8;
    T3CONbits.TMR3CS = 0; //Clock source, Fosc/4;
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
    double average, PrevAngle;
    Ki = 2; //I've found that these values create a pretty smooth motion;
    Kp = 0.5;
    Kd = 0;
    TMR0H = timerHigh; //We will use the standard PID loop time of 30 ms;
    TMR0L = timerLow;
    PIDEnableFlag = 3;
    SetAngle = 0;
    INTCONbits.GIE = 0;
    INTCONbits.PEIE = 0;
    T0CONbits.TMR0ON = 1;
    do
    {
        CurrentAngle = RTD2Angle(ReadRTDpos());
        calculatePID(CurrentAngle, SetAngle);
        ImplementPIDMotion(motorInput);
        while (INTCONbits.TMR0IF == 0);
        INTCONbits.TMR0IF = 0;
        TMR0H = timerHigh;
        TMR0L = timerLow;
        average = (PrevAngle + CurrentAngle) / 2;
        PrevAngle = CurrentAngle;
    }
    while ((average > 1) && (abs(error) > 1)); //Get within 1 degree of zero;
}