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

    extern unsigned char RCflag;
    extern unsigned char ReceivedChar;
    extern unsigned char* DoublePtr;
    extern unsigned char DoubleSPIM[4];

    void SPIInitM(void);
    unsigned char MReceiveSPI(unsigned char);
    double SPIReassembleDouble(void);
    void MSendSPI(unsigned char, unsigned char);
    void MReceiveStrSPI(unsigned char*, unsigned char);
    unsigned char checksum(void);

    void SPIDisassembleDouble(double dub);
    unsigned char MGenerateChecksum(void);


#ifdef	__cplusplus
}
#endif

#endif	/* SPIMASTER_H */

