#include <xc.h>
#include <spi.h>
#include <string.h>
#include "SerComm.h"
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
    CurrentAngle = 35.29;

    SlaveReady = 0; //Start the slave in the ready condition;
    SSP1BUF = dummy_byte;

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
                temporary = SSP1BUF;
            }
            else if ((Command == 0x01) || (Command == 0x05) || (Command == 0x07) || (Command == 0x09))
            {
                SlaveReady = 0;
                for (x = 0; x != 4; x++)
                    DoubleSPIS[x] = ReceiveSPI1();
                if (Command == 0x01)
                {
                    CurrentAngle = SPIReassembleDouble();
                    PIDEnableFlag = 0x03; //This flag sets two bits.  Bit 0 will be used by the main loop to determine whether or not the PID is active.  Bit 1 will be used to determine whether or not this is a new angle that is being sent;
                    breakDouble(SetAngle);
                }
                else if (Command == 0x05)
                {
                    Kp = SPIReassembleDouble();
                    breakDouble(Kp);
                }
                else if (Command == 0x07)
                {
                    Ki = SPIReassembleDouble();
                    breakDouble(Ki);
                }
                else if (Command == 0x09)
                {
                    Kd = SPIReassembleDouble();
                    breakDouble(Kd);
                }
                temporary = SSP1BUF;
            }
            PIE1bits.SSP1IE = 1;
        }
    }
}

void initialize(void)
{
    SPIInit();
    SerInit();
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}

void interrupt ISR(void)
{
    if (PIR1bits.SSP1IF == 1)
    {
        SPIInt();
    }
}