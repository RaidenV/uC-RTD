/* 
 * File:   EEPROM.h
 * Author: raidenv
 *
 * Created on August 23, 2015, 5:32 PM
 */

#ifndef EEPROM_H
#define	EEPROM_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <xc.h>
#include <EEP.h>
    
/* The following definitions allow more flexibility in the main body of code in
 * the event, let's say, that a double does not happen to be 8 bytes (there's
 * no guarantee of the size of the double);
 */
#define LASTCOMPOSloc 0x00
#define KPPARAMloc 0x03
#define KIPARAMloc 0x06
#define KDPARAMloc 0x09
#define PORTAloc 0x0C
#define PORTBloc 0x0D
#define PORTCloc 0x0E
#define PIDENABLEloc 0x15
#define SAVEDloc 0x16 //This lets us know whether or not the unit has settings saved;
    
    extern unsigned char DDouble[3];
    extern unsigned char* DoublePtr;
    extern double Kp;
    extern double Ki;
    extern double Kd;
    extern double SetAngle;
    extern double CurrentAngle;
    extern double CurrentVelocity;
    extern unsigned char PIDEnableFlag;

    void EEPROMInit(void);
    void EEBootUp(void);
    void EEDisassembleDouble(double);
    double EEReassembleDouble(void);
    void EEWriteDouble(unsigned char, double);
    double EEReadDouble(unsigned char);

    void EEWriteChar(unsigned char, unsigned char);
    char EEReadChar(unsigned char);

    void SaveAll(void);

#ifdef	__cplusplus
}
#endif

#endif	/* EEPROM_H */

