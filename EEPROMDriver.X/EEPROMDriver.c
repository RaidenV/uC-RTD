#include <pic18f8722.h>
#include "EEPROM.h"
#include "SerComm.h"

#pragma config OSC = HSPLL
#pragma config WDT = OFF
#pragma config FCMEN = OFF

void initialize(void);

double hello = 3254.2130;
unsigned char ch = 6;

void main(void)
{
    
    initialize();
    
    EEWriteDouble(0x00, hello);
    EEWriteChar(0x08, ch);
    
    ch = 0;
    hello = 0;
    
    SerTxStr("Here is the double string: ");
    SerTxStr(DDouble);
    SerTx(newLine);
    SerTx(carriageReturn);
    hello = EEReadDouble(0x00);
    ch = EEReadChar(0x08);
    
    SerTxStr("Here is the double string again: ");
    SerTxStr(DDouble);
    SerTxStr("And here is the char");
    SerTx(ch + 0x30);
    
    while(1);
}

void initialize(void)
{
    SerInit();
    EEPROMInit();
}
