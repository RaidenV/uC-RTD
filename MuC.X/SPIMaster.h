/* 
 * File:   SPIMaster.h
 * Author: raidenv
 *
 * Created on October 11, 2015, 7:28 PM
 */

#ifndef SPIMASTER_H
#define	SPIMASTER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include <spi.h>
#include <stdlib.h>
#include "KeyValue.h"
#include "SerComm.h"
#include <delays.h>

#define SlaveReady1 PORTBbits.RB1
#define SlaveReady2 PORTBbits.RB2
#define SlaveSelect1 PORTBbits.RB3
#define SlaveSelect2 PORTBbits.RB4

    extern double Kp;
    extern double Ki;
    extern double Kd;
    extern double CurrentAngle;
    extern double CurrentVelocity;
    extern double AZlast;
    extern double ELlast;
    extern unsigned char RCFlag;
    extern unsigned char ReceivedChar;
    extern unsigned char* DoublePtr;
    extern unsigned char DoubleSPIM[4];
    extern const unsigned int ReceiveLodeSize;
    extern const unsigned int DataLodeSize;
    extern unsigned char ReceiveLode[1809];
    extern double DataLode[603];

    void SPIInitM(void);
    unsigned char MReceiveSPI(unsigned char);
    void MReceiveStrSPI(unsigned char);
    void MSendSPI(unsigned char, unsigned char);
    double SPIReassembleDouble(void);
    void SPIDisassembleDouble(double);
    void SPIReassembleLode(void);
    void MReceiveLodeSPI(unsigned char);

    void MSPIRoutine(unsigned char, unsigned char, double);
    void MSPIRecRoutine(unsigned char, unsigned char);

    void RestartSPI(void);
    void SelectSlaveStart(unsigned char);
    void SelectSlaveEnd(unsigned char);
    unsigned char SlaveQuery(unsigned char);

    unsigned char checksum(void);
    unsigned char MGenerateChecksum(void);


#ifdef	__cplusplus
}
#endif

#endif	/* SPIMASTER_H */

