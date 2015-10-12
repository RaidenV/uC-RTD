#include "SPIMaster.h"

unsigned char AZEL;
unsigned char* DoublePtrSPI;
unsigned char DoubleSPI[3];
//These Variables are here temporarily in order to drive the SPI routine;
double Kp;
double Ki;
double Kd;
double SetAngle;
double CurrentAngle;
double CurrentVelocity;

void SPIMInit(void)
{
    OpenSPI(SPI_FOSC_4, MODE_00, SMPMID); //Open with the communication frequency being 10 MHz (TCY);
    TRISAbits.RA1 = 1; //Slave 1 Ready;
    TRISAbits.RA2 = 1; //Slave 2 Ready;
    TRISAbits.RA3 = 0; //Slave 1 Chip Select;
    TRISAbits.RA4 = 0; //Slave 2 Chip Select;
}

unsigned char MReceiveSPI(void)
{
    unsigned char tempChar;
    tempChar = SSPBUF; //Clear the SSPBUF;
    PIR1bits.SSPIF = 0; //Set the flag to 0 (this will let us know when the slave has sent the byte);
    SSPBUF = 0x00; //This starts the clock of the master, allowing the slave to clock its data out;
    while (!PIR1bits.SSPIF); //Wait until the byte was received from the slave;
    return SSPBUF; //Return the byte;
}

void MWriteSPI(unsigned char data)
{
    unsigned char tempChar;
    tempChar = SSPBUF; //Clear the Buffer;
    PIR1bits.SSPIF = 0; //Set the flag to 0;
    SSPBUF = data; //Start the data cycle;
    while (!PIR1bits.SSPIF); //Wait for it to complete;
    Delay10TCYx(1); //Add a slight delay (10 instruction cycles);
}

/* M_RW_Routine
 * This routine controls the entire SPI interface from the Master's end;  After discerning the
 * appropriate slave to be toggled, it sends that chip select low then commences communication;
 * The Master will the repeatedly check to make sure that the slave hasn't been interrupt by the
 * PID Loop while communicating with the slave;
 */
void M_RW_Routine(unsigned char Command)
{
    INTCONbits.GIE = 0; //Disable interrupts for the duration of the transaction;
    unsigned char x = 0;
    if (AZEL == 0)
    {
        SlaveSelect1 = 0;
        while (SlaveReady1 == 1); //Wait for the slave to be ready;
        if ((Command == 0x01) || (Command == 0x05) || (Command == 0x07) || (Command == 0x09))
        {
            MWriteSPI(Command);

            if (Command == 0x01)
            {
                SPIDisassembleDouble(SetAngle); //Break the double into 3 bytes;
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady1 == 1); //Check that the slave is not busy;
                    MWriteSPI(DoubleSPI[x]);
                }
            }
            else if (Command == 0x05)
            {
                SPIDisassembleDouble(Kp); //Break the double into 3 bytes;
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady1 == 1); //Check that the slave is not busy;
                    MWriteSPI(DoubleSPI[x]);
                }
            }
            else if (Command == 0x07)
            {
                SPIDisassembleDouble(Ki); //Break the double into 3 bytes;
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady1 == 1); //Check that the slave is not busy;
                    MWriteSPI(DoubleSPI[x]);
                }
            }
            else if (Command == 0x09)
            {
                SPIDisassembleDouble(Kd); //Break the double into 3 bytes;
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady1 == 1); //Check that the slave is not busy;
                    MWriteSPI(DoubleSPI[x]);
                }
            }
        }

        else if ((Command == 0x02) || (Command == 0x03) || (Command == 0x04) || (Command == 0x06) || (Command == 0x08))
        {
            MWriteSPI(Command);

            if (Command == 0x02)
            {
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady1 == 1); //Check that the slave is not busy;
                    DoubleSPI[x] = MReceiveSPI(); //Receive the 3 bytes of the double;
                }
                CurrentAngle = SPIReassembleDouble(); //Reassemble the double and place it in the appropriate variable;
            }
            else if (Command == 0x03)
            {
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady1 == 1); //Check that the slave is not busy;
                    DoubleSPI[x] = MReceiveSPI(); //Receive the 3 bytes of the double;
                }
                CurrentVelocity = SPIReassembleDouble(); //Reassemble the double and place it in the appropriate variable;
            }
            else if (Command == 0x04)
            {
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady1 == 1); //Check that the slave is not busy;
                    DoubleSPI[x] = MReceiveSPI(); //Receive the 3 bytes of the double;
                }
                Kp = SPIReassembleDouble(); //Reassemble the double and place it in the appropriate variable;
            }
            else if (Command == 0x06)
            {
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady1 == 1); //Check that the slave is not busy;
                    DoubleSPI[x] = MReceiveSPI(); //Receive the 3 bytes of the double;
                }
                Ki = SPIReassembleDouble(); //Reassemble the double and place it in the appropriate variable;
            }
            else if (Command == 0x08)
            {
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady1 == 1); //Check that the slave is not busy;
                    DoubleSPI[x] = MReceiveSPI(); //Receive the 3 bytes of the double;
                }
                Kd = SPIReassembleDouble(); //Reassemble the double and place it in the appropriate variable;
            }
        }
        SlaveSelect1 = 1; //Reset the Slave;
    }

    else if (AZEL == 1)
    {
        SlaveSelect2 = 0;
        while (SlaveReady2 == 1); //Wait for the slave to be ready;
        if ((Command == 0x01) || (Command == 0x05) || (Command == 0x07) || (Command == 0x09))
        {
            MWriteSPI(Command);

            if (Command == 0x01)
            {
                SPIDisassembleDouble(SetAngle); //Break the double into 3 bytes;
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady2 == 1); //Check that the slave is not busy;
                    MWriteSPI(DoubleSPI[x]);
                }
            }
            else if (Command == 0x05)
            {
                SPIDisassembleDouble(Kp); //Break the double into 3 bytes;
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady2 == 1); //Check that the slave is not busy;
                    MWriteSPI(DoubleSPI[x]);
                }
            }
            else if (Command == 0x07)
            {
                SPIDisassembleDouble(Ki); //Break the double into 3 bytes;
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady2 == 1); //Check that the slave is not busy;
                    MWriteSPI(DoubleSPI[x]);
                }
            }
            else if (Command == 0x09)
            {
                SPIDisassembleDouble(Kd); //Break the double into 3 bytes;
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady2 == 1); //Check that the slave is not busy;
                    MWriteSPI(DoubleSPI[x]);
                }
            }
        }

        else if ((Command == 0x02) || (Command == 0x03) || (Command == 0x04) || (Command == 0x06) || (Command == 0x08))
        {
            MWriteSPI(Command);

            if (Command == 0x02)
            {
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady2 == 1); //Check that the slave is not busy;
                    DoubleSPI[x] = MReceiveSPI(); //Receive the 3 bytes of the double;
                }
                CurrentAngle = SPIReassembleDouble(); //Reassemble the double and place it in the appropriate variable;
            }
            else if (Command == 0x03)
            {
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady2 == 1); //Check that the slave is not busy;
                    DoubleSPI[x] = MReceiveSPI(); //Receive the 3 bytes of the double;
                }
                CurrentVelocity = SPIReassembleDouble(); //Reassemble the double and place it in the appropriate variable;
            }
            else if (Command == 0x04)
            {
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady2 == 1); //Check that the slave is not busy;
                    DoubleSPI[x] = MReceiveSPI(); //Receive the 3 bytes of the double;
                }
                Kp = SPIReassembleDouble(); //Reassemble the double and place it in the appropriate variable;
            }
            else if (Command == 0x06)
            {
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady2 == 1); //Check that the slave is not busy;
                    DoubleSPI[x] = MReceiveSPI(); //Receive the 3 bytes of the double;
                }
                Ki = SPIReassembleDouble(); //Reassemble the double and place it in the appropriate variable;
            }
            else if (Command == 0x08)
            {
                for (x = 0; x < 3; x++)
                {
                    while (SlaveReady2 == 1); //Check that the slave is not busy;
                    DoubleSPI[x] = MReceiveSPI(); //Receive the 3 bytes of the double;
                }
                Kd = SPIReassembleDouble(); //Reassemble the double and place it in the appropriate variable;
            }
        }
        SlaveSelect2 = 1; //Reset the Slave;
    }
    INTCONbits.GIE = 1; //Enable interrupts after transaction is complete;
}

void SPIDisassembleDouble(double dub)
{
    DoublePtrSPI = (unsigned char*) &dub; //This sets the pointer to the location of the first byte of the double;
    DoubleSPI[0] = DoublePtrSPI[0]; //The following lines extract the double byte-by-byte into the Double variable;
    DoubleSPI[1] = DoublePtrSPI[1]; //This way, whenever the double is operated upon, the referenced double is not altered;
    DoubleSPI[2] = DoublePtrSPI[2];
}

double SPIReassembleDouble(void)
{
    double dub;
    DoublePtrSPI = (unsigned char*) &dub; //This sets the pointer to the location of the first byte of the double;
    unsigned char x; //Loop variable;

    for (x = 3; x > 0; --x) //
    {
        DoublePtrSPI[x - 1] = DoubleSPI[x - 1]; //This reassembles the double at it's location;
    }

    return dub; //Return the reconstructed double;
}



