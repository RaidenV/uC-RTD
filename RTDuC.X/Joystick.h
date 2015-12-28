/* 
 * File:   Joystick.h
 * Author: raidenv
 *
 * Created on August 22, 2015, 12:37 PM
 */

#ifndef JOYSTICK_H
#define	JOYSTICK_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include <xc.h>
#include <stdlib.h>
#include "ResolverToDigital.h"

#define JOYSTICKLED PORTAbits.RA2
#define JOYSTICKDETECT PORTBbits.RB1
#define JSOFFSET 512

    extern unsigned char PIDEnableFlag; //Global variable used to disable the PID loop when the Joystick is enabled;
    extern unsigned char JSEnableFlag; //Global variable to enable the Joystick motion detection algorithm;
    extern double CurrentAngle;
    extern int DeadbandLow;
    extern int DeadbandHigh;

    void JoystickInit(void);
    void DetectJoystick(void);
    int DetectMovement(void);


#ifdef	__cplusplus
}
#endif

#endif	/* JOYSTICK_H */

