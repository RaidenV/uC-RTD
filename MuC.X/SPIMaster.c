#include "SPIMaster.h"

double Kp;
double Ki;
double Kd;
double CurrentAngle;
double CurrentVelocity;
double AZlast;
double ELlast;
unsigned char* DoublePtr;
unsigned char DoubleSPIM[4];
const unsigned int ReceiveLodeSize = 1809;
const unsigned int DataLodeSize = 603;
unsigned char ReceiveLode[1809];
double DataLode[603];

/*--------------------------------------------------------\
| SPIInitM                                                 |
|     													   |
| Initializes the SPI module and it's required ports;      |
\---------------------------------------------------------*/
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

/*--------------------------------------------------------\
| MSendSPI                                                 |
|     													   |
| Takes the character to send and the Slave number and tra-|
| nsmits them via SPI;                                     |
\---------------------------------------------------------*/
void MSendSPI(unsigned char data, unsigned char Slave)
{
    SelectSlaveStart(Slave);
    Delay10TCYx(1); //delay for 10 clock cycles to ensure the slave is ready;
    unsigned char tempChar;
    tempChar = SSPBUF;
    PIR1bits.SSPIF = 0;
    while (SlaveQuery(Slave));
    SSPBUF = data;
    while (!PIR1bits.SSPIF);
    data = SSPBUF;
    SelectSlaveEnd(Slave);
}

/*--------------------------------------------------------\
| MReceiveSPI                                              |
|     													   |
| Takes the Slave number (either 1 or 2) and returns a sin-|
| gle character received via SPI;                          |
\---------------------------------------------------------*/
unsigned char MReceiveSPI(unsigned char Slave)
{
    unsigned char tempCH;
    tempCH = SSPBUF; //Clear the buffer;
    PIR1bits.SSPIF = 0; //Clear the MSSP Interrupt Flag; 
    while (SlaveQuery(Slave));
    SSPBUF = 0x00; //Initiate communication by sending a dummy byte;
    while (!PIR1bits.SSPIF); // Wait until transmission is complete;
    PIR1bits.SSPIF = 0;
    return SSPBUF; //Read/return the buffer;
}

/*--------------------------------------------------------\
| MReceiveStrSPI                                           |
|     													   |
| Takes the Slave number (which is then passed to the      |
| MReceiveSPI function and receives the characteristic 4   |
| byte double/checksum code from the Slave via SPI;        |
\---------------------------------------------------------*/
void MReceiveStrSPI(unsigned char Slave)
{
    unsigned char x;

    SelectSlaveStart(Slave);
    Delay10TCYx(30); //300 TCY Delay;
    for (x = 0; x < 4; x++)
        DoubleSPIM[x] = MReceiveSPI(Slave); //Read data from slave;
    SelectSlaveEnd(Slave);
}

/*--------------------------------------------------------\
| MSPIRoutine                                              |
|     													   |
| Takes the Slave number, the Key (a.k.a. the command      |
| issued by the Laptop), and the Value (double) which is to|
| be transmitted, and enters a decision structure which di-|
| ctates what to send to the slave;                        |
\---------------------------------------------------------*/
void MSPIRoutine(unsigned char Slave, unsigned char key, double value)
{
    unsigned char x;
    double SetAngle;
    if ((key == 0x02) || (key == 0x03) || (key == 0x04) || (key == 0x06) || key == 0x08 || key == 0x0B)
    {
        do
        {
            if (key == 0x02)
            {
                MSendSPI(key, Slave); //Write the command byte to the slave;
                MReceiveStrSPI(Slave); //Understanding that I know how long the array will be, the Receive function requires two inputs, the variable which the data is received to, and the Slave which the master communicates with;
                CurrentAngle = SPIReassembleDouble(); //The master then converts the received value into a known value using the first three bytes of the received data;
                if (Slave == 1)
                    SerTxStr("Azimuth = ");
                else if (Slave == 2)
                    SerTxStr("Elevation = ");
                breakDouble(CurrentAngle);
                SerNL();
            }
            else if (key == 0x03)
            {
                MSendSPI(key, Slave); //Write the command byte to the slave;
                MReceiveStrSPI(Slave);
                CurrentVelocity = SPIReassembleDouble();
                if (Slave == 1)
                    SerTxStr("Azimuth Velocity = ");
                else if (Slave == 2)
                    SerTxStr("Elevation Velocity = ");
                breakDouble(CurrentVelocity);
                SerNL();
            }
            else if (key == 0x04)
            {
                MSendSPI(key, Slave); //Write the command byte to the slave;
                MReceiveStrSPI(Slave);
                Kp = SPIReassembleDouble();
                SerTxStr("Kp = ");
                breakDouble(Kp);
                SerNL();
            }
            else if (key == 0x06)
            {
                MSendSPI(key, Slave); //Write the command byte to the slave;
                MReceiveStrSPI(Slave);
                Ki = SPIReassembleDouble();
                SerTxStr("Ki = ");
                breakDouble(Ki);
                SerNL();
            }
            else if (key == 0x08)
            {
                MSendSPI(key, Slave); //Write the command byte to the slave;
                MReceiveStrSPI(Slave);
                Kd = SPIReassembleDouble();
                SerTxStr("Kd = ");
                breakDouble(Kd);
                SerNL();
            }

            else if (key == 0x0B)
            {
                MSendSPI(key, Slave);
                MReceiveStrSPI(Slave);
                SetAngle = SPIReassembleDouble();
                SerTxStr("Set Angle = ");
                breakDouble(SetAngle);
                SerNL();
            }
        }
        while (!checksum()); //While the Checksum does not correlate with the received value;
    }
    else if ((key == 0x01) || (key == 0x05) || (key == 0x07) || (key == 0x09)) //If the key is something that requires the master to send data;
    {
        MSendSPI(key, Slave); //Send the stripped key;
        SPIDisassembleDouble(value); //While we wait for the slave to be ready we'll break down the double;
        SelectSlaveStart(Slave);
        while (SlaveQuery(Slave));
        Delay10TCYx(50);
        for (x = 0; x != 4; x++)
            MSendSPI(DoubleSPIM[x], Slave);
        SelectSlaveEnd(Slave);
        if (key == 0x01 && Slave == 1)
            AZlast = value;
        else if (key == 0x01 && Slave == 2)
            ELlast = value;
    }

}

/*--------------------------------------------------------\
| MSPIRecRoutine                                           |
|     													   |
| Takes the Slave number, and the Key (which is a command  |
| generated by receiving the string "rec" or "REC" from the|
| laptop) and begins the data recording routine on the     |
| slave;                                                   |
\---------------------------------------------------------*/
void MSPIRecRoutine(unsigned char Slave, unsigned char key)
{
    if (Slave == 1)
        SerTxStr("Sending command to Azimuth slave...");
    else if (Slave == 2)
        SerTxStr("Sending command to Elevation slave...");
    SerNL();
    MSendSPI(key, Slave); //Write the command byte to the slave;
    SerTxStr("Waiting on slave...");
    SerNL();
    while (SlaveQuery(Slave));
    MReceiveLodeSPI(Slave);
    SerTxStr("Data received; Reassembling data...");
    SerNL();
    SPIReassembleLode(); //The master then converts the received value into a known value using the first three bytes of the received data;
    SerTxStr("Data reassembled; Transmitting now...");
    SerNL();
    SendLode(DataLode, DataLodeSize);
}

/*--------------------------------------------------------\
| MReceiveLodeSPI                                          |
|     													   |
| Takes the Slave number and, much as the SPIReassembleLode|
| is simply a repeated application of the MReceiveSPI func-|
| tion; places the results in a global variable to be util-|
| ized elsewhere;                                          |
\---------------------------------------------------------*/
void MReceiveLodeSPI(unsigned char Slave)
{
    unsigned int x;
    SelectSlaveStart(Slave);
    Delay10TCYx(30); //250 TCY Delay;
    for (x = 0; x < ReceiveLodeSize; x++)
        ReceiveLode[x] = MReceiveSPI(Slave); //Read data from slave;
    SelectSlaveEnd(Slave);
}

/*--------------------------------------------------------\
| SPIReassembleDouble                                      |
|     													   |
| SPI communication is performed byte-wise.  This function |
| reassembles the double sent by the Slave into a readable |
| double.  The double is returned;                         |
\---------------------------------------------------------*/
double SPIReassembleDouble(void)
{
    double dub;
    DoublePtr = (unsigned char*) &dub; //This sets the pointer to the location of the first byte of the double;
    DoublePtr[0] = DoubleSPIM[0]; //The following lines extract the double byte-by-byte into the DDouble variable;
    DoublePtr[1] = DoubleSPIM[1]; 
    DoublePtr[2] = DoubleSPIM[2];
    return dub; //Return the reconstructed double;
}

/*--------------------------------------------------------\
| SPIDisassembleDouble                                     |
|     													   |
| Takes a single double and converts it into three bytes;  |
\---------------------------------------------------------*/
void SPIDisassembleDouble(double dub)
{
    DoublePtr = (unsigned char*) &dub;
    DoubleSPIM[0] = DoublePtr[0];
    DoubleSPIM[1] = DoublePtr[1];
    DoubleSPIM[2] = DoublePtr[2];
    DoubleSPIM[3] = MGenerateChecksum();
}

/*--------------------------------------------------------\
| SPIReassembleLode                                        |
|     													   |
| Reassembles the entirety of the data sent from the Slave |
| during a record routine.  This simply encapsulates sever-|
| al hundred applications of the SPIReassembleDouble funct-|
| ion;  places the results in a global variable to be util-|
| ized elsewhere;                                          |
\---------------------------------------------------------*/
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

/*--------------------------------------------------------\
| RestartSPI                                               |
|     													   |
| Restarts, and effectively resets, the SPI module.        |
\---------------------------------------------------------*/
void RestartSPI(void)
{
    CloseSPI();
    OpenSPI(SPI_FOSC_16, MODE_00, SMPMID);
}

/*--------------------------------------------------------\
| SelectSlaveStart                                         |
|     													   |
| Takes the Slave number and starts the slave select.      |
\---------------------------------------------------------*/
void SelectSlaveStart(unsigned char Slave)
{
    if (Slave == 1)
        SlaveSelect1 = 0;
    else if (Slave == 2)
        SlaveSelect2 = 0;
}

/*--------------------------------------------------------\
| SelectSlaveEnd                                           |
|     													   |
| Takes the Slave number and ends the slave select.        |
\---------------------------------------------------------*/
void SelectSlaveEnd(unsigned char Slave)
{
    if (Slave == 1)
        SlaveSelect1 = 1;
    else if (Slave == 2)
        SlaveSelect2 = 1;
}

/*--------------------------------------------------------\
| SlaveQuery                                               |
|     													   |
| Queries the SlaveReady lines to ensure the selected slave|
| is ready;                                                |
\---------------------------------------------------------*/
unsigned char SlaveQuery(unsigned char Slave)
{
    if (Slave == 1)
        return SlaveReady1;
    else if (Slave == 2)
        return SlaveReady2;
}

unsigned char MGenerateChecksum(void)
{
    unsigned char z, sum = 0;
    for (z = 0; z != 3; z++)
        sum += DoubleSPIM[z];
    return sum;
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
