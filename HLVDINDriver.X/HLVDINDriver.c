#include <xc.h>
#include "EEPROM.h"
#include "SerComm.h"

#pragma config OSC = HSPLL
#pragma config FCMEN = OFF
#pragma config WDT = OFF

void initialize(void);
void interrupt ISR(void);

void main(void)
{
    initialize();
    while (1)
    {
        if (EEReadChar(0x01) == 0x05)
        {
            SerTxStr("HelloWorld!!!");
            SerNL();
        }
        else
        {
            SerTxStr("Nope");
            SerNL();
        }
            while (1);
    }
}

void initialize(void)
{
    SerInit();
    EEPROMInit();

    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}

void interrupt ISR(void)
{
    if (PIR2bits.HLVDIF == 1)
    {
        EEWriteChar(0x01, 0x05);
    }
}
