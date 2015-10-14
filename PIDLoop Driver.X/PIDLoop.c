#include <xc.h>
#include "PID.h"
#include "ResolverToDigital.h"
#include "MotorControl.h"
#include "KeyValue.h"
#include "SerComm.h"
#include "LCD.h"

#pragma config OSC = HSPLL
#pragma config WDT = OFF
#pragma config FCMEN = OFF

void initialize(void);
void interrupt ISR(void);

void main(void)
{
    initialize();

    while (1)
    {
        if ((PIDEnableFlag & 0x02) == 0x02) //Tests if the bit has been set by the StrippedKey = 0x01 in the KeyValue code;
        {
            TMR0H = timerHigh;
            TMR0L = timerLow;
            INTCONbits.TMR0IE = 1; //If so, enable the PID loop;
        }
        else if((PIDEnableFlag & 0x01) == 0x01)
            INTCONbits.TMR0IE = 1;
        else
            PIDEnableFlag = 0;
        
        LCDBreakDouble(CurrentAngle);
    }
}

void initialize(void)
{
    PIDInit(); //Initialize all modules;
    MotorDriverInit();
    RTDInit();
    SerInit();

    INTCONbits.GIE = 1; //Enable General Interrupts;
    INTCONbits.PEIE = 1; //Enable Peripheral Interrupts;
    RCONbits.IPEN = 1; //Enable Interrupt Priorities;
}

void interrupt ISR(void)
{
    if (PIR1bits.RC1IF == 1) //If the computer has attempted to talk to the unit;
        RCInt(); //Enter the Receive routine;
    else if ((INTCONbits.TMR0IF == 1) && (INTCONbits.TMR0IE == 1)) //If the interrupt flag is raised and the interrupt is enabled;
        TMR0Int(); //Run the PID loop;
}