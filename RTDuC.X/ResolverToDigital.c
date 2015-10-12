#include "ResolverToDigital.h"
#include <pic18f8722.h>

void RTDInit(void)
{
    TRISD = 0xFF;   //Set the Low Byte Port as an input;
    TRISH = 0xFF;   //Set the High Byte Port as an input;
    TRISEbits.RE0 = 0;  //Set the SAMPLE pin as an output;
    TRISEbits.RE1 = 0;  //Set the RDVEL pin as an output;
    TRISEbits.RE2 = 0;  //Set the RD pin as an output;
    TRISJbits.RJ0 = 0;  //Set the RESET pin as an output;
    TRISEbits.RE5 = 1;  //Set the DIR pin as an input;
    TRISEbits.RE6 = 1;  //Set the DOS pin as an input;
    TRISEbits.RE7 = 1;  //Set the LOT pin as an input;
    RDrtd = 0;
    
}

/* This function reads the RTD's position and returns that value;
 * I think the spec sheet is wrong.  It quotes events against a clock cycle
 * which does not seem to match the actual timing
 * CLOCK(fclk) = 8.192 MHz
 * Between SAMPLE low and RD: 6 * (1/fclk) + 20; = 20 ns according ot the datasheet.  
 * I think they're full of shit, but I'll write it this way to start.  
 * I think that the datasheet should be 6 * ((1/fclk) + 20);
 */
unsigned int ReadRTDpos(void)
{
    unsigned int FullPosition;
    unsigned char HighPosition, LowPosition;
    
    if((LOT != 0) && (DOS != 0))
    {
        RDVEL = 1;
        SAMPLE = 0;
        RDrtd = 0;
        HighPosition = HighByte;
        LowPosition = LowByte;
        SAMPLE = 1;
        FullPosition = LowPosition;
        FullPosition = (HighPosition & 0x0F) << 8; 
        
        return FullPosition;
    }
    
    else
        return 0xFF;    //If there is loss of tracking or degradation of signal, send back a unique int;
}

unsigned int ReadRTDvel(void)
{
    unsigned int FullVelocity;
    unsigned char HighVelocity, LowVelocity;
    
    if((LOT != 0) && (DOS != 0))
    {
        RDVEL = 0;
        SAMPLE = 0;
        RDrtd = 0;
        HighVelocity = HighByte;
        LowVelocity = LowByte;
        SAMPLE = 1;
        FullVelocity = LowVelocity;
        FullVelocity = (HighVelocity & 0x07) << 8; 
        
        return FullVelocity;
    }
    
    else
        return 0xFF;    //If there is loss of tracking or degradation of signal, send back a unique int;
}

/* RTD2Angle
 * This function converts the value returned by the ReadRTDpos to an angle between
 * 0 and 360 degrees; 
 */
double RTD2Angle(unsigned int RTDAngle)
{
    return ANGLERATIO * RTDAngle;
}
double RTD2Velocity(unsigned int);  //The contents of this function need to be decided once the radial velocity is considered;

unsigned char RTDdirection(void)
{
    return DIR;
}
