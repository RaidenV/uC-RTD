#include "SPISlave.h"
#include <xc.h>

unsigned char SPIflag;
unsigned char Command;
unsigned char dummy_byte = 0x00;
unsigned char* DoublePtr;
unsigned char DoubleSPIS[4];
unsigned char PIDEnableFlag;
unsigned char RECFlag;
double SetAngle;
double CurrentAngle;
double CurrentVelocity;
double Kp;
double Ki;
double Kd;

unsigned char timer1High = 0xC0;
unsigned char timer1Low = 0x00;

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
    if (Command == 0x0A)
        RECFlag = 1;
    else
        SPIflag = 1; //Set the SPIflag;
}

void SendSPI1(unsigned char data)
{
    SSP1BUF = data; //Put the data into the SSPBUF;   
    unsigned char temp;
    PIR1bits.SSP1IF = 0; //Clear the MSSP2 Interrupt flag;
    temp = SSP1BUF; //Clear the SSPBUF; 
    PIR1bits.TMR1IF = 0;
    T1CONbits.TMR1ON = 1;
    TMR1H = timer1High;
    TMR1L = timer1Low;
    while (!PIR1bits.SSP1IF) //Wait for the byte to be clocked out;
    {
        if (PIR1bits.TMR1IF == 1) //This is sort of a last shot.  I've noticed that the program repeatedly hangs at this point for whatever reason.  If the program takes too long to respond, I break from this while routine by setting the flag high;
        {
            PIR1bits.SSPIF = 1;
            temp = SSP1BUF;
        }
    }
    SSP1CON1bits.SSPOV1 = 0;
    PIR1bits.SSP1IF = 0; //Clear the flag;
}

unsigned char ReceiveSPI1(void)
{
    SSP1BUF = 0x00; //Load dummy byte into SBUF;
    PIR1bits.TMR1IF = 0;
    T1CONbits.TMR1ON = 1;
    TMR1H = timer1High;
    TMR1L = timer1Low;
    while (!PIR1bits.SSP1IF) //Wait for transmission to complete;
    {
        if (PIR1bits.TMR1IF == 1)
            PIR1bits.SSPIF = 1;
    }
    PIR1bits.SSP1IF = 0; //Clear the flag;
    SSP1CON1bits.SSPOV1 = 0;
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

void SPIDisassembleLode(double* Data, unsigned char* Transmit)
{
    unsigned int y = 0;
    double dub;
    DoublePtr = (unsigned char*) &dub;
    for (y = 0; y != 1800; y += 3)
    {
        dub = Data[y / 3];
        Transmit[y] = DoublePtr[0];
        Transmit[y + 1] = DoublePtr[1];
        Transmit[y + 2] = DoublePtr[2];
    }
}

unsigned char GenerateChecksum(void)
{
    if ((Command > 0x00) && (Command < 0x0A)) //This has to be revised...  This may be a method of correcting for errors, but if the command is not understood by the slave and it happens to be the master sending data to the slave, there could be an issue.  On top of that, I've already handled this case elsewhere in the code, so the code actually never makes it here;
    {
        unsigned char y, sum = 0;
        for (y = 0; y != 3; y++)
            sum += DoubleSPIS[y];
        return sum;
    }
    else
        return 0xFF;
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
