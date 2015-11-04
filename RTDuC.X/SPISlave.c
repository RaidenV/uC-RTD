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
    TRISCbits.RC4 = 1;
    TRISCbits.RC5 = 0;
    TRISFbits.RF7 = 1;
    TRISCbits.RC6 = 0; //Set the SlaveReady pin as an output;
    OpenSPI1(SLV_SSON, MODE_00, SMPMID);
    PIE1bits.SSP1IE = 1; //Enable the MSSP1 Interrupt on byte received;
}

void SPIInt(void)
{
    SlaveReady = 1; //Set the SlaveReady pin, stopping the master from initiating a transfer until the slave is ready;
    INTCONbits.GIE = 0; //Turn off interrupts immediately;
    Command = SSP1BUF;
    PIR1bits.SSP1IF = 0; //Clear the interrupt flag;
    PIE1bits.SSP1IE = 0; //Disable the interrupt so that subsequent transfers don't cause interrupts;
    SPIflag = 1; //Set the SPIflag;
}

void SendSPI1(unsigned char data)
{
    SSP1BUF = data; //Put the data into the SSPBUF;   
    unsigned char temp;
    PIR1bits.SSP1IF = 0; //Clear the MSSP2 Interrupt flag;
    temp = SSP1BUF; //Clear the SSPBUF; 
    while (!PIR1bits.SSP1IF); //Wait for the byte to be clocked out;
    SSP1CON1bits.SSPOV1 = 0;
    PIR1bits.SSP1IF = 0; //Clear the flag;
}

unsigned char ReceiveSPI1(void)
{
    SSP1BUF = 0x00; //Load dummy byte into SBUF;
    while (!PIR1bits.SSP1IF); //Wait for transmission to complete;
    PIR1bits.SSP1IF = 0; //Clear the flag;
    return SSP1BUF;
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
