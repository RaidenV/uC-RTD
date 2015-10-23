#include <xc.h>
#include "PID.h"
#include "ResolverToDigital.h"
#include "MotorControl.h"
#include "KeyValue.h"
#include "SerComm.h"

#pragma config OSC = HSPLL
#pragma config WDT = OFF
#pragma config FCMEN = OFF

void initialize(void);
void interrupt high_priority hISR(void);
void interrupt low_priority lISR(void);

void main(void)
{
    initialize();

    while (1)
    {
        if (PIDEnableFlag == 3) //Tests if the bit has been set by the StrippedKey = 0x01 in the KeyValue code;
        {
            TMR0H = timerHigh;
            TMR0L = timerLow;
            INTCONbits.TMR0IE = 1; //If so, enable the PID loop;
            T0CONbits.TMR0ON = 1;
        }
        //        else if (PIDEnableFlag == 0)
        //            PIDEnableFlag = 0;
    }
}

void initialize(void)
{
    RTDInit();
    PIDInit(); //Initialize all modules;
    MotorDriverInit();
    SerInit();
    lcdInit();

    INTCONbits.GIE = 1; //Enable General Interrupts;
    INTCONbits.PEIE = 1; //Enable Peripheral Interrupts;
    RCONbits.IPEN = 1; //Enable Interrupt Priorities;
}

void interrupt high_priority hISR(void)
{
    if ((INTCONbits.TMR0IF == 1) && (INTCONbits.TMR0IE == 1)) //If the interrupt flag is raised and the interrupt is enabled;
        TMR0Int(); //Run the PID loop;
}

void interrupt low_priority lISR(void)
{
    if (PIR1bits.RC1IF == 1) //If the computer has attempted to talk to the unit;
        RCInt(); //Enter the Receive routine;
}