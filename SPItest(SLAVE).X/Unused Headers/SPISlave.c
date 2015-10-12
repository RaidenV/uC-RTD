/* Please note that the final version of this program should be altered such that
 * the controller operates over MSSP1, not MSSP2.  The program is this way
 * because the LCD screen is connected via the MSSP1 pins on the dev board;
 */
#include "SPISlave.h"

unsigned char* DoublePtrSPI;
unsigned char DoubleSPIS[7];
//These Variables are here temporarily in order to drive the SPI routine;
double Kp;
double Ki;
double Kd;
double SetAngle;
double CurrentAngle;
double CurrentVelocity;
unsigned char PIDEnableFlag;

void SPISInit(void)
{
    OpenSPI2(SLV_SSON, MODE_00, SMPMID); //Open the chip in Slave mode with the SS
    TRISCbits.RC6 = 0; //The "SlaveReady" pin;
    TRISAbits.RA2 = 0; //The "SPI Active" LED;
    SlaveReady = 0; //Start with the slave ready;
    SPILED = 0; //Start with the SPILED off;

    /*This section should be enabled in the final revision.*/
    //    RCONbits.IPEN = 1; //Enable the Interrupt priority;
    //    IPR1bits.SSP1IP = 0; //Set the SPI to Low Priority;
    //    PIE1bits.SSP1IE = 1; //Enable the SPI Interrupt;
    
    PIE3bits.SSP2IE = 1;
}

void SendSPI1(unsigned char data)
{
    SSP2BUF = data; //Put the data into the SSPBUF;   
    unsigned char temp;
    PIR3bits.SSP2IF = 0; //Clear the MSSP2 Interrupt flag;
    temp = SSP2BUF; //Clear the SSPBUF; 
    while(!PIR3bits.SSP2IF); //Wait for the byte to be clocked out;
    SSP2CON1bits.SSPOV2 = 0;
    PIR3bits.SSP2IF = 0; //Clear the flag;
}

void SendStrSPI1(unsigned char* chPtr)
{
    while (chPtr++)
        SendSPI1(*chPtr);
}

unsigned char ReceiveSPI1Int(void)
{
    unsigned char tempChar;
    while (SSP2STATbits.BF == 0);
    tempChar = SSP2BUF;
    return tempChar;
}

unsigned char ReceiveSPI1(void)
{
    unsigned char tempChar;
    PIR3bits.SSP2IF = 0;
    SSP2BUF = 0x00;
    while(!PIR3bits.SSP2IF);
    return SSP2BUF;
}



void ReceiveStrSPI1(unsigned char* chPtr, unsigned char length)
{
    unsigned char z;
    for (z = 0; z < length; z++)
    {
        chPtr[z] = ReceiveSPI1();
    }
}

void S_RW_Routine(void)
{
    unsigned char Command, x;

    SlaveReady = 1; //It is crucial that the slave is always reset to the ready position at the end of each loop of this routine, otherwise the master will never be able to send to the slave;  This immediate "Not Ready" command is to ensure that the slave has setup time for the next write cycle;
    Command = ReceiveSPI1(); //Receive the command (this function should basically fall straight through as this is interrupt driven and therefore can only be triggered by the reception of a byte);
    SPILED = 1;

    if ((Command == 0x01) || (Command == 0x05) || (Command == 0x07) || (Command == 0x09))
    {
        if (Command == 0x01)
        {
            SlaveReady = 0; //After the slave has determined which command was sent, let the master know that the slave is ready;
            for (x = 0; x < 3; x++)
            {
                DoubleSPIS[x] = ReceiveSPI1(); //Receive 3 bytes (this may have to be looked over in the event that the master is sending data too quickly for the slave as the slave will have to perform the operation of incrementing the counter every time while the master will send a byte;
            }
            SlaveReady = 1; //Let the master know that the slave needs time to process;
            SetAngle = SPIReassembleDouble(); //Convert the data to a double;
            PIDEnableFlag = 1; //In this specific instance, the PIDEnableFlag is raised; This means that the user on the PC end has sent a new angle to be approached by the PID loop, which in turn deactivates the joystick;
        }
        else if (Command == 0x05)
        {
            SlaveReady = 0;
            for (x = 0; x < 3; x++)
            {
                DoubleSPIS[x] = ReceiveSPI1();
            }
            SlaveReady = 1;
            Kp = SPIReassembleDouble();
        }
        else if (Command == 0x07)
        {
            SlaveReady = 0;
            for (x = 0; x < 3; x++)
            {
                DoubleSPIS[x] = ReceiveSPI1();
            }
            SlaveReady = 1;
            Ki = SPIReassembleDouble();
        }
        else if (Command == 0x09)
        {
            SlaveReady = 0;
            for (x = 0; x < 3; x++)
            {
                DoubleSPIS[x] = ReceiveSPI1();
            }
            SlaveReady = 1;
            Kd = SPIReassembleDouble();
        }

            
    }

    else if ((Command == 0x02) || (Command == 0x03) || (Command == 0x04) || (Command == 0x06) || (Command == 0x08))
    {
        if (Command == 0x02)
        {
            SPIDisassembleDouble(CurrentAngle); //While the master is held at "Slave Busy" prepare the data;
            SlaveReady = 0; //Signal that the slave is ready;
            for (x = 0; x < 3; x++)
            {
                SendSPI1(DoubleSPIS[x]); //I'm going to cross my fingers and hope that the master will be able to handle this;  The slave is going to want to send data fairly quickly and I'm still not 100% sure how to approximate the times when the slave will be able to utilize on the Master's clocks;
            }
        }
        else if (Command == 0x03)
        {
            SPIDisassembleDouble(CurrentVelocity);
            SlaveReady = 0;
            for (x = 0; x < 3; x++)
            {
                SendSPI1(DoubleSPIS[x]);
            }
        }
        else if (Command == 0x04)
        {
            SPIDisassembleDouble(Kp);
            SlaveReady = 0;
            for (x = 0; x < 3; x++)
            {
                SendSPI1(DoubleSPIS[x]);
            }
        }
        else if (Command == 0x06)
        {
            SPIDisassembleDouble(Ki);
            SlaveReady = 0;
            for (x = 0; x < 3; x++)
            {
                SendSPI1(DoubleSPIS[x]);
            }
        }
        else if (Command == 0x08)
        {
            SPIDisassembleDouble(Kd);
            SlaveReady = 0;
            for (x = 0; x < 3; x++)
            {
                SendSPI1(DoubleSPIS[x]);
            }
        }
    }
    x = SSP1BUF; //Clear the SSP1BUF of any extra data;
    SlaveReady = 0; //Return the slave to the ready condition; This bit can, of course, be set elsewhere in the program (most notably when the PID loop is processing);
    SPILED = 0; //Turn the SPI LED off;
}

unsigned char DataReadySPI1(void)
{
    if (SSP1STATbits.BF == 1)
        return 1;
    else
        return 0;
}

void SPIDisassembleDouble(double dub)
{
//    DoublePtrSPI = (unsigned char*) &dub; //This sets the pointer to the location of the first byte of the double;
//    DoubleSPI[0] = DoublePtrSPI[0]; //The following lines extract the double byte-by-byte into the DDouble variable;
//    DoubleSPI[1] = DoublePtrSPI[1]; //This way, whenever the double is operated upon, the referenced double is not altered;
//    DoubleSPI[2] = DoublePtrSPI[2];
    
     unsigned int tempDub, temp1;
    DoubleSPIS[0] = 0x30;    
    tempDub = dub * 100;
    DoubleSPIS[1] = (tempDub / 10000) + 0x30;
    temp1 = tempDub % 10000;
    DoubleSPIS[2] = (temp1 / 1000) + 0x30;
    temp1 = temp1 % 1000;
    DoubleSPIS[3] = (temp1 / 100) + 0x30;
    temp1 = temp1 % 100;
    DoubleSPIS[4] = '.';
    DoubleSPIS[5] = (temp1 / 10) + 0x30;
    DoubleSPIS[6] = (temp1 % 10) + 0x30;
}

double SPIReassembleDouble(void)
{
    double dub;
    DoublePtrSPI = (unsigned char*) &dub; //This sets the pointer to the location of the first byte of the double;
    unsigned char x; //Loop variable;

    for (x = 3; x > 0; --x) //
    {
        DoublePtrSPI[x - 1] = DoubleSPIS[x - 1]; //This reassembles the double at it's location;
    }

    return dub; //Return the reconstructed double;
}

