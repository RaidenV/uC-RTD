/* 
 * File:   ResolverToDigital.h
 * Author: raidenv
 *
 * Created on August 18, 2015, 8:53 PM
 */

#ifndef RESOLVERTODIGITAL_H
#define	RESOLVERTODIGITAL_H

#ifdef	__cplusplus
extern "C" {
#endif

#define LowByte PORTD		//Takes the low byte of the RTD;
#define HighByte PORTH		//Takes the high byte of the RTD;
#define SAMPLE PORTEbits.RE0	
#define RDVEL PORTEbits.RE1
#define RDrtd PORTEbits.RE2
#define DIR PORTEbits.RE5
#define DOS PORTEbits.RE6
#define LOT PORTEbits.RE7
#define RESETrtd PORTJbits.RJ0
#define ANGLERATIO 0.087890625  //This is 360 degrees divided by the 12 bits (4096) and represents a constant of the angle in proportion to the received data;

    void RTDInit(void);
    unsigned int ReadRTDpos(void);
    unsigned int ReadRTDvel(void);
    double RTD2Angle(unsigned int);
    double RTD2Velocity(unsigned int);
    unsigned char RTDdirection(void);

#ifdef	__cplusplus
}
#endif

#endif	/* RESOLVERTODIGITAL_H */

