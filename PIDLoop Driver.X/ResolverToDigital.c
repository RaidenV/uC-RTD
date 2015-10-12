#include "ResolverToDigital.h"

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

    while((LOT == 0) && (DOS == 0)) //Check to make sure the startup condition has been reached;
    {
	mRESET = 0; //Reset is initially off;
	NOP(); //Set a slight delay;
	mRESET = 1; //Toggle Resest ON;
	Delay1KTCYx(20); //Delay for 200,000 clock cycles (20 ms);
	SAMPLE = 0; //After the 20 ms delay, pulse the SAMPLE pin low;
        Delay1TCYx(1); //Delay 10 clock cycles;
	SAMPLE = 1;
     }	
    RDrtd = 1;
    
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
    unsigned int FullPosition;
    unsigned char HighPosition, LowPosition;
    
    if((LOT != 0) && (DOS != 0))
    {
        RDVEL = 1; //Set the pin to read position;
        SAMPLE = 0; //Toggle the sample low;
        for(x = 0; x < 8; x++) //Wait 800 ns for the data before grabbing the data;
	 NOP();
        RDrtd = 0; //Toggle the RD pin low;
        HighPosition = HighByte; //Get the data;
        LowPosition = LowByte;
        RDrtd = 1; //Set the RD pin high;
	SAMPLE = 1; //Set the SAMPLE pin high, this allows the chip to start processing the angle again;
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
    unsigned char x;
    
    if((LOT != 0) && (DOS != 0))
    {
        RDVEL = 0; //Clear the pin to read velocity;
        SAMPLE = 0; //Toggle the sample low;
        for(x = 0; x < 8; x++) //Wait 800 ns for the data before grabbing the data;
	 NOP();
        RDrtd = 0; //Toggle the RD pin low;
        HighVelocity = HighByte; //Get the data;
        LowVelocity = LowByte;
        RDrtd = 1; //Set the RD pin high;
	SAMPLE = 1; //Set the SAMPLE pin high, this allows the chip to start processing the angle again;
        FullVelocity = LowVelocity;
        FullVelocity = (HighVelocity & 0x0F) << 8;
        
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
