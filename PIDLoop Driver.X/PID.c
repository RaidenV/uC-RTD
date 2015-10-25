#include "PID.h"
#include "LCD.h"
#include "ResolverToDigital.h"
#include "MotorControl.h"

unsigned char PIDEnableFlag;
unsigned char TMR0Flag = 0;
double Ki;
double Kp;
double Kd;
double SetAngle;
double CurrentAngle;
double error;
double prevErr;
double intErr;
int motorInput;
double loopTime = 0.03;

void PIDInit(void)
{
    /* This code should initialize the timer 0 oscillator*/
    error = 0;
    prevErr = 0;
    intErr = 0;

    // INTCONbits.TMR0IE = 1; //Enable the Timer 0 interrupt;  Scratch that, the Timer 0 Interrupt should only be enabled when the PID flag is raised;
    //INTCON2bits.TMR0IP = 1; //Set the Timer 0 interrupt to high priority;
    T0CON = 0x04; //Enable Timer 0, 32:1 prescaler;
    TMR0H = timerHigh; //Offset the timer by 0xDB60 to allow for a 0.03 second timer loop;
    TMR0L = timerLow; //Calculation: 0xffff - ((0.03/(1/10e6))/32);
    INTCON2bits.TMR0IP = 1; //High Priority;

}

void calculatePID(double angle, double setpoint)
{
    double derErr; //Generate derivative variable;
    if (PIDEnableFlag == 3) //Check if this is a new angle sent by the master;
    {
        error = 0; //Because we're starting from a newly commanded angle, all errors are essentially 0 until evaluated;
        prevErr = 0;
        intErr = 0;
        PIDEnableFlag = 1; //Since we have evaluated this as a new angle, we no longer need the new angle flag;
    }

    error = setpoint - angle;
    if (error > 180)
        error -= 360;
    else if (error < -180)
        error += 360;

    derErr = error - prevErr; //Calculate the derivative error;
    intErr += error; //Calculate the integral error;
    if (intErr > 2000)
        intErr = 2000; //Put a cap on the integral error.  Allowing it to run away too far creates a loss of control;

    motorInput = Kp * error + (Ki * intErr * loopTime) + (Kd * (derErr / loopTime));

    prevErr = error;
}

void TMR0Int(void)
{
    TMR0H = timerHigh;
    TMR0L = timerLow;
    // LCDBreakDouble(CurrentAngle);
    /*I suspect that the LCD Routine is bogging down the processor.  Time to eliminate it;*/
    TMR0Flag = 1;
    INTCONbits.TMR0IF = 0;
}