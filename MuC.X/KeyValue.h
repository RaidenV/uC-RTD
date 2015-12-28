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

#define DELIMITER '=' //Defines the delimiting character, posted here in single-quotes for clarity;
#define KEYLENGTH 5 //Arbitrary maximum key length, posted here so that it can change in the future;
#define VALUELENGTH 10 //Arbitrary maximum value length based on the amount of precision that the system is capable of (10 digits would far exceed it);
#define LENGTH 30 //Arbitrary length of the received string, with plenty of buffer room for spaces and whatnot;
#define carriageReturn 0x0D 
#define newLine 0x0A

    extern double Kp;
    extern double Ki;
    extern double Kd;
    extern double SetAngle;
    extern double CurrentAngle;
    extern double CurrentVelocity;

    extern double StrippedValue;
    extern unsigned char StrippedKey;
    extern unsigned char AZEL; //This should either be 0 for Azimuth or 1 for Elevation.  Use this variable to determine which SPI CS to use in communication;
    extern unsigned char key[KEYLENGTH];
    extern unsigned char value[VALUELENGTH];
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
