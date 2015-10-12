/* 
 * File:   SPI2.h
 * Author: raidenv
 *
 * Created on September 1, 2015, 4:03 PM
 */

#ifndef SPI2_H
#define	SPI2_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <pic18f8722.h>
    
    unsigned char SendSPI2(unsigned char);
    void SendStrSPI2(unsigned char*);
    unsigned char ReceiveSPI2(void);
    void ReceiveStrSPI2(unsigned char*, unsigned char);  
    unsigned char DataReadySPI2(void);


#ifdef	__cplusplus
}
#endif

#endif	/* SPI2_H */

