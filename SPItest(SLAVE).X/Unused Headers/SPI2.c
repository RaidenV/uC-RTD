#include "SPI2.h"

unsigned char SendSPI2(unsigned char data)
{
    unsigned char tempChar;
    tempChar = SSP2BUF;
    SSP2BUF = data;
}

void SendStrSPI2(unsigned char* chPtr)
{
    while(chPtr++)
        SendSPI2(*chPtr);
}

unsigned char ReceiveSPI2(void)
{  
    while(SSP2STATbits.BF == 0);
    return SSP2BUF;
}

void ReceiveStrSPI2(unsigned char* chPtr, unsigned char length)
{
    unsigned char z;
    for(z = 0; z < length; z++)
    {
        chPtr[z] = ReceiveSPI2();
    }
}

unsigned char DataRdySPI2(void)
{
    if(SSP2STATbits.BF == 1)
        return 1;
    else
        return 0;
}
