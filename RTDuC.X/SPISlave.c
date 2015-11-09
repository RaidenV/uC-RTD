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

unsigned char timer4 = 0x00;

void SPIInit(void)
{
    TRISCbits.RC4 = 1;
    TRISCbits.RC5 = 0;
    TRISFbits.RF7 = 1;
    TRISCbits.RC6 = 0; //Set the SlaveReady pin as an output;
    OpenSPI1(SLV_SSON, MODE_00, SMPMID);
    PIE1bits.SSP1IE = 1; //Enable the MSSP1 Interrupt on byte received;
    T4CONbits.T4CKPS = 0x3; //Set Timer4, the SSPIF break timer, to prescale value of 16;
}

/* SPIInt
 * Handles the SPI Interrupt;
 */
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

/* SendSPI1
 * Transmits data via the MSSP1 module on the controller;
 */
void SendSPI1(unsigned char data)
{
    SSP1BUF = data; //Put the data into the SSPBUF;   
    unsigned char temp;
    PIR1bits.SSP1IF = 0; //Clear the MSSP1 Interrupt flag;
    temp = SSP1BUF; //Clear the SSPBUF; 
    PIR3bits.TMR4IF = 0;
    T4CONbits.TMR4ON = 1;
    TMR4 = timer4;
    while (!PIR1bits.SSP1IF) //Wait for the byte to be clocked out;
    {
        if (PIR3bits.TMR4IF == 1) //This is sort of a last shot.  I've noticed that the program repeatedly hangs at this point for whatever reason.  If the program takes too long to respond, I break from this while routine by setting the flag high;
        {
            SPIRestart();
        }
    }
    SSP1CON1bits.SSPOV1 = 0;
    PIR1bits.SSP1IF = 0; //Clear the flag;
}

/* ReceiveSPI1
 * Receives data via the MSSP1 module on the controller;
 */
unsigned char ReceiveSPI1(void)
{
    SSP1BUF = 0x00; //Load dummy byte into SBUF;
    PIR3bits.TMR4IF = 0;
    T4CONbits.TMR4ON = 1;
    TMR4 = timer4;
    while (!PIR1bits.SSP1IF) //Wait for transmission to complete;
    {
        if (PIR3bits.TMR4IF == 1)
        {
            SPIRestart();
        }
    }
    PIR1bits.SSP1IF = 0; //Clear the flag;
    SSP1CON1bits.SSPOV1 = 0;
    return SSP1BUF;
}

/* SPIDisassembleDouble
 * Breaks a double from a solid three bytes into individual, transmittable
 * bytes;
 */
void SPIDisassembleDouble(double dub)
{
    DoublePtr = (unsigned char*) &dub; //This sets the pointer to the location of the first byte of the double;
    DoubleSPIS[0] = DoublePtr[0]; //The following lines extract the double byte-by-byte into the DDouble variable;
    DoubleSPIS[1] = DoublePtr[1]; //This way, whenever the double is operated upon, the referenced double is not altered;
    DoubleSPIS[2] = DoublePtr[2];
    DoubleSPIS[3] = GenerateChecksum();
}

/* SPIDisassembleLode
 * Disassembles all the data into the same single-byte structure as the previous
 * module;
 */
void SPIDisassembleLode(double* Data, unsigned char* Transmit)
{
    unsigned int y = 0;
    double dub;
    DoublePtr = (unsigned char*) &dub;
    for (y = 0; y != 1809; y += 3)
    {
        dub = Data[y / 3];
        Transmit[y] = DoublePtr[0];
        Transmit[y + 1] = DoublePtr[1];
        Transmit[y + 2] = DoublePtr[2];
    }
}

/* GenerateChecksum
 * Generates a checksum byte for the master to interpret and determine whether
 * data needs to be resent;
 */
unsigned char GenerateChecksum(void)
{
    if ((Command > 0x00) && (Command < 0x0B)) //This has to be revised...  This may be a method of correcting for errors, but if the command is not understood by the slave and it happens to be the master sending data to the slave, there could be an issue.  On top of that, I've already handled this case elsewhere in the code, so the code actually never makes it here;
    {
        unsigned char y, sum = 0;
        for (y = 0; y != 3; y++)
            sum += DoubleSPIS[y];
        return sum;
    }
    else
        return 0xFF; //If the command was not understood, send a unique byte to the master;
}

/* SPIReassembleDouble
 * Reassembles single bytes received by the master into doubles readable by the
 * slave's various modules;
 */
double SPIReassembleDouble(void)
{
    double dub;
    DoublePtr = (unsigned char*) &dub;
    DoublePtr[0] = DoubleSPIS[0];
    DoublePtr[1] = DoubleSPIS[1];
    DoublePtr[2] = DoubleSPIS[2];

    return dub;
}

/* SPIRestart
 * I've found that there is a higher degree of successful transmissions if the
 * SPI module of the slave (which is the module which frequently errors as
 * opposed to the master) is reset at certain points in the code.  This helps
 * clear any erroneous bits set in the slave's MSSP registers;
 */
void SPIRestart(void)
{
    unsigned char temp;
    PIR1bits.SSPIF = 1;
    temp = SSP1BUF;
    CloseSPI1();
    OpenSPI1(SLV_SSON, MODE_00, SMPMID);
}
