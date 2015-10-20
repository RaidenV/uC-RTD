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
#define DeadbandHigh 5
#define DeadbandLow -5
#define JSOFFSET 512

    extern unsigned char PIDEnableFlag;
    extern unsigned char JSEnableFlag;

    void JoystickInit(void);
    void DetectJoystick(void);
    int DetectMovement(void);


#ifdef	__cplusplus
}
#endif

#endif	/* JOYSTICK_H */

