#include "KeyValue.h"

double StrippedValue; //This is going to be the value outgoing to the slave;
unsigned char PIDEnableFlag = 0;
unsigned char StrippedKey; //This is going to be the command, whether it's internal to the master or outward to the slave;  this is simply updated by the keyvalue function, it must be acted upon by the master;
unsigned char AZEL = 1; //This identifies which PIC will be addressed;
unsigned char key[5]; //This is the storage variable for the key;
unsigned char value[10]; //This is the storage variable for the value;
unsigned char received[30]; // this is the storage variable for the received bytes from the computer;
unsigned char RCFlag;
unsigned char RECFlag;

double Kp;
double Ki;
double Kd;
double SetAngle = 0;
double CurrentAngle;
double CurrentVelocity;

void RCInt(void)
{
    unsigned char counter = 0; //Initialize a counter variable;
    do
    {
        while (PIR1bits.RCIF == 0); //The routine will immediately fall through this 
        received[counter] = RCREG; //Add the received byte to our "received" array;
        SerTx(RCREG); //Echo the received byte back;
        if ((received[counter] == 0x08) || received[counter] == 0x7F) //If the received byte is a delete or backspace, move the position back one and add a null terminator to that position, signalling a string end;
        {
            if (counter != 0)
                counter--; //decrease the counter;
            received[counter] = '\0'; //add the null-terminator;
        }
        else if ((received[counter] != 0x08) && (received[counter] != carriageReturn)) //If the received byte does not equal delete, or ENTER, then increase the counter to accept the next byte of information;
            counter++;
    }
    while (RCREG != carriageReturn); //Do this process while the received byte is not ENTER;

    SerTx(newLine); //Go to a new line;
    SerTx(carriageReturn); //Return to the base position;
    keyValue(received, LENGTH); //Strip the key and the value from the received byte;
    SerTx(newLine); //Go to a new line;
    SerTx(carriageReturn); //Return to the base position;

    if (StrippedKey == 0x01)
    {
        if (SetAngle != StrippedValue) //This is to safeguard against the user reentering the same angle and it restarting the PID loop;
        {
            SetAngle = StrippedValue;
            PIDEnableFlag = 0x03; //If this is indeed a new angle, let the PID loop know that it is a new angle;
        }
        else
            PIDEnableFlag = 0x01; //Else, simply ensure that the PID loop flag is raised;
    }

    else if (StrippedKey == 0x05)
    {
        Kp = StrippedValue;
    }

    else if (StrippedKey == 0x07)
    {
        Ki = StrippedValue;
    }

    else if (StrippedKey == 0x09)
    {
        Kd = StrippedValue;
    }

    PIR1bits.RCIF = 0; //Clear the RX flag;
}

void keyValue(unsigned char* str, unsigned short length)
{
    unsigned char x = 0;
    unsigned char y = 0;
    unsigned char* str_end;
    unsigned char flag = 0; //The flag is used so that the program doesn't hang when the delimiter is not entered;

    while ((str[x] != DELIMITER) && x < length) //Find the location of the delimiter;
    {
        x++; //Because the first character cannot possibly be the delimiter, we first increase x;
        if (str[x] == DELIMITER) //If we found the delimiter;
        {
            flag = 1; //Set the found delimiter flag;
        }
        else
            flag = 0;
    }
    if ((x < length) && flag) //If x hasn't gone past the bounds of the "received" string and the flag is set (meaning we found the delimiter);
    {
        for (y = 0; y < x && y < KEYLENGTH; y++) //For y < x (the location of the delimiter) and y < the possible length of the key (in this case 5);
        {
            if (isspace(str[y]) == 0) //If the character is not a space, tab, line feed, or carriage return;
                key[y] = str[y]; // The key is equal to the contents of the "received" string before the delimiter;
        }

        SerTxStr("Key: ");
        SerTxStr(key); //Echo the key;

        for (y = 0; str[x + 1] != NULL; y++) //The str[x+1] is to avoid an off-by-one error where x is increased to null;
        {
            x++;
            // if (isspace(str[x]) == 0) //If the character is not a space, tab, line feed, or carriage return;
            value[y] = str[x]; //The value is equal to the contents of the "received" string after the delimiter, but before the end of the string (when str[x+1] = NULL);
        }
        SerTxStr("   Value: ");
        SerTxStr(value);

        if ((strcmp(key, "POS") == 0) || (strcmp(key, "pos") == 0)) //Check if the string matches the "POS" command;
        {
            StrippedKey = 0x01; //If so, set the StrippedKey variable to the command which will be sent to the slave indicating that the slave needs to change position;
            SerTx(';');
        }

        else if ((strcmp(key, "KP") == 0) || (strcmp(key, "kp") == 0)) //Check if the string matches the "KP" command;
        {
            StrippedKey = 0x05; //If so, set the StrippedKey variable to the command which will be sent to the slave indicating that the slave has a new KP value coming;
            SerTx(';');
        }

        else if ((strcmp(key, "KI") == 0) || (strcmp(key, "ki") == 0)) //Check if the string matches the "KI" command;
        {
            StrippedKey = 0x07; //If so, set the StrippedKey variable to the command which will be sent to the slave indicating that the slave has a new KI value coming;
            SerTx(';');
        }

        else if ((strcmp(key, "KD") == 0) || (strcmp(key, "kd") == 0)) //Check if the string matches the "KD" command;
        {
            StrippedKey = 0x09; //If so, set the StrippedKey variable to the command which will be sent to the slave indicating that the slave has a new KD value coming;
            SerTx(';');
        }

        else //If the entered value from the PC contains the proper delimiter character, but does not match any of the associated commands, send a string indicating that it doesn't understand the command;
        {
            SerTxStr("???");
            StrippedKey = 0;
        }
    }

    else if (flag == 0)
    {
        y = 0;
        while (str[y] != carriageReturn)
        {
            //  if (isspace(str[y]) == 0) //If the character is not a space, tab, line feed, or carriage return;
            key[y] = str[y]; // The key is equal to the contents of the "received" string before the delimiter;
            y++;
        }

        SerTxStr("Key: ");
        SerTxStr(key); //Echo the key;

        if ((strcmp(key, "AZ") == 0) || (strcmp(key, "az") == 0)) //Check if the string matches the "AZ" command;
        {
            AZEL = 1; //If so, set the AZEL command to indicate Azimuth;
            SerTx(';');
        }

        else if ((strcmp(key, "EL") == 0) || (strcmp(key, "el") == 0)) //Check if the string matches the "EL" command;
        {
            AZEL = 2; //If so, set the AZEL command to indicate Elevation;
            SerTx(';');
        }

        else if ((strcmp(key, "POS") == 0) || (strcmp(key, "pos") == 0))
        {
            StrippedKey = 0x02;
            SerTx(';');
        }

        else if ((strcmp(key, "VEL") == 0) || (strcmp(key, "vel") == 0))
        {
            StrippedKey = 0x03;
            SerTx(';');
        }

        else if ((strcmp(key, "KP") == 0) || (strcmp(key, "kp") == 0)) //Check if the string matches the Return Slave KP command;
        {
            StrippedKey = 0x04; //If so, the stripped key indicates the return KP command;
            SerTx(';');
        }

        else if ((strcmp(key, "KI") == 0) || (strcmp(key, "ki") == 0)) //Check if the string matches the Return Slave KI command;
        {
            StrippedKey = 0x06; //If so, the stripped key indicates the return KI command;
            SerTx(';');
        }

        else if ((strcmp(key, "KD") == 0) || (strcmp(key, "kd") == 0)) //Check if the string matches the Return Slave KD command;
        {
            StrippedKey = 0x08; //If so, the stripped key indicates the return KD command;
            SerTx(';');
        }

        else if ((strcmp(key, "REC") == 0) || (strcmp(key, "rec") == 0))
        {
            StrippedKey = 0x0A;
            SerTx(';');
            SerNL();
            SerTxStr("The system will now begin the PID loop analysis...");
            SerNL();
            RECFlag = 1;
        }

        else
        {
            SerTxStr("???");
            StrippedKey = 0;
        }
    }

    StrippedValue = strtod(value, &str_end);

    //The following three blocks clear all of the variables used;         

    for (x = 0; x < KEYLENGTH; x++)
    {
        key[x] = NULL;
    }

    for (x = 0; x < VALUELENGTH; x++)
    {
        value[x] = NULL;
    }

    for (x = 0; x < LENGTH; x++)
    {
        received[x] = NULL;
    }
    flag = 0;
    if (RECFlag == 1)
        RCFlag = 0;
    else
        RCFlag = 1;
}