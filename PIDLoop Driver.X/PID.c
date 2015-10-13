#include "PID.h"

unsigned char PIDEnableFlag;  
double Ki;
double Kp;
double Kd;
double SetAngle;
double CurrentAngle;
double err;
double prevErr;
double intErr;
double StartAngle;
int motorInput;

void PIDInit(void)
{
    /* This code should initialize the timer 0 oscillator*/
    err = 0;
    prevErr = 0;
    intErr = 0;
    
   // INTCONbits.TMR0IE = 1; //Enable the Timer 0 interrupt;  Scratch that, the Timer 0 Interrupt should only be enabled when the PID flag is raised;
    INTCON2bits.TMR0IP = 1; //Set the Timer 0 interrupt to high priority;
    T0CON = 0x84; //Enable Timer 0, 32:1 prescaler;
    TMR0H = timerHigh; //Offset the timer by 0xDB60 to allow for a 0.03 second timer loop;
    TMR0L = timerLow; //Calculation: 0xffff - ((0.03/(1/10e6))/32); hello world
    
}
void calculatePID(double angle, double setpoint)
{
    double derErr; //Generate derivative variable;
    if((PIDEnableFlag & 0x02) == 0x02) //Check if this is a new angle sent by the master;
    {
        StartAngle = angle; //If this is a new angle, note that this is the StartAngle, or angle upon which our error will be repeatedly calculated for consistency;
        err = 0; //Because we're starting from a newly commanded angle, all errors are essentially 0 until evaluated;
        prevErr = 0;
        intErr = 0;
        PIDEnableFlag = PIDEnableFlag & 0x01; //Since we have evaluated this as a new angle, we no longer need the new angle flag;
    }
    
    if(((StartAngle - setpoint) > 180) || ((StartAngle - setpoint) < -180)) 
    {
        if((StartAngle - setpoint) > 180) // such as in the case of current = 350 and set = 10;
        {
            if(angle > StartAngle) //While the current angle is larger than where it started (as it approaches 360);
                err = 360 - angle + setpoint; //The error is equal to 360 - the current angle + the set angle, in this case, 360 - 350 + 10, which gives us an error of 20.  This error is positive, which correlates to the direction which we'd like our motor to turn (big checkmark there!!!);           
            else
                err = setpoint - angle; //else, the error is a factor of the set angle - the current angle.  Let's take the case where the motor has now crossed 360 degrees and is around 1 degree.  We'd still like the error to be positive considering we're moving in the positive direction.  So we take our set angle, of 10 and subtract 1 degree from it, once again yielding a positive angle which is effectively the difference, check;
        }
        else if((StartAngle - setpoint) < -180) //such as in the case of current = 10 and set = 350;
        {
            if(angle < StartAngle)
                err = setpoint - 360 - angle; //The error is equal to the set angle - 360 - current, in this case, 350 - 360 - 10 = -20.  We are moving in a negative direction, which is reflected by the result;
            else
                err = setpoint - angle; //If the current angle becomes something like 359, then we have this formula: 350 - 359 = -9, which still yields a negative value.  Once again, yes!;
        }        
    }
    else if(((StartAngle - setpoint) <= 180) && ((StartAngle - setpoint) > -180)) //in all cases where the angle does not cross the 360/0 line;
    {
        if(((setpoint - StartAngle) > -180) && (setpoint - StartAngle < 0)) //Lets use the example of start current 270, set 100;
        {
            if(angle <= StartAngle) //If the current is less than the start current (<270), all is well;
                err = setpoint - angle; //Standard stuff.  270 - 100 = -170. Error = -170 degrees.  All good;
            else //But what happens in the extreme case where the set angle is 0 degrees, and the current angle is 40 degrees (you're approaching from the positive side);
                err = setpoint + 360 - angle; //In an ideal world, you'd have zero overshoot, so you'd never have to worry about this, however, we have to account for the fact that there is a strong likelihood that the motor will over shoot the 0 degree mark and drop into the high 300s.  if this the case, then your error becomes positive, indicating an overshoot.  the way to counteract that is by setting the error to set - 360 - current. In our case this yields, 0 + 360 - 350 (let's say the error is a whopping 10 degrees), yielding a positive error of 10;
        }
        else if(((setpoint - StartAngle) < 180) && (setpoint - StartAngle >= 0)) //If the start current - the set point is less than 180, lets say in the case where the start current is 100 and the set is 120;
        {
            if(angle <= StartAngle)
                err = setpoint - angle; //then, again, the error is simply 120 - 100;
            else
                err = setpoint - 360 - angle; //Again, what if the extreme case occurs where the user enters 360 degrees. In that case, you must once again account for the fact that your error is now going to be negative going, as you've overshot in the postive direction, so the error becomes set - 360 - current.  So let's take the example of Setpoint = 350, Current angle is 270.  Does the if statement include this? 350 - 270 = 80, which is less than 180.  Check. You overshoot to 10 degrees in the positive direction.  Your current angle, 10 degrees, is less than your starting angle, which was 270, which indicates that it has passed the 360 degree mark.  Now your error is 350 - 360 - 10 = -10.  This -10 indicates that you have to go in the opposite direction from where you started. 
        }
    }
    
    intErr = err + prevErr; //Calculate the integral error;
    derErr = err - prevErr; //Calculate the derivative error;
    
    motorInput = Kp * err + (Ki * intErr * loopTime) + (Kd * (derErr/loopTime));
    
    if(motorInput > 255) //Set the limits so that input is not greater than the possible motor input;
        motorInput = 255;
    else if (motorInput < -255)
        motorInput = -255;
    
    prevErr = err;
}

void TMR0Int(void)
{
    CurrentAngle = RTD2Angle(ReadRTDpos);
    calculatePID(CurrentAngle, SetAngle);
    ImplementPIDMotion(motorInput);
    TMR0H = timerHigh;
    TMR0L = timerLow;
    
    INTCONbits.TMR0IF = 0;
}