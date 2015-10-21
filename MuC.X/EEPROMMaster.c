#include "EEPROMMaster.h"

unsigned char DDouble[3];
unsigned char* DoublePtr;
double Kp;
double Ki;
double Kd;
double SetAngle;
double CurrentAngle;
double CurrentVelocity;
unsigned char PIDEnableFlag;

void EEPROMInit(void)
{
    EECON1bits.EEPGD = 0; //Set the mode to EEPROM;
    EECON1bits.CFGS = 0; //Set to access EEPROM memory, instead of Configuration registers;
    EECON1bits.WREN = 1; //Enable writing to the EEPROM;
    
    EEBootUp(); //Check to see if the EEPROM contains previous values; if so, load those values;

    HLVDCON = 0x3D; //Enable the High/Low Voltage Detect Control, Event occurs when voltage falls below trip point, trip point is 13/14 in terms of internal settings;
    PIE2bits.HLVDIE = 1; //Enable the HLVD interrupt;
    IPR2bits.HLVDIP = 1; //Set the HLVD to high priority;
}

#pragma interrupt_level 1 //According to the XC8 user's guide, this guarantees that there is no duplication of the interrupt in the event that all interrupts are turned off, which is the case with this function and those hereafter;
void EEBootUp(void) //This function restores variables that were lost;
{
    if (EEReadChar(SAVEDloc)) //If the SAVEDloc indicates that this is not the first time that the chip has booted or data has been entered, proceed by loading the data;
    {
        SetAngle = EEReadDouble(LASTCOMPOSloc);
        Kp = EEReadDouble(KPPARAMloc);
        Ki = EEReadDouble(KIPARAMloc);
        Kd = EEReadDouble(KDPARAMloc);
        PIDEnableFlag = EEReadChar(PIDENABLEloc);
        if((PIDEnableFlag | 0x01) == 0x01) //If the PID loop is enabled;
        {
            PIDEnableFlag = 0x03; //Let the PID Loop handle the set angle as if it is a newly entered angle;
        }
    }
}

/* DisassembleDouble
 * This takes a double variable and breaks it into 8 parts.  Note that this can 
 * be used for SPI communication as well;
 */
#pragma interrupt_level 1
void EEDisassembleDouble(double dub)
{
    DoublePtr = (unsigned char*) &dub; //This sets the pointer to the location of the first byte of the double;
    DDouble[0] = DoublePtr[0]; //The following lines extract the double byte-by-byte into the DDouble variable;
    DDouble[1] = DoublePtr[1]; //This way, whenever the double is operated upon, the referenced double is not altered;
    DDouble[2] = DoublePtr[2];
}

/* ReassembleDouble
 * This takes a double which was broken down into 8 parts and reassembles it 
 * into a double variable.  Note that this can be used for SPI communication as 
 * well;
 */
#pragma interrupt_level 1
double EEReassembleDouble(void)
{
    double dub;
    DoublePtr = (unsigned char*) &dub; //This sets the pointer to the location of the first byte of the double;
    DoublePtr[0] = DDouble[0]; //The following lines extract the double byte-by-byte into the DDouble variable;
    DoublePtr[1] = DDouble[1]; //This way, whenever the double is operated upon, the referenced double is not altered;
    DoublePtr[2] = DDouble[2];
    return dub; //Return the reconstructed double;
}

/* WriteDouble
 * This function includes the DisassembleDouble function.  It uses that function
 * to first break the double down, then write it the given location;
 */
#pragma interrupt_level 1
void EEWriteDouble(unsigned char location, double dub)
{
    unsigned char x; //Loop variable;

    EEDisassembleDouble(dub); //Break that double down;

    INTCON = INTCON & 0x3F; //Turn off interrupts;

    for (x = 0; x < 3; x++)
    {
        Write_b_eep(location, DDouble[x]); //Use the predefined functions given by XC8 to write the double;
        location++; //Increase the location;
        Busy_eep(); //Check to see if the write is finished;
    }

    INTCON = INTCON | 0xC0; //Turn on interrupts;
}

/* ReadDouble
 * Used to read a double from a location; implements the ReassembleDouble
 * function;  Returns the reassembled double;
 */
#pragma interrupt_level 1
double EEReadDouble(unsigned char location)
{
    unsigned char x; //Loop variable;

    INTCON = INTCON & 0x3F; //Turn off interrupts;

    for (x = 0; x < 3; x++)
    {
        DDouble[x] = Read_b_eep(location + x); //Read the double, recalling from the earlier write function that the double writes to a location LSByte first;
        Busy_eep();
    }

    INTCON = INTCON | 0xC0; //Turn on interrupts;

    return EEReassembleDouble(); //Reassemble & Return;
}

/* WriteChar
 * Self-explanatory;  Uses predefined library functions to write a char to a
 * location in memory;
 */
#pragma interrupt_level 1
void EEWriteChar(unsigned char location, unsigned char ch)
{
    INTCON = INTCON & 0x3F; //Turn off interrupts;

    Write_b_eep(location, ch); //Write the char to the location;
    Busy_eep(); //Check to see if EEPROM is done writing;

    INTCON = INTCON | 0xC0; //Turn on interrupts;
}

/* ReadChar
 * Use predefined library functions to read a char from a location in memory;
 */
#pragma interrupt_level 1
char EEReadChar(unsigned char location)
{
    unsigned char ch; //Dummy variable;

    INTCON = INTCON & 0x3F; //Turn off interrupts;

    ch = Read_b_eep(location); //Read char into dummy variable;

    INTCON = INTCON | 0xC0; //Turn on interrupts;

    return ch; //Return the character;
}

#pragma interrupt_level 1
void HLVDInt(void) //This is the shutdown routine;
{
    EEWriteDouble(LASTCOMPOSloc, SetAngle);
    EEWriteDouble(KPPARAMloc, Kp);
    EEWriteDouble(KIPARAMloc, Ki);
    EEWriteDouble(KDPARAMloc, Kd);
    EEWriteChar(PORTAloc, PORTA);
    EEWriteChar(PORTBloc, PORTB);
    EEWriteChar(PORTCloc, PORTC);
    EEWriteChar(PIDENABLEloc, PIDEnableFlag);
    EEWriteChar(SAVEDloc, 0x01);
}