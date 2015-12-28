#include "SerComm.h"

/* This initialization routine assumes a 40 MHz clock
 * The initialization routine opens communication on TX1 and RX1 with the
 * following conditions:
 * 
 * - Asynchronous
 * - 8-bit mode
 * - High Baud Rate
 * - Sync Break tranmission completed
 * - Transmit enabled
 * - Receive enabled
 * - Serial Port Enabled
 * - Continuous Receive enabled
 * - 8-bit Baud Rate Generator
 * - 115200 Baud Rate (this is considering the external oscillator frequency is a set 10 MHz);
 */

void SerInit(void)
{
    TXSTA1 = 0x24; //Asynchronous, 8-bit, High Baud Rate, Sync Break Transmission Complete, Transmit enabled
    RCSTA1 = 0x90; //Receive Enabled, Serial Port Enabled, Continuous Receive Enabled
    BAUDCON = 0x00; //8-Bit Baud Rate Generator
    SPBRG = 21;

    PIE1bits.RC1IE = 1; //Enable the Receive interrupt;

    SerTxStr("Welcome to the world of tomorrow!!!");  //Emerge from your cryo-tube circa 3000 AD;
    SerNL();
}

/*--------------------------------------------------------\
| SerTx                                                    |
|     													   |
| Sends a single byte;                                     |
\---------------------------------------------------------*/
void SerTx(unsigned char c)
{
    TXREG1 = c;
    while (PIR1bits.TX1IF == 0);
}

/*--------------------------------------------------------\
| SerTxStr                                                 |
|     													   |
| Sends a string of bytes;                                 |
\---------------------------------------------------------*/
void SerTxStr(unsigned char* string)
{
    while (*string)
        SerTx(*string++);
}

/*--------------------------------------------------------\
| SerNL                                                    |
|     													   |
| Simple function for writing a generating a new line;     |
\---------------------------------------------------------*/
void SerNL(void)
{
    SerTx(newLine);
    SerTx(carriageReturn);
}

/*--------------------------------------------------------\
| SerRx                                                    |
|     													   |
| Receives a single byte;                                  |
\---------------------------------------------------------*/
unsigned char SerRx(void)
{
    return RCREG1;
}

/*--------------------------------------------------------\
| SerRxStr                                                 |
|     													   |
| Receives a string of bytes;                              |
\---------------------------------------------------------*/
void SerRxStr(unsigned char* str)
{
    unsigned short x = 0;
    while ((RCREG1 != carriageReturn) || (x != strlen(str)))
    {
        str[x] = RCREG1;
        x++;
    }
}

/*--------------------------------------------------------\
| breakDouble                                              |
|     													   |
| Takes a double and breaks it down to a set of characters |
| for transmission, with three digits of precision.  Also  |
| eliminates leading zeros.  This algorithm considers that |
| the maximum double is 360, as this is reporting angle.   |
\---------------------------------------------------------*/
void breakDouble(double dubs)
{
    unsigned short long temp1, temp2;
    unsigned short long tempDub;
    unsigned char LeadingFlag = 0;

    if (dubs >= 0)
    {
        tempDub = dubs * 1000;
        temp1 = tempDub / 100000;
        if (temp1 != 0)
            LeadingFlag = 1;
        temp2 = tempDub % 100000;
        if (LeadingFlag)
            SerTx(temp1 + 0x30);
        temp1 = temp2 / 10000;
        temp2 = temp2 % 10000;
        if (temp1 != 0)
            LeadingFlag = 1;
        if (LeadingFlag)
            SerTx(temp1 + 0x30);
        temp1 = temp2 / 1000;
        temp2 = temp2 % 1000;
        SerTx(temp1 + 0x30);
        SerTx('.');
        temp1 = temp2 / 100;
        temp2 = temp2 % 100;
        SerTx(temp1 + 0x30);
        temp1 = temp2 / 10;
        temp2 = temp2 % 10;
        SerTx(temp1 + 0x30);
        SerTx(temp2 + 0x30);
    }

    else if (dubs < 0)
    {
        dubs = dubs * -1;
        tempDub = dubs * 1000;
        temp1 = tempDub / 100000;
        SerTx('-');
        if (temp1 != 0)
            LeadingFlag = 1;
        temp2 = tempDub % 100000;
        if (LeadingFlag)
            SerTx(temp1 + 0x30);
        temp1 = temp2 / 10000;
        temp2 = temp2 % 10000;
        if (temp1 != 0)
            LeadingFlag = 1;
        if (LeadingFlag)
            SerTx(temp1 + 0x30);
        temp1 = temp2 / 1000;
        temp2 = temp2 % 1000;
        SerTx(temp1 + 0x30);
        SerTx('.');
        temp1 = temp2 / 100;
        temp2 = temp2 % 100;
        SerTx(temp1 + 0x30);
        temp1 = temp2 / 10;
        temp2 = temp2 % 10;
        SerTx(temp1 + 0x30);
        SerTx(temp2 + 0x30);
    }
}

/*--------------------------------------------------------\
| SendLode                                                 |
|     													   |
| Repeated application of several of the aforementioned fu-|
| nctions.  Adds the sentinels "-=Begin=-" and "-=End=-".  |
\---------------------------------------------------------*/
void SendLode(double* Deliverables, unsigned int size)
{
    unsigned int z;
    unsigned int limit;
    double time = 0;
    double multiplier = 0.002; //Knowing the clock which the slave is set to record data at, this is a time multiplier;

    SerTxStr("-=Begin=-"); //Begin sentinel;
    SerNL();
    for (z = 0; z < size - 3; z++)
    {
        time = multiplier * z;
        breakDouble(time);
        SerTxStr("           ");
        breakDouble(Deliverables[z]);
        SerNL();
    }
    breakDouble(Deliverables[size - 3]); //Kp;
    SerNL();
    breakDouble(Deliverables[size - 2]); //Ki;
    SerNL();
    breakDouble(Deliverables[size - 1]); //Kd;
    SerNL();
    SerTxStr("-=End=-"); //End sentinel;
    SerNL();
}
