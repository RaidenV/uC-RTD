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
    
    //The following portions which have been commented out are due to the inability of resolving the issue with the LOT and DOS signals.
    //A PMIC Supervisor chip may be used to overcome this design flaw, however we have found the RTD to be stable during operation
    //without the use of these system checks.  In the final version, this must be encorporated.
    //
    //    while((LOT == 0) && (DOS == 0)) //Check to make sure the startup condition has been reached;
    //    {
    mRESET = 0; //Reset is initially off;
    Delay10TCYx(10);
    mRESET = 1; //Toggle Resest ON;
    Delay1KTCYx(20); //Delay for 200,000 clock cycles (20 ms);
    SAMPLE = 0; //After the 20 ms delay, pulse the SAMPLE pin low;
    Delay1TCYx(1); //Delay 10 clock cycles;
    SAMPLE = 1;
    //     }	
    RDrtd = 0;

}

/* This function reads the RTD's position and returns that value;
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

    //The braces around the following statements are to, once again, qualify the DOS and LOT bits presented
    //by the RTD chip.  At this point we have a stable reading from the chip and therefore do not require them
    //however, as the chip appears to utilize these functions, they must be implemented in the final design;
    {
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
        return FullPosition;
    }
}

unsigned int ReadRTDvel(void)
{
    unsigned int FullVelocity;
    unsigned char HighVelocity, LowVelocity;
    unsigned char x;

//    if ((LOT != 0) && (DOS != 0))
//    {
        RDVEL = 0; //Clear the pin to read velocity;
        SAMPLE = 0; //Toggle the sample low;
        for (x = 0; x < 8; x++) //Wait 800 ns for the data before grabbing the data;
            NOP();
        RDrtd = 0; //Toggle the RD pin low;
        HighVelocity = HighByte; //Get the data;
        LowVelocity = LowByte;
        RDrtd = 1; //Set the RD pin high;
        SAMPLE = 1; //Set the SAMPLE pin high, this allows the chip to start processing the angle again;
        FullVelocity = LowVelocity;
        FullVelocity = (HighVelocity & 0x0F) << 8;

        return FullVelocity;
//    }

//    else
//        return 0xFF; //If there is loss of tracking or degradation of signal, send back a unique int;
}

/* RTD2Angle
 * This function converts the value returned by the ReadRTDpos to an angle between
 * 0 and 360 degrees; 
 */
double RTD2Angle(unsigned int RTDAngle)
{
    return ANGLERATIO * RTDAngle;
}
double RTD2Velocity(unsigned int); //The contents of this function need to be decided once the radial velocity is considered;

unsigned char RTDdirection(void)
{
    return DIR;
}
