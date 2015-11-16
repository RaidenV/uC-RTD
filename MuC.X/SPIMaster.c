#include "SPIMaster.h"

unsigned char RCFlag = 0;
unsigned char ReceivedChar;
unsigned char* DoublePtr;
unsigned char DoubleSPIM[4];
const unsigned int ReceiveLodeSize = 1809;
const unsigned int DataLodeSize = 603;
unsigned char ReceiveLode[1809];
double DataLode[603];

void SPIInitM(void)
{
    OpenSPI(SPI_FOSC_16, MODE_00, SMPMID);
    ADCON1 = 0x0F; //Switch all pins related to the ADC to digital I/O's
    TRISBbits.RB1 = 1; //Set the SlaveReady pins as an input;
    TRISBbits.RB2 = 1;
    TRISBbits.RB3 = 0; //Set the SlaveSelect pins as an output;
    TRISBbits.RB4 = 0;
    INTCON2bits.RBPU = 1;

}

void MSendSPI(unsigned char data, unsigned char Slave)
{
    if (Slave == 1)
    {
        SlaveSelect1 = 0; //Bring the SS to 0, enabling the slave;
        Delay10TCYx(1); //delay for 10 clock cycles to ensure the slave is ready;
        unsigned char tempChar;
        tempChar = SSPBUF;
        PIR1bits.SSPIF = 0;
        while (SlaveReady1);
        SSPBUF = data;
        while (!PIR1bits.SSPIF);
        data = SSPBUF;
        SlaveSelect1 = 1; //Set the SS, resetting the bit count of the slave;
    }

    else if (Slave == 2)
    {
        SlaveSelect2 = 0; //Bring the SS to 0, enabling the slave;
        Delay10TCYx(1); //delay for 10 clock cycles to ensure the slave is ready;
        unsigned char tempChar;
        tempChar = SSPBUF;
        PIR1bits.SSPIF = 0;
        while (SlaveReady2);
        SSPBUF = data;
        while (!PIR1bits.SSPIF);
        data = SSPBUF;
        SlaveSelect1 = 1; //Set the SS, resetting the bit count of the slave;
    }
}

unsigned char MReceiveSPI(unsigned char Slaves)
{
    unsigned char tempCH;
    tempCH = SSPBUF; //Clear the buffer;
    PIR1bits.SSPIF = 0; //Clear the MSSP Interrupt Flag; 
    if (Slaves == 1)
        while (SlaveReady1);
    else if (Slaves == 2)
        while (SlaveReady2);
    SSPBUF = 0x00; //Initiate communication by sending a dummy byte;
    while (!PIR1bits.SSPIF); // Wait until transmission is complete;
    PIR1bits.SSPIF = 0;
    return SSPBUF; //Read/return the buffer;
}

void MReceiveStrSPI(unsigned char Slave)
{
    if (Slave == 1)
    {
        unsigned char x;
        SlaveSelect1 = 0; //Clear the SS, enabling the slave; 
        Delay10TCYx(30); //250 TCY Delay;
        for (x = 0; x < 4; x++)
            DoubleSPIM[x] = MReceiveSPI(1); //Read data from slave;
        SlaveSelect1 = 1; //Set the SS, ending communication with the slave;
    }
    else if (Slave == 2)
    {
        unsigned char x;
        SlaveSelect2 = 0; //Clear the SS, enabling the slave; 
        Delay10TCYx(25); //500 TCY Delay;
        for (x = 0; x < 4; x++)
            DoubleSPIM[x] = MReceiveSPI(2); //Read data from slave;
        Delay10TCYx(1);
        SlaveSelect2 = 1;
    }
}

void MReceiveLodeSPI(unsigned char Slave)
{
    if (Slave == 1)
    {
        unsigned int x;
        SlaveSelect1 = 0; //Clear the SS, enabling the slave; 
        Delay10TCYx(30); //250 TCY Delay;
        for (x = 0; x < ReceiveLodeSize; x++)
            ReceiveLode[x] = MReceiveSPI(1); //Read data from slave;
        SlaveSelect1 = 1; //Set the SS, ending communication with the slave;
    }
    else if (Slave == 2)
    {
        unsigned int x;
        SlaveSelect2 = 0; //Clear the SS, enabling the slave; 
        Delay10TCYx(30); //500 TCY Delay;
        for (x = 0; x < ReceiveLodeSize; x++)
            ReceiveLode[x] = MReceiveSPI(2); //Read data from slave;
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

void SPIReassembleLode(void)
{
    unsigned int z = 0;
    unsigned int y = 0;
    double dub;
    DoublePtr = (unsigned char*) &dub;
    for (z = 0; z != ReceiveLodeSize; z += 3)
    {
        DoublePtr[0] = ReceiveLode[z];
        DoublePtr[1] = ReceiveLode[z + 1];
        DoublePtr[2] = ReceiveLode[z + 2];

        DataLode[y] = dub;
        y++;
        dub = 0;
    }
}

unsigned char checksum(void)
{
    unsigned char y;
    int sum = 0;
    for (y = 0; y != 3; y++)
        sum += DoubleSPIM[y];
    sum = sum & 0xFF; //Truncate the int;
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