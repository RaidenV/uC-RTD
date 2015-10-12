#include <xc.h>
#include "SPIMaster.h"

#pragma config OSC = HSPLL
#pragma config WDT = OFF
#pragma config FCMEN = OFF



void initialize(void);
void interrupt ISR(void);

void main(void)
{
    unsigned char x = 0;
    initialize();
    SlaveSelect1 = 1;

    while (1)
    {
        if (RCflag == 1)
        {
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
                        }
                        else if (StrippedKey == 0x03)
                        {
                            MSendSPI(StrippedKey, 1); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 1);
                            CurrentVelocity = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                        }
                        else if (StrippedKey == 0x04)
                        {
                            MSendSPI(StrippedKey, 1); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 1);
                            Kp = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                        }
                        else if (StrippedKey == 0x06)
                        {
                            MSendSPI(StrippedKey, 1); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 1);
                            Ki = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                        }
                        else if (StrippedKey == 0x08)
                        {
                            MSendSPI(StrippedKey, 1); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 1);
                            Kd = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
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
                        }
                        else if (StrippedKey == 0x03)
                        {
                            MSendSPI(StrippedKey, 2); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 2);
                            CurrentVelocity = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                        }
                        else if (StrippedKey == 0x04)
                        {
                            MSendSPI(StrippedKey, 2); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 2);
                            Kp = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                        }
                        else if (StrippedKey == 0x06)
                        {
                            MSendSPI(StrippedKey, 2); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 2);
                            Ki = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                        }
                        else if (StrippedKey == 0x08)
                        {
                            MSendSPI(StrippedKey, 2); //Write the command byte to the slave;
                            MReceiveStrSPI(DoubleSPIM, 2);
                            Kd = SPIReassembleDouble();
                            for (x = 0; x != 4; x++)
                                DoubleSPIM[x] = '\0'; //Clear the characters in the array;
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
        }
    }
}

void interrupt ISR(void)
{
    if (PIR1bits.RCIF == 1)
    {
        RCInt();
    }
}

void initialize(void)
{
    SPIInitM();
    SerInit(); //Initialize the RS-232 communication;
    INTCONbits.GIE = 1; //Enable general interrupts;
    INTCONbits.PEIE = 1; //Enable Peripheral interrupts;
}

