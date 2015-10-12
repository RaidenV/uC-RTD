#include "SPISlave.h"

unsigned char SPIflag;
unsigned char Command;
unsigned char dummy_byte = 0x00;
unsigned char* DoublePtr;
unsigned char DoubleSPIS[4];
unsigned char PIDEnableFlag;
double SetAngle;
double CurrentAngle;
double CurrentVelocity;
double Kp;
double Ki;
double Kd;

void SPIInit(void)
{
    OpenSPI2(SLV_SSON, MODE_00, SMPMID);
    TRISCbits.RC6 = 0; //Set the SlaveReady pin as an output;
    PIE3bits.SSP2IE = 1; //Enable the MSSP1 Interrupt on byte received;
}

void SPIInt(void)
{
    Command = SSP2BUF;
    SlaveReady = 0; //Set the SlaveReady pin, stopping the master from initiating a transfer until the slave is ready;
    PIR3bits.SSP2IF = 0; //Clear the interrupt flag;
    PIE3bits.SSP2IE = 0; //Disable the interrupt so that subsequent transfers don't cause interrupts;
    SPIflag = 1; //Set the SPIflag;
}

void SendSPI1(unsigned char data)
{
    SSP2BUF = data; //Put the data into the SSPBUF;   
    unsigned char temp;
    PIR3bits.SSP2IF = 0; //Clear the MSSP2 Interrupt flag;
    temp = SSP2BUF; //Clear the SSPBUF; 
    while (!PIR3bits.SSP2IF); //Wait for the byte to be clocked out;
    SSP2CON1bits.SSPOV2 = 0;
    PIR3bits.SSP2IF = 0; //Clear the flag;
}

unsigned char ReceiveSPI1(void)
{
    SSP2BUF = 0x00; //Load dummy byte into SBUF;
    while (SSP2STATbits.BF == 0); //Wait for transmission to complete;
    return SSP2BUF;
}

void SPIDisassembleDouble(double dub)
{
    DoublePtr = (unsigned char*) &dub; //This sets the pointer to the location of the first byte of the double;
    DoubleSPIS[0] = DoublePtr[0]; //The following lines extract the double byte-by-byte into the DDouble variable;
    DoubleSPIS[1] = DoublePtr[1]; //This way, whenever the double is operated upon, the referenced double is not altered;
    DoubleSPIS[2] = DoublePtr[2];
    DoubleSPIS[3] = GenerateChecksum();
}

unsigned char GenerateChecksum(void)
{
    unsigned char y, sum = 0;
    for (y = 0; y != 3; y++)
        sum += DoubleSPIS[y];
    return sum;
}

double SPIReassembleDouble(void)
{
    double dub;
    DoublePtr = (unsigned char*) &dub;
    DoublePtr[0] = DoubleSPIS[0];
    DoublePtr[1] = DoubleSPIS[1];
    DoublePtr[2] = DoubleSPIS[2];

    return dub;
}
