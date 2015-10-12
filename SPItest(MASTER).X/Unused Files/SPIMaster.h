/* 
 * File:   SPIMaster.h
 * Author: raidenv
 *
 * Created on September 2, 2015, 8:13 PM
 */

#ifndef SPIMASTER_H
#define	SPIMASTER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <pic18f2620.h>
#include <spi.h>
#include <delays.h>

#define SlaveReady1 PORTAbits.RA1
#define SlaveReady2 PORTAbits.RA2
#define SlaveSelect1 PORTAbits.RA3
#define SlaveSelect2 PORTAbits.RA4

#define GetPOS 0x02

    extern unsigned char AZEL;
    //These Variables are here temporarily in order to drive the SPI routine;
    extern double Kp;
    extern double Ki;
    extern double Kd;
    extern double SetAngle;
    extern double CurrentAngle;
    extern double CurrentVelocity;
    extern unsigned char* DoublePtrSPI;
    extern unsigned char DoubleSPI[3];

    void SPIMInit(void);
    unsigned char MReceiveSPI(void);
    void MWriteSPI(unsigned char);
    void M_RW_Routine(unsigned char);

    void SPIDisassembleDouble(double);
    double SPIReassembleDouble(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SPIMASTER_H */

