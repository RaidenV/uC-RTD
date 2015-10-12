#include <xc.h>
#include <spi.h>
#include <stdlib.h>
#include "KeyValue.h"
#include "SerComm.h"
#include <delays.h>

#pragma config OSC = HSPLL
#pragma config WDT = OFF
#pragma config FCMEN = OFF

#define SlaveReady1 PORTAbits.RA1
#define SlaveReady2 PORTAbits.RA2
#define SlaveSelect1 PORTAbits.RA3
#define SlaveSelect2 PORTAbits.RA4

void initialize(void);
void interrupt ISR(void);
unsigned char MReceiveSPI(void);
double SPIReassembleDouble(void);
void MSendSPI(unsigned char);
void MReceiveStrSPI(unsigned char*, unsigned char);
unsigned char checksum(void);

void SPIDisassembleDouble(double dub);
unsigned char MGenerateChecksum(void);

unsigned char RCflag = 0;
unsigned char ReceivedChar;
unsigned char* DoublePtr;
unsigned char DoubleSPIM[4];

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
            if ((StrippedKey == 0x02) || (StrippedKey == 0x03) || (StrippedKey == 0x04) || (StrippedKey == 0x06) || StrippedKey == 0x08)
            {
                do
                {
                    if (StrippedKey == 0x02)
                    {
                        MSendSPI(StrippedKey); //Write the command byte to the slave;
                        MReceiveStrSPI(DoubleSPIM, 1);
                        CurrentAngle = SPIReassembleDouble();
                        for (x = 0; x != 4; x++)
                            DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                    }
                    else if (StrippedKey == 0x03)
                    {
                        MSendSPI(StrippedKey); //Write the command byte to the slave;
                        MReceiveStrSPI(DoubleSPIM, 1);
                        CurrentVelocity = SPIReassembleDouble();
                        for (x = 0; x != 4; x++)
                            DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                    }
                    else if (StrippedKey == 0x04)
                    {
                        MSendSPI(StrippedKey); //Write the command byte to the slave;
                        MReceiveStrSPI(DoubleSPIM, 1);
                        Kp = SPIReassembleDouble();
                        for (x = 0; x != 4; x++)
                            DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                    }
                    else if (StrippedKey == 0x06)
                    {
                        MSendSPI(StrippedKey); //Write the command byte to the slave;
                        MReceiveStrSPI(DoubleSPIM, 1);
                        Ki = SPIReassembleDouble();
                        for (x = 0; x != 4; x++)
                            DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                    }
                    else if (StrippedKey == 0x08)
                    {
                        MSendSPI(StrippedKey); //Write the command byte to the slave;
                        MReceiveStrSPI(DoubleSPIM, 1);
                        Kd = SPIReassembleDouble();
                        for (x = 0; x != 4; x++)
                            DoubleSPIM[x] = '\0'; //Clear the characters in the array;
                    }
                }
                while (!checksum());
            }
            else if ((StrippedKey == 0x01) || (StrippedKey == 0x05) || (StrippedKey == 0x07) || (StrippedKey == 0x09))
            {

                MSendSPI(StrippedKey);
                SPIDisassembleDouble(StrippedValue);
                SlaveSelect1 = 0;
                while (SlaveReady1);
                Delay10TCYx(50);
                for (x = 0; x != 4; x++)
                    MSendSPI(DoubleSPIM[x]);
                SlaveSelect1 = 1;
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
    OpenSPI(SPI_FOSC_4, MODE_00, SMPMID);
    INTCONbits.GIE = 1; //Enable general interrupts;
    INTCONbits.PEIE = 1; //Enable Peripheral interrupts;
    TRISAbits.RA1 = 1; //Set the SlaveReady pin as an input;
    TRISAbits.RA3 = 0; //Set the SlaveSelect pin as an output;
    SerInit(); //Initialize the RS-232 communication;
}

void MSendSPI(unsigned char data)
{
    SlaveSelect1 = 0; //Bring the SS to 0, enabling the slave;
    Delay10TCYx(1); //delay for 10 clock cycles to ensure the slave is ready;
    unsigned char tempChar;
    tempChar = SSPBUF;
    PIR1bits.SSPIF = 0;
    SSPBUF = data;
    while (!PIR1bits.SSPIF);
    data = SSPBUF;
    SlaveSelect1 = 1; //Set the SS, resetting the bit count of the slave;
}

unsigned char MReceiveSPI(void)
{
    unsigned char tempCH;
    tempCH = SSPBUF; //Clear the buffer;
    PIR1bits.SSPIF = 0; //Clear the MSSP Interrupt Flag; 
    SSPBUF = 0x00; //Initiate communication by sending a dummy byte;
    while (!PIR1bits.SSPIF); // Wait until transmission is complete;
    return SSPBUF; //Read/return the buffer;
}

void MReceiveStrSPI(unsigned char* str, unsigned char Slave)
{
    if (Slave == 1)
    {
        unsigned char x;
        SlaveSelect1 = 0; //Clear the SS, enabling the slave; 
        while (SlaveReady1);
        Delay10TCYx(25); //250 TCY Delay;
        for (x = 0; x < 4; x++)
            DoubleSPIM[x] = MReceiveSPI(); //Read data from slave;
        breakDouble(SPIReassembleDouble()); //This function takes the double and converts it into readable characters on the screen;
        SlaveSelect1 = 1; //Set the SS, ending communication with the slave;
    }
    else if (Slave == 2)
    {
        unsigned char x;
        SlaveSelect2 = 0; //Clear the SS, enabling the slave; 
        while (SlaveReady2);
        Delay10TCYx(50); //500 TCY Delay;
        for (x = 0; x < 3; x++)
            str[x] = MReceiveSPI(); //Read data from slave;
        Delay10TCYx(1);
        SlaveSelect2 = 1;
    }
}

double SPIReassembleDouble(void)
{
    double dub;
    DoublePtr = (unsigned char*) &dub; //This sets the pointer to the location of the first byte of the double;
    DoublePtr[0] = DoubleSPIM[0]; //The following lines extract the double byte-by-byte into the DDouble variable;
    DoublePtr[1] = DoubleSPIM[1]; //This way, whenever the double is operated upon, the referenced double is not altered;
    DoublePtr[2] = DoubleSPIM[2];
    return dub; //Return the reconstructed double;
}

unsigned char checksum(void)
{
    unsigned char y, sum = 0;
    for (y = 0; y != 3; y++)
        sum += DoubleSPIM[y];
    if ((sum - DoubleSPIM[3]) == 0)
        return 1;
    else
        return 0;
}

void SPIDisassembleDouble(double dub)
{
    DoublePtr = (unsigned char*) &dub;
    DoubleSPIM[0] = DoublePtr[0];
    DoubleSPIM[1] = DoublePtr[1];
    DoubleSPIM[2] = DoublePtr[2];
    DoubleSPIM[3] = MGenerateChecksum();
}

unsigned char MGenerateChecksum(void)
{
    unsigned char z, sum = 0;
    for (z = 0; z != 3; z++)
        sum += DoubleSPIM[z];
    return sum;
}