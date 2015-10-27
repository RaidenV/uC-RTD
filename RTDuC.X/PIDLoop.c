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

        if (PIDEnableFlag == 1 && TMR0Flag == 1) //This is the option which will run more frequently, therefore it should come first to avoid an instruction cycle of testing the PIDEnableFlag for the less likely value of '3';
        {
            INTCONbits.GIE = 0; //Disable interrupts while the PID loop runs;
            CurrentAngle = RTD2Angle(ReadRTDpos());
            calculatePID(CurrentAngle, SetAngle);
            ImplementPIDMotion(motorInput);
            TMR0Flag = 0;
            INTCONbits.GIE = 1; //Enable interrupts;

            SerTxStr("Current Angle: ");
            breakDouble(CurrentAngle);
        }

        else if (PIDEnableFlag == 3) //Tests if the bit has been set by the StrippedKey = 0x01 in the KeyValue code;
        {
            INTCONbits.GIE = 0; //Disable interrupts while the PID loop runs;
            TMR0H = timerHigh;
            TMR0L = timerLow;
            CurrentAngle = RTD2Angle(ReadRTDpos());
            calculatePID(CurrentAngle, SetAngle);
            ImplementPIDMotion(motorInput);
            INTCONbits.GIE = 1;
            INTCONbits.TMR0IE = 1; //If so, enable the PID loop;
            T0CONbits.TMR0ON = 1;
        }
    }
}

void initialize(void)
{
    RTDInit();  //Initialize all modules;
    PIDInit();
    MotorDriverInit();
    SerInit();
    lcdInit();

    INTCONbits.GIE = 1; //Enable General Interrupts;
    INTCONbits.PEIE = 1; //Enable Peripheral Interrupts;
    RCONbits.IPEN = 1; //Enable Interrupt Priorities;
}

void interrupt high_priority hISR(void)
{
    if ((INTCONbits.TMR0IF == 1) && (INTCONbits.TMR0IE == 1) && ((PIDEnableFlag == 1) || (PIDEnableFlag == 3))) //If the interrupt flag is raised and the interrupt is enabled;
        TMR0Int(); //Run the PID loop;
}

void interrupt low_priority lISR(void)
{
    if (PIR1bits.RC1IF == 1) //If the computer has attempted to talk to the unit;
    {
        KillMotors();
        RCInt(); //Enter the Receive routine;
        StartMotors();
    }
        
}