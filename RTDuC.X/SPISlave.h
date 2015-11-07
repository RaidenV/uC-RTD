/* 
 * File:   SPISlave.h
 * Author: raidenv
 *
 * Created on October 10, 2015, 5:13 PM
 */

#ifndef SPISLAVE_H
#define	SPISLAVE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include <spi.h>
    
#define SlaveReady PORTCbits.RC6
#define SlaveSelect PORTFbits.RF7
    
    extern unsigned char SPIflag;
    extern unsigned char Command;
    extern unsigned char dummy_byte;
    extern unsigned char* DoublePtr;
    extern unsigned char DoubleSPIS[4];
    extern unsigned char PIDEnableFlag;
    extern unsigned char RECFlag;
    extern double SetAngle;
    extern double CurrentAngle;
    extern double CurrentVelocity;
    extern double Kp;
    extern double Ki;
    extern double Kd;

    void SPIInit(void);
    void SPIInt(void);
    void SendSPI1(unsigned char);
    void SPIDisassembleDouble(double);
    void SPIDisassembleLode(double*, unsigned char*);
    unsigned char GenerateChecksum(void);
    unsigned char ReceiveSPI1(void);
    unsigned char checksum(void);
    double SPIReassembleDouble(void);


#ifdef	__cplusplus
}
#endif

#endif	/* SPISLAVE_H */

