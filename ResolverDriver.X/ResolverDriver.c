#include <pic18f8722.h>
#include "LCD.h"
#include "ResolverToDigital.h"
#include "SerComm.h"

#pragma config OSC = HSPLL
#pragma config WDT = OFF
#pragma config FCMEN = OFF
#pragma config PWRT = ON

void initialize(void);
void C2LCD(double);

unsigned char Angle[5] = {'3', '4', '1', '2', '2'};
double fiveAngle[5];
double average;
unsigned char x;

void main(void)
{
    
    initialize();
    lcdGoTo(0x00);
    lcdWriteString("Angle: ");
    while(1)
    {      
        C2LCD(RTD2Angle(ReadRTDpos()));
        SerTxStr("Angle: ");
        SerTx(Angle[0]);
        SerTx(Angle[1]);
        SerTx(Angle[2]);
        SerTx('.');
        SerTx(Angle[3]);
        SerTx(Angle[4]);
        SerTx(newLine);
        SerTx(carriageReturn);
    }
    
}

void initialize(void)
{
    while(OSCCONbits.OSTS == 0);
    
    SerInit();
    RTDInit();
    lcdInit();
}

void C2LCD(double angle)
{
    
    lcdGoTo(0x09);
    unsigned int temp, temp2;
    temp = angle * 100;
    temp2 = temp / 10000;
    lcdChar(temp2 + 0x30);
    Angle[0] = (temp2 + 0x30);
    temp = temp % 10000;
    temp2 = temp / 1000;
    lcdChar(temp2 + 0x30);
    Angle[1] = (temp2 + 0x30);
    temp = temp % 1000;
    temp2 = temp/100;
    lcdChar(temp2 + 0x30);
    Angle[2] = (temp2 + 0x30);
    lcdChar('.');
    temp = temp % 100;
    temp2 = temp/10;
    lcdChar(temp2 + 0x30);
    Angle[3] = (temp2 + 0x30);
    temp = temp % 10;
    lcdChar(temp + 0x30);
    Angle[4] = (temp2 + 0x30);
}
