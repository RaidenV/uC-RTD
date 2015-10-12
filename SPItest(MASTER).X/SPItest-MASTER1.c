#include <pic18f2620.h>
#include <spi.h>
#include <delays.h>
#include "SerComm.h"

#pragma config OSC = INTIO7
#pragma config WDT = OFF
#pragma config FCMEN = OFF

unsigned char array[10] = {'1', '1', '1', '2', '2', '2', '3', '3', '3', '4'};
void initialize(void);
void interrupt ISR(void);
void RCInt(void);

void main(void)
{
    unsigned char x = 0;
    initialize();
    
    while (1);
}

void interrupt ISR(void)
{
    if(PIR1bits.RCIF == 1)
    {
        RCInt();
    }
}

void RCInt(void)
{
    unsigned char x;
    x = RCREG;
    SerTx(x);
    WriteSPI(x);
    PIR1bits.RCIF = 0;
}

void initialize(void)
{
    OSCCON = 0x72;
    OSCTUNEbits.PLLEN = 1;
    while (OSCCONbits.IOFS == 0);
    OpenSPI(SPI_FOSC_4, MODE_01, SMPMID);
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    SerInit();
}

