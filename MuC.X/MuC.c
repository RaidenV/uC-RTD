#include <xc.h>
#include "KeyValue.h"
#include "SerComm.h"
#include "SPIMaster.h"
#include "EEPROMMaster.h"

#pragma config OSC = HSPLL
#pragma config WDT = OFF
#pragma config FCMEN = OFF

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

void main(void)
{
    unsigned char x = 0;
    initialize();

    T0CONbits.TMR0ON = 1; //Turn on Timer 0;

    while (1)
    {
        if (RCflag == 1)
        {
            INTCONbits.GIE = 0; //Turn interrupts off for the transmission segment;
            RCflag = 0;
            if (AZEL == 1)
            {
                if ((StrippedKey == 0x02) || (StrippedKey == 0x03) || (StrippedKey == 0x04) || (StrippedKey == 0x06) || StrippedKey == 0x08)
                {
                    do
                    {
                        if (StrippedKey == 0x02)
                        {
                            MSendSPI(StrippedKey, 1); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 1); //Understanding that I know how long the array will be, the Receive function requires two inputs, the variable which the data is received to, and the Slave which the master communicates with;
                            CurrentAngle = SPIReassembleDouble(); //The master then converts the received value into a known value using the first three bytes of the received data;
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                            SerTxStr("Azimuth = ");
                            breakDouble(CurrentAngle);
                            SerNL();
                        }
                        else if (StrippedKey == 0x03)
                        {
                            MSendSPI(StrippedKey, 1); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 1);
                            CurrentVelocity = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                            SerTxStr("Azimuth Velocity = ");
                            breakDouble(CurrentVelocity);
                            SerNL();
                        }
                        else if (StrippedKey == 0x04)
                        {
                            MSendSPI(StrippedKey, 1); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 1);
                            Kp = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                            SerTxStr("Kp = ");
                            breakDouble(Kp);
                            SerNL();
                        }
                        else if (StrippedKey == 0x06)
                        {
                            MSendSPI(StrippedKey, 1); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 1);
                            Ki = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                            SerTxStr("Ki = ");
                            breakDouble(Ki);
                            SerNL();
                        }
                        else if (StrippedKey == 0x08)
                        {
                            MSendSPI(StrippedKey, 1); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 1);
                            Kd = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                            SerTxStr("Kd = ");
                            breakDouble(Kd);
                            SerNL();
                        }
                    }
                    while (!checksum()); //While the Checksum does not correlate with the received value;
                }
                else if ((StrippedKey == 0x01) || (StrippedKey == 0x05) || (StrippedKey == 0x07) || (StrippedKey == 0x09)) //If the key is something that requires the master to send data;
                {
                    MSendSPI(StrippedKey, 1); //Send the stripped key;
                    SPIDisassembleDouble(StrippedValue); //While we wait for the slave to be ready we'll break down the double;
                    SlaveSelect1 = 0;
                    while (SlaveReady1);
                    Delay10TCYx(50);
                    for (x = 0; x != 4; x++)
                        MSendSPI(DoubleSPIM[x], 1);
                    SlaveSelect1 = 1;
                }
            }

            else if (AZEL == 2)
            {
                if ((StrippedKey == 0x02) || (StrippedKey == 0x03) || (StrippedKey == 0x04) || (StrippedKey == 0x06) || StrippedKey == 0x08)
                {
                    do
                    {
                        if (StrippedKey == 0x02)
                        {
                            MSendSPI(StrippedKey, 2); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 2); //Understanding that I know how long the array will be, the Receive function requires two inputs, the variable which the data is received to, and the Slave which the master communicates with;
                            CurrentAngle = SPIReassembleDouble(); //The master then converts the received value into a known value using the first three bytes of the received data;
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                            SerTxStr("Elevation = ");
                            breakDouble(CurrentAngle);
                            SerNL();

                        }
                        else if (StrippedKey == 0x03)
                        {
                            MSendSPI(StrippedKey, 2); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 2);
                            CurrentVelocity = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                            SerTxStr("Elevation Velocity = ");
                            breakDouble(CurrentVelocity);
                            SerNL();
                        }
                        else if (StrippedKey == 0x04)
                        {
                            MSendSPI(StrippedKey, 2); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 2);
                            Kp = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                            SerTxStr("Kp = ");
                            breakDouble(Kp);
                            SerNL();
                        }
                        else if (StrippedKey == 0x06)
                        {
                            MSendSPI(StrippedKey, 2); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 2);
                            Ki = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                            SerTxStr("Ki = ");
                            breakDouble(Ki);
                            SerNL();
                        }
                        else if (StrippedKey == 0x08)
                        {
                            MSendSPI(StrippedKey, 2); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 2);
                            Kd = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                            SerTxStr("Kd = ");
                            breakDouble(Kd);
                            SerNL();
                        }
                    }
                    while (!checksum()); //While the Checksum does not correlate with the received value;
                }
                else if ((StrippedKey == 0x01) || (StrippedKey == 0x05) || (StrippedKey == 0x07) || (StrippedKey == 0x09)) //If the key is something that requires the master to send data;
                {
                    MSendSPI(StrippedKey, 2); //Send the stripped key;
                    SPIDisassembleDouble(StrippedValue); //While we wait for the slave to be ready we'll break down the double;
                    SlaveSelect2 = 0;
                    while (SlaveReady2);
                    Delay10TCYx(50);
                    for (x = 0; x != 4; x++)
                        MSendSPI(DoubleSPIM[x], 2);
                    SlaveSelect2 = 1;
                }
            }
            INTCONbits.GIE = 1; //Turn interrupts back on after communication with slave;
        }

        if (TMR0Flag == 1)
        {
            do
            {
                INTCONbits.GIE = 0; //Disable interrupts for transmission;
                while (SlaveReady1); //Wait for the slave to be ready;

                MSendSPI(0x02, 1); //Write the command byte to the slave;

                while (SlaveReady1); //Wait for the slave to be ready;
                MReceiveStrSPI(DoubleSPIM, 1); //Understanding that I know how long the array will be, the Receive function requires two inputs, the variable which the data is received to, and the Slave which the master communicates with;
                CurrentAngle = SPIReassembleDouble(); //The master then converts the received value into a known value using the first three bytes of the received data;
                for (x = 0; x != 4; x++)
                    DoubleSPIM[x] = '\0'; //Clear the characters in the array;

            }
            while (!checksum());

            SerTxStr("Azimuth = ");
            breakDouble(CurrentAngle);
            SerNL();

            //            while (SlaveReady2); //Wait for the slave to be ready;
            //
            //            MSendSPI(0x02, 2); //Write the command byte to the slave;
            //
            //            while (SlaveReady2); //Wait for the slave to be ready;
            //            MReceiveStrSPI(DoubleSPIM, 2); //Understanding that I know how long the array will be, the Receive function requires two inputs, the variable which the data is received to, and the Slave which the master communicates with;
            //            CurrentAngle = SPIReassembleDouble(); //The master then converts the received value into a known value using the first three bytes of the received data;
            //            for (x = 0; x != 4; x++)
            //                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
            //
            //            SerTxStr("Elevation = ");
            //            breakDouble(CurrentAngle);
            //            SerNL();
            INTCONbits.GIE = 1; //Enable interrupts after transmission;

            TMR0Flag = 0;
        }
    }
}

void initialize(void)
{
    while (OSCCONbits.OSTS == 0); //Wait here while the Oscillator stabilizes;

    SPIInitM(); //Initialize all modules;
    SerInit();
    EEPROMInit();
    TMR0Init();
    InitializeInterrupts();

    Delay10TCYx(10); //Give a slight delay to allow for the Slaves to come up;

    while (SlaveReady1 || SlaveReady2); //While both slaves are not ready;

}

void InitializeInterrupts(void)
{
    PIE2bits.OSCFIE = 1; //Enable the Oscillator Fail interrupt;
    IPR2bits.OSCFIP = 1; //High priority;
    INTCONbits.TMR0IE = 1; //Enable the timer interrupt;
    INTCON2bits.TMR0IP = 1; //High priority;

    INTCONbits.GIE = 1; //Enable general interrupts;
    INTCONbits.PEIE = 1; //Enable Peripheral interrupts;

}

void TMR0Init(void)
{
    T0CON = 0x87; //Prescaler of 256 enabled;
    TMR0H = timerHigh;
    TMR0L = timerLow;

    //The timer will be initialized externally to this protocol, giving the slaves time to start up;
}

void interrupt ISR(void)
{
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
    }
    INTCONbits.GIE = 1;
}