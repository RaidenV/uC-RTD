/*--------------------------------------------------------\
| MuC.c                                                    |
|                                                          |
| V 1.0                                                    |
| 12/14/15                                                 |
|                                                          |
| This file holds the main portion of code which will run  |
| on the PIC18F2620.                                       |
\---------------------------------------------------------*/

#include <xc.h> //Include the standard PIC definitions;
#include "KeyValue.h" //Contains all of the functions regarding commands issued by the attached computer;
#include "SerComm.h" //Sets and controls serial communication;
#include "SPIMaster.h" //Contains all the functions upon which SPI communication depends;
#include "EEPROMMaster.h" //Contains all functions related to the EEPROM;

#pragma config OSC = HSPLL //With an external clock of 10 MHz the High-Speed PLL will multiply to 40 MHz;
#pragma config WDT = OFF //Turn off the WDT;
#pragma config FCMEN = OFF //Since there exists not the code to handle two speeds of communication and the slave and master are not synchronized on a single clock, there is no need for this;
#pragma config PWRT = ON //Utilize the Power-on Reset Timer, holding the uC in reset until the power input stabilizes;

#define STATUSLED PORTBbits.RB0


void initialize(void); //Initializes all modules and PIC-specific settings;
void InitializeInterrupts(void); //Initializes all interrupts;
void TMR0Init(void);  //Initializes Timer 0;
void interrupt ISR(void); //The interrupt service routine;
void TMR0Int(void); //Timer 0 interrupt;

unsigned char timerHigh = 0xC6; //Set the timer to go off every 0.375 seconds with a prescaler of 256, this should equal: (0.375)/(1/10,000,000 * 256) = 14650, or 0xC6C6 in hex;
unsigned char timerLow = 0xC6;
unsigned char TMR0Flag = 0;

double ELlast; //Saves the last position to which the Azimuth and Elevation were commanded;
double AZlast;

/*---------------------------------------------------\ 
| The following variables are defined elsewhere, but  |
| I have included them here for readability purposes. |
\---------------------------------------------------*/
double Kp; //The Proportional Constant of the PID loop;
double Ki; //The Integral Constant of the PID loop;
double Kd; //The Derivative Constant of the PID loop;
double SetAngle; //The angle to which the given Slave is set;
double CurrentAngle; //The current angle reported by the given Slave;
double CurrentVelocity; //The current velocity reported by the given Slave;

double StrippedValue; //The value derived from the string sent from the computer;
unsigned char StrippedKey; //The key derived from the string sent from the computer;
unsigned char AZEL; //This should either be 0 for Azimuth or 1 for Elevation.  Use this variable to determine which SPI CS to use in communication;
unsigned char RCFlag; //Flag which signals a command received from the computer;
unsigned char RECFlag; //Special flag denoting that a data recording routine should follow;
unsigned char AZFlowFlag; //Flag which toggles the data output for a specific axis;
unsigned char ELFlowFlag;

void main(void)
{
    unsigned char x = 0;
    initialize();

    T0CONbits.TMR0ON = 1; //Turn on Timer 0;

    while (1)
    {
        if (RCFlag == 1)
        {
            INTCONbits.GIE = 0; //Turn interrupts off for the transmission segment;
            RCFlag = 0;

            MSPIRoutine(AZEL, StrippedKey, StrippedValue);

            if ((StrippedKey == 0x01) || (StrippedKey == 0x05) || (StrippedKey == 0x07) || (StrippedKey == 0x09)) //If the value is a newly entered one from the user, save it;
                SaveAll();

            StrippedKey = 0; //Clear the received values;
            StrippedValue = 0;

            TMR0H = timerHigh; //Reset the timer by reapplying the timer values;
            TMR0L = timerLow;
            TMR0Flag = 0; //Clear the timer flag;
            INTCONbits.TMR0IF = 0; //Clear the timer interrupt flag;

            INTCONbits.GIE = 1; //Turn interrupts back on after communication with slave;
        }

        else if ((RECFlag == 1) && (RCFlag == 0)) //Something truly odd occurred here.  If the parameter (RCFlag == 0) is not included, the program will, from time to time, enter this loop regardless of the RECFlag.  For that reason, this condition is now included;
        {
            INTCONbits.GIE = 0; //Turn interrupts off for the transmission segment;
            RCFlag = 0;
            RECFlag = 0;

            MSPIRecRoutine(AZEL, StrippedKey); ///Run the Record Routine;

            TMR0H = timerHigh; //Reset the timer;
            TMR0L = timerLow;
            TMR0Flag = 0;
            INTCONbits.TMR0IF = 0;

            INTCONbits.GIE = 1; //Turn interrupts back on after communication with slave;
        }

        if (TMR0Flag == 1 && (AZFlowFlag == 1 || ELFlowFlag == 1))
        {
            do
            {
                INTCONbits.GIE = 0; //Disable interrupts for transmission;
                while (SlaveQuery(1)); //Wait for the slave to be ready;

                MSendSPI(0x02, 1); //Write the command byte to the slave;

                while (SlaveQuery(1)); //Wait for the slave to be ready;
                MReceiveStrSPI(1); //Understanding that I know how long the array will be and that I have created a global variable to handle this reception, the Receive function requires a single input: the Slave which the master communicates with;
                CurrentAngle = SPIReassembleDouble(); //The master then converts the received value into a known value using the first three bytes of the received data;

            }
            while ((checksum() == 0) || ((AZlast != 0) && (CurrentAngle == 0))); //A slight VW;

            if (AZFlowFlag == 1) //AZ and EL flow flags are handled by the KeyValue header and it's respective source.  This denotes whether or not this should be transmitted to the computer;
            {
                SerTxStr("Azimuth = ");
                breakDouble(CurrentAngle);
                SerNL();
            }

            do
            {
                INTCONbits.GIE = 0; //Disable interrupts for transmission;
                while (SlaveQuery(2)); //Wait for the slave to be ready;

                MSendSPI(0x02, 2); //Write the command byte to the slave;

                while (SlaveQuery(2)); //Wait for the slave to be ready;
                MReceiveStrSPI(2); //Understanding that I know how long the array will be, the Receive function requires two inputs, the variable which the data is received to, and the Slave which the master communicates with;
                CurrentAngle = SPIReassembleDouble(); //The master then converts the received value into a known value using the first three bytes of the received data;

            }
            while ((checksum() == 0) || ((ELlast != 0) && (CurrentAngle == 0)));

            if (ELFlowFlag == 1)
            {
                SerTxStr("Elevation = ");
                breakDouble(CurrentAngle);
                SerNL();
            }
            INTCONbits.GIE = 1; //Enable interrupts after transmission;

            TMR0Flag = 0;
        }
    }
}

void initialize(void)
{
    while (OSCCONbits.OSTS == 0); //Wait here while the Oscillator stabilizes;


    SerInit(); //Initialize all modules;
    SerTxStr("Serial Communications Initialized...");
    SerNL();
    SPIInitM();
    SerTxStr("SPI Initialized...");
    SerNL();
    TMR0Init();
    SerTxStr("Timers Initialized...");
    SerNL();
    InitializeInterrupts();
    SerTxStr("Interrupts Initialized...");
    SerNL();
    EEPROMInit();
    SerTxStr("EEPROM Initialized...");
    SerNL();
    SerTxStr("Waiting for Slaves...");
    SerNL();
    Delay10TCYx(100); //Give a slight delay to allow for the Slaves to come up and zero themselves;

    while (SlaveQuery(1) || SlaveQuery(2)); //While both slaves are not ready;
    SerTxStr("Slaves ready...");
    SerNL();
    SerTxStr("System Ready");
    SerNL();

    TRISBbits.RB0 = 0; //Set the Status LED as an output;

    STATUSLED = 1;

    T0CONbits.TMR0ON = 1; //Start Timer 0;
}

void InitializeInterrupts(void)
{
    PIE2bits.OSCFIE = 1; //Enable the Oscillator Fail interrupt;
    IPR2bits.OSCFIP = 1; //High priority;
    INTCONbits.TMR0IE = 1; //Enable the timer interrupt;
    INTCON2bits.TMR0IP = 1; //High priority;

    INTCONbits.GIE = 1; //Enable general interrupts;
    INTCONbits.PEIE = 1; //Enable Peripheral interrupts;

    PIE2bits.OSCFIE = 1; //Enable the Oscillator Fail interrupt;
}

void TMR0Init(void)
{
    T0CON = 0x07; //Prescaler of 256 enabled;
    TMR0H = timerHigh; //Load the defined timer values;
    TMR0L = timerLow;

    //The timer will be initialized externally to this protocol, giving the slaves time to start up;
}

void interrupt ISR(void)
{
    INTCONbits.GIE = 0;
    if (INTCONbits.TMR0IF == 1)
    {
        TMR0Flag = 1; //Set the Timer 0 flag;
        TMR0H = timerHigh; //Load the defined timer values;
        TMR0L = timerLow;
        INTCONbits.TMR0IF = 0; //Clear the interrupt flag;
    }

    if (PIR1bits.RCIF == 1)
    {
        RCInt();
        TMR0H = timerHigh; //I added this because I'm considering the case when the interrupt occurs directly before the Timer interrupt routine in the main body.  The RC routine should be serviced before this routine to ensure that the data transfer between master and slave avoids error;
        TMR0L = timerLow;
        TMR0Flag = 0;
    }

    if (PIR2bits.OSCFIF == 1) //If this Oscillator failed, run this;
    {
        STATUSLED = 0;
        RESET(); //If the Oscillator flag has been raised, there's a serious issue with the oscillator, so reset the device, understanding that the first part of the initialization routine checks the oscillator;
    }
    INTCONbits.GIE = 1;
}
