/* 
 * File:   KeyValue.h
 * Author: raidenv
 *
 * Created on August 23, 2015, 2:47 PM
 */

#ifndef KEYVALUE_H
#define	KEYVALUE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "SerComm.h"

#define DELIMITER '='
#define KEYLENGTH 5
#define VALUELENGTH 10
#define LENGTH 30
#define carriageReturn 0x0D
#define newLine 0x0A

    //These Variables are here temporarily in order to drive the SPI routine;
    extern double Kp;
    extern double Ki;
    extern double Kd;
    extern double SetAngle;
    extern double CurrentAngle;
    extern double CurrentVelocity;

    extern double StrippedValue;
    extern unsigned char StrippedKey;
    extern unsigned char AZEL; //This should either be 0 for Azimuth or 1 for Elevation.  Use this variable to determine which SPI CS to use in communication;
    extern unsigned char key[5];
    extern unsigned char value[10];
    extern unsigned char received[30];
    extern unsigned char RCFlag;
    extern unsigned char RECFlag;
    extern unsigned char AZFlowFlag;
    extern unsigned char ELFlowFlag;

    void RCInt(void);
    void keyValue(unsigned char*, unsigned short);
    void HelpString(void);


#ifdef	__cplusplus
}
#endif

#endif	/* KEYVALUE_H */

