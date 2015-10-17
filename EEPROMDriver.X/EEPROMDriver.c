#include <pic18f8722.h>
#include "EEPROM.h"
#include "SerComm.h"

#pragma config OSC = HSPLL
#pragma config WDT = OFF
#pragma config FCMEN = OFF

void initialize(void);

void main(void)
{
    initialize();

    double hello = 255;
    unsigned char ch = 6;

    Sernl();
    Sernl();
    SerTxStr("Here is the double string: ");
    breakDouble(hello);
    Sernl();
    SerTxStr("Here is the char: ");
    SerTx(ch + 0x30);
    Sernl();

    EEWriteDouble(0x00, hello);
    EEWriteChar(0x04, ch);

    hello = 0;
    ch = 0;

    Sernl();
    hello = EEReadDouble(0x00);
    ch = EEReadChar(0x04);

    SerTxStr("Here is the double string again: ");
    breakDouble(hello);
    SerTxStr("And here is the char: ");
    SerTx(ch + 0x30);

    while (1);
}

void initialize(void)
{
    SerInit();
    EEPROMInit();
}
