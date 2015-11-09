#include "ResolverToDigital.h"

void RTDInit(void)
{
    TRISD = 0xFF; //Set the Low Byte Port as an input;
    TRISH = 0xFF; //Set the High Byte Port as an input;
    TRISEbits.RE0 = 0; //Set the SAMPLE pin as an output;
    TRISEbits.RE1 = 0; //Set the RDVEL pin as an output;
    TRISEbits.RE2 = 0; //Set the RD pin as an output;
    TRISJbits.RJ0 = 0; //Set the RESET pin as an output;
    TRISEbits.RE5 = 1; //Set the DIR pin as an input;
    TRISEbits.RE6 = 1; //Set the DOS pin as an input;
    TRISEbits.RE7 = 1; //Set the LOT pin as an input;
    
    mRESET = 0; //Reset is initially off;
    Delay10TCYx(10);
    mRESET = 1; //Toggle Resest ON;
    Delay1KTCYx(20); //Delay for 200,000 clock cycles (20 ms);
    SAMPLE = 0; //After the 20 ms delay, pulse the SAMPLE pin low;
    Delay1TCYx(1); //Delay 10 clock cycles;
    SAMPLE = 1;
    RDrtd = 0;

}

/* ReadRTDpos
 * This function reads the RTD's position and returns that value;
 * I spoke with Analog Devices, discovered that the spec sheet out to be, considering T2,
 * 6 * (1/Fosc) + 20ns, where the result is in seconds;  This gives us a delay of 300 ns
 * for T1, which is a doubtable characteristic as I don't think this time span is notable,
 * and roughly 800 ns for T2;
 */
unsigned int ReadRTDpos(void)
{
    unsigned char x;
    unsigned int FullPosition, helloworld;
    unsigned char HighPosition, LowPosition;

        INTCONbits.GIE = 0; //Turn off interrupts;
        RDVEL = 1; //Set the pin to read position;
        SAMPLE = 0; //Toggle the sample low;
        for (x = 0; x < 8; x++) //Wait 800 ns for the data before grabbing the data;
            NOP();
        RDrtd = 1;
        RDrtd = 0; //Toggle the RD pin low;
        Delay10TCYx(1);
        HighPosition = HighByte; //Get the data;
        LowPosition = LowByte;
        RDrtd = 1; //Set the RD pin high;
        SAMPLE = 1; //Set the SAMPLE pin high, this allows the chip to start processing the angle again;
        FullPosition = LowPosition;
        FullPosition = FullPosition | ((HighPosition & 0x0F) << 8);
        INTCONbits.GIE = 1; //Turn on interrupts;
        return FullPosition;
}

/* ReadRTDvel
 * Reads the velocity value output by the resolver to digital converter;
 * This section of code isn't used at the moment and is simply included in the
 * event that some sort of expansion is required.  As far as I'm concerned, the
 * accompanying C++ utility can use the Position to calculate velocity well
 * enough that this may not ever be necessary;
 */
unsigned int ReadRTDvel(void)
{
    unsigned int FullVelocity;
    unsigned char HighVelocity, LowVelocity;
    unsigned char x;

        INTCONbits.GIE = 0; //Turn off interrupts;
        RDVEL = 1; //Set the pin to read velocity;
        SAMPLE = 0; //Toggle the sample low;
        for (x = 0; x < 8; x++) //Wait 800 ns for the data before grabbing the data;
            NOP();
        RDrtd = 1;
        RDrtd = 0; //Toggle the RD pin low;
        Delay10TCYx(1);
        HighVelocity = HighByte; //Get the data;
        LowVelocity = LowByte;
        RDrtd = 1; //Set the RD pin high;
        SAMPLE = 1; //Set the SAMPLE pin high, this allows the chip to start processing the angle again;
        FullVelocity = LowVelocity;
        FullVelocity = FullVelocity | ((HighVelocity & 0x0F) << 8);
        INTCONbits.GIE = 1; //Turn on interrupts;
        return FullVelocity;
}

/* RTD2Angle
 * This function converts the value returned by the ReadRTDpos to an angle 
 * between 0 and 360 degrees; 
 */
double RTD2Angle(unsigned int RTDAngle)
{
    return ANGLERATIO * RTDAngle;
}
double RTD2Velocity(unsigned int); //The contents of this function need to be decided once the radial velocity is considered;

/* RTDdirection
 * I'm not sure if this ever needs to be used, but here it is;
 */
unsigned char RTDdirection(void)
{
    return DIR;
}
