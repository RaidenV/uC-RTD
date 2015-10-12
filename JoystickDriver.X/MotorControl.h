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

#define DeadbandHigh 100
#define DeadbandLow -100

    void MotorDriverInit(void);
    void KillMotors(void);
    void ImplementPIDMotion(int);
    void ImplementJSMotion(int);


#ifdef	__cplusplus
}
#endif

#endif	/* MOTORCONTROL_H */

