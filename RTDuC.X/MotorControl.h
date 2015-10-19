/* 
 * File:   MotorControl.h
 * Author: raidenv
 *
 * Created on August 22, 2015, 5:05 PM
 */

#ifndef MOTORCONTROL_H
#define	MOTORCONTROL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include <stdlib.h>

#define MOTORFAILLED PORTAbits.RA4
#define MotorFailPin PORT
#define DeadbandHigh 5
#define DeadbandLow -5

    extern unsigned char PIDEnableFlag;
    unsigned char MotorFailFlag;

    void MotorDriverInit(void);
    void KillMotors(void);
    void ImplementPIDMotion(int);
    void ImplementJSMotion(int);
    void INT0Int(void);


#ifdef	__cplusplus
}
#endif

#endif	/* MOTORCONTROL_H */

