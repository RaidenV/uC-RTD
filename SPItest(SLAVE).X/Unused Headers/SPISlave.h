/* 
 * File:   SPISlave.h
 * Author: raidenv
 *
 * Created on September 4, 2015, 8:07 PM
 */

#ifndef SPISLAVE_H
#define	SPISLAVE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <pic18f8722.h>
#include <spi.h>
#include <delays.h>
#define SlaveReady PORTCbits.RC6
#define SPILED PORTAbits.RA2

    extern double Kp;
    extern double Ki;
    extern double Kd;
    extern double SetAngle;
    extern double CurrentAngle;
    extern double CurrentVelocity;
    extern unsigned char* DoublePtrSPI;
    extern unsigned char DoubleSPIS[7];
    extern unsigned char PIDEnableFlag;

    void SPISInit(void);
    void SendSPI1(unsigned char);
    void SendStrSPI1(unsigned char*);
    unsigned char ReceiveSPI1Int(void);
    unsigned char ReceiveSPI1(void);
    void ReceiveStrSPI1(unsigned char*, unsigned char);
    void S_RW_Routine(void);
    unsigned char DataReadySPI1(void);

    void SPIDisassembleDouble(double);
    double SPIReassembleDouble(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SPISLAVE_H */

