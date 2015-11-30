#include <xc.h>
#include "KeyValue.h"
#include "SerComm.h"
#include "SPIMaster.h"
#include "EEPROMMaster.h"

#pragma config OSC = HSPLL
#pragma config WDT = OFF
#pragma config FCMEN = OFF
#pragma config PWRT = ON

#define STATUSLED PORTBbits.RB0

void initialize(void);
void InitializeInterrupts(void);
void TMR0Init(void);
void interrupt ISR(void);
void TMR0Int(void);

/*The following timer setting is controversial.  Is it necessary to have 1/4 second interrupts?*/
unsigned char timerHigh = 0xC6; //Set the timer to go off every quarter second with a prescaler of 256, this should equal: (0.25)/(1/10,000,000 * 256) = 9766, or 0x2626 in hex;
unsigned char timerLow = 0xC6;
unsigned char TMR0Flag = 0;

double ELlast;
double AZlast;

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
            
            TMR0H = timerHigh; //Reset the timer;
            TMR0L = timerLow;
            TMR0Flag = 0;
            INTCONbits.TMR0IF = 0;
            
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
                MReceiveStrSPI(1); //Understanding that I know how long the array will be, the Receive function requires two inputs, the variable which the data is received to, and the Slave which the master communicates with;
                CurrentAngle = SPIReassembleDouble(); //The master then converts the received value into a known value using the first three bytes of the received data;

            }
            while ((checksum() == 0) || ((AZlast != 0) && (CurrentAngle == 0)));

            if (AZFlowFlag == 1)
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


    SerInit();
    SerTxStr("Serial Communications Initialized...");
    SerNL();
    Delay10TCYx(1);

    SPIInitM(); //Initialize all modules;
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

    T0CONbits.TMR0ON = 1;
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
    TMR0H = timerHigh;
    TMR0L = timerLow;

    //The timer will be initialized externally to this protocol, giving the slaves time to start up;
}

void interrupt ISR(void)
{
    INTCONbits.GIE = 0;
    if (INTCONbits.TMR0IF == 1)
    {
        TMR0Flag = 1;
        TMR0H = timerHigh;
        TMR0L = timerLow;
        INTCONbits.TMR0IF = 0;
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