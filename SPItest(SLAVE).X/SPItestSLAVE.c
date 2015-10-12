#include <pic18f8722.h>
#include <spi.h>
#include <string.h>
#include "LCD.h"
#include "SPISlave.h"

#pragma config OSC = HSPLL
#pragma config FCMEN = OFF
#pragma config WDT = OFF

void initialize(void);
void interrupt ISR(void);
void LCDbreakDouble(double dubs);

void main(void)
{
    unsigned char temporary, x;
    initialize();
    SPIDisassembleDouble(355.89); //This is the double that will be sent if the slave does not understand anything;

    SlaveReady = 0; //Start the slave in the ready condition;
    SSP2BUF = dummy_byte;

    while (1)
    {
        if (SPIflag == 1)
        {
            SPIflag = 0;
            if ((Command == 0x02) || (Command == 0x03) || (Command == 0x04) || (Command == 0x06) || (Command == 0x08))
            {
                if (Command == 0x02)
                    SPIDisassembleDouble(CurrentAngle);
                else if (Command == 0x03)
                    SPIDisassembleDouble(CurrentVelocity);
                else if (Command == 0x04)
                    SPIDisassembleDouble(Kp);
                else if (Command == 0x06)
                    SPIDisassembleDouble(Ki);
                else if (Command == 0x08)
                    SPIDisassembleDouble(Kd);
                SlaveReady = 0;
                for (x = 0; x < 4; x++) //Test sending multiple bytes;
                    SendSPI1(DoubleSPIS[x]);
                temporary = SSP2BUF;
            }
            else if ((Command == 0x01) || (Command == 0x05) || (Command == 0x07) || (Command == 0x09))
            {
                SlaveReady = 0;
                for (x = 0; x != 4; x++)
                    DoubleSPIS[x] = ReceiveSPI1();
                if (Command == 0x01)
                {
                    SetAngle = SPIReassembleDouble();
                    PIDEnableFlag = 0x03; //This flag sets two bits.  Bit 0 will be used by the main loop to determine whether or not the PID is active.  Bit 1 will be used to determine whether or not this is a new angle that is being sent;
                    LCDbreakDouble(SetAngle);
                }
                else if (Command == 0x05)
                {
                    Kp = SPIReassembleDouble();
                    LCDbreakDouble(Kp);
                }
                else if (Command == 0x07)
                {
                    Ki = SPIReassembleDouble();
                    LCDbreakDouble(Ki);
                }
                else if (Command == 0x09)
                {
                    Kd = SPIReassembleDouble();
                    LCDbreakDouble(Kd);
                }
                temporary = SSP2BUF;
            }
            PIE3bits.SSP2IE = 1;
        }
    }
}

void initialize(void)
{
    lcdInit();
    SPIInit();
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}

void interrupt ISR(void)
{
    if (PIR3bits.SSP2IF == 1)
    {
        SPIInt();
    }
}

void LCDbreakDouble(double dubs)
{
    unsigned int temp1, temp2;
    unsigned int tempDub;

    lcdCommand(0x01);
    lcdGoTo(0x40);
    tempDub = dubs * 100;
    temp1 = tempDub / 10000;
    temp2 = tempDub % 10000;
    if (temp1 != 0)
        lcdChar(temp1 + 0x30);
    temp1 = temp2 / 1000;
    temp2 = temp2 % 1000;
    if (temp1 != 0)
        lcdChar(temp1 + 0x30);
    temp1 = temp2 / 100;
    temp2 = temp2 % 100;
    lcdChar(temp1 + 0x30);
    lcdChar('.');
    temp1 = temp2 / 10;
    temp2 = temp2 % 10;
    lcdChar(temp1 + 0x30);
    lcdChar(temp2 + 0x30);
} 