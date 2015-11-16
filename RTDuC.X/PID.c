#include "PID.h"
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
    error = 0;
    prevErr = 0;
    intErr = 0;
    
    T0CON = 0x04; //Timer 0 32:1 prescaler;
    TMR0H = timerHigh; //Offset the timer by 0xDB60 to allow for a 0.03 second timer loop;
    TMR0L = timerLow; //Calculation: 0xffff - ((0.03/(1/10e6))/32);
}

/* calculatePID
 * Calculates the output of the PID equation based on the Kp, Ki, and Kd entered
 * by the user;
 */
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
    if (error > 180) //If the error is greater than 180...;
        error -= 360; //Then the shortest angle (and thus the error) is the current error -360;
    else if (error < -180) //Else if the error is less than negative 180 (considering the setpoint of 10 and the angle of 270);
        error += 360; //Then add 360 to that (-260 + 360 = 100), 100 + 270 (passing over 360 degrees) is indeed 10;

    derErr = error - prevErr; //Calculate the derivative error;
    intErr += error; //Calculate the integral error;
    if (intErr > 2000)
        intErr = 2000; //Put a cap on the integral error.  Allowing it to run away too far creates a loss of control (this is known as Integral Windup and is a prime source of overshooting/undershooting);

    motorInput = Kp * error + (Ki * intErr * loopTime) + (Kd * (derErr / loopTime)); //Standard PID equation;

    prevErr = error;
}

/* TMR0Int
 * Handles the timer0 interrupt;
 */
void TMR0Int(void)
{
    TMR0H = timerHigh; //Set the timer count;
    TMR0L = timerLow;
    TMR0Flag = 1; //Raise the timer flag;
    INTCONbits.TMR0IF = 0; //Lower the interrupt flag;
}