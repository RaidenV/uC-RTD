/* 
 * File:   SerComm.h
 * Author: raidenv
 *
 * Created on August 1, 2015, 3:43 PM
 */

#ifndef SERCOMM_H
#define	SERCOMM_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>

#define carriageReturn 0x0D
#define newLine 0x0A

    void SerInit(void);
    void SerTx(unsigned char);
    void SerTxStr(unsigned char*);
    void SerNL(void);
    unsigned char SerRx(void);
    void SerRxStr(unsigned char*);
    void breakDouble(double);
    void SendLode(double*);

#ifdef	__cplusplus
}
#endif

#endif	/* SERCOMM_H */

