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
 * - 115200 Baud Rate (this is considering the internal oscillator frequency being 8 MHz);
 */

void SerInit(void)
{
    //    TXSTA = 0x24; //Asynchronous, 8-bit, High Baud Rate, Sync Break Transmission Complete, Transmit enabled
    //    RCSTA = 0x90; //Receive Enabled, Serial Port Enabled, Continuous Receive Enabled
    //    BAUDCON = 0x00; //8-Bit Baud Rate Generator
    //    SPBRG = 51;

    TXSTA1 = 0x24; //Asynchronous, 8-bit, High Baud Rate, Sync Break Transmission Complete, Transmit enabled
    RCSTA1 = 0x90; //Receive Enabled, Serial Port Enabled, Continuous Receive Enabled
    BAUDCON = 0x00; //8-Bit Baud Rate Generator
    SPBRG = 21;

    RCONbits.IPEN = 1; //Enable interrupt priority;
    IPR1bits.RCIP = 0; //Set the RCIP to high; (for the purposes of this PID driver, it will be low);
    PIE1bits.RC1IE = 1; //Enable the Receive interrupt;

    SerTxStr("Welcome to the world of tomorrow!!!");
}

/* SerTx
 * Serially sends a single byte;
 */
void SerTx(unsigned char c)
{
    TXREG1 = c;
    while (PIR1bits.TX1IF == 0);
}

/* SerTxStr
 * Serially sends a string of data;
 */
void SerTxStr(unsigned char* string)
{
    while (*string)
        SerTx(*string++);
}

/* SerRx
 * Receives a byte of data; 
 */

unsigned char SerRx(void)
{
    return RCREG1;
}

/* SerRxStr
 * Serially receives a string of data with bounds checking;
 */

void SerRxStr(unsigned char* str)
{
    unsigned short length = sizeof (str) / sizeof (str[0]); //Checks length of string
    unsigned short x = 0;
    while ((RCREG1 != carriageReturn) || (x != length))
    {
        str[x] = RCREG1;
        x++;
    }
}

void breakDouble(double dubs)
{
    unsigned int temp1, temp2;
    unsigned int tempDub;

    tempDub = dubs * 100;
    temp1 = tempDub / 10000;
    temp2 = tempDub % 10000;
    if (temp1 != 0)
        SerTx(temp1 + 0x30);
    temp1 = temp2 / 1000;
    temp2 = temp2 % 1000;
    if (temp1 != 0)
        SerTx(temp1 + 0x30);
    temp1 = temp2 / 100;
    temp2 = temp2 % 100;
    SerTx(temp1 + 0x30);
    SerTx('.');
    temp1 = temp2 / 10;
    temp2 = temp2 % 10;
    SerTx(temp1 + 0x30);
    SerTx(temp2 + 0x30);
    SerTx(newLine);
    SerTx(carriageReturn);
}