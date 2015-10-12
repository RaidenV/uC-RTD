/* 
 * File:   PID.h
 * Author: raidenv
 *
 * Created on September 30, 2015, 6:56 PM
 */

#ifndef PID_H
#define	PID_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include <stdlib.h>
#include "ResolverToDigital.h"
#include "MotorControl.h"

#define loopTime 0.03
#define timerHigh 0xDB
#define timerLow 0x60

    extern unsigned char PIDEnableFlag;    
    extern double Ki;
    extern double Kp;
    extern double Kd;
    extern double SetAngle;
    extern double CurrentAngle;
    extern double err;
    extern double prevErr;
    extern double intErr;
    extern double StartAngle;
    extern int motorInput;

    void PIDInit(void);
    void calculatePID(double, double);
    void TMR0Int(void);

#ifdef	__cplusplus
}
#endif

#endif	/* PID_H */

