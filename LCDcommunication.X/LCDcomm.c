#include <p18f8722.h>
#include <spi.h>
#include <delays.h>
 
// this is our chip select (CS) pin according to our pic18 explorer board's connections
#define CS PORTAbits.RA2
// addresses from MCP23S17's datasheet, think of the IODIR as TRIS and GPIO as PORT for the MCP23S17 (no the PIC micro)
#define IODIRA_ADDRESS 0x00
#define IODIRB_ADDRESS 0x01
#define GPIOA_ADDRESS 0x12
#define GPIOB_ADDRESS 0x13
// configuration bits
#pragma config OSC = HS         // Oscillator Selection bits (HS oscillator)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config WDT = OFF        // Watchdog Timer (WDT disabled (control is placed on the SWDTEN bit))
 
void setIODIR(char, char);
void setGPIO(char, char);
void lcdCommand(char);
void lcdChar(unsigned char);
void lcdGoTo(char);
void lcdWriteString(unsigned char*);

unsigned char x = 0;
unsigned char y = 0;
unsigned char temp = 0;
unsigned char mynameis[15] = { 'M', 'y', ' ' , 'n', 'a', 'm', 'e', ' ' , 'i', 's', ' ', 'A', 'l', 'e', 'x'};
 
void main(){
    TRISAbits.RA2=0; // our chip select pin needs to be an output so that we can toggle it
    CS=1; // set CS pin to high, meaning we are sending any information to the MCP23S17 chip
 
    // configure SPI: the MCP23S17 chip's max frequency is 10MHz, let's use 10MHz/64 (Note FOSC=10Mhz, our external oscillator)
    OpenSPI1(SPI_FOSC_64, MODE_10, SMPEND); // frequency, master-slave mode, sampling type
    // set LCD pins DB0-DB7 as outputs
    setIODIR(IODIRB_ADDRESS,0x00);
    // set RS and E LCD pins as outputs
    setIODIR(IODIRA_ADDRESS,0x00);
    // RS=0, E=0
    setGPIO(IODIRA_ADDRESS,0x00);
    // Function set: 8 bit, 2 lines, 5x8
    lcdCommand(0b00111111);
    // Cursor or Display Shift
    lcdCommand(0b00001111);
    // clear display
    lcdCommand(0b00000001);
    // entry mode
    lcdCommand(0b00000110);
    // send characters
    lcdWriteString("Hello World"); // using the string function
    lcdGoTo(0x40); // go to line two
    
    while(1){
        for(x = 0; x < 16; x++)
        {
                lcdGoTo((0x40 + x));
                lcdChar(mynameis[y]);
                y++;
                if(y == 15)
                    y = 0;
            
        }
 
    }
}
 
/*
 * used to set the values of the ports ( think of it as when you use a PORT register)
 */
void setGPIO(char address, char value){
    CS=0; // we are about to initiate transmission
    // pins A2,A1 and A0 of the MCP23S17 chip are equal to 0 because they are grounded
    // we are just going to be writing so R/W=0 also
    WriteSPI1(0x40);    // write command 0b0100[A2][A1][A0][R/W] = 0b01000000 = 0x40
    WriteSPI1(address); // select register by providing address
    WriteSPI1(value);    // set value
    CS=1; // we are ending the transmission
}
 
/*
 * used to set the directions of the ports (like when you use TRIS registers)
 * this function is actually identical to setGPIO, but I think a different
 * for setting the port directio helps with keeping the code organized
 */
void setIODIR(char address, char dir){
    CS=0;
    WriteSPI1(0x40);    // write command (0b0100[A2][A1][A0][R/W]) also equal to 0x40
    WriteSPI1(address); // select IODIRB
    WriteSPI1(dir);    // set direction
    CS=1;
}
 
/*
 * used to send commands and settings information
 */
void lcdCommand(char command){
    setGPIO(GPIOA_ADDRESS,0x00); // E=0
    Delay10TCYx(0);
    setGPIO(GPIOB_ADDRESS, command); // send data
    Delay10TCYx(0);
    setGPIO(GPIOA_ADDRESS,0x40); // E=1
    Delay10TCYx(0);
    setGPIO(GPIOA_ADDRESS,0x00); // E=0
    Delay10TCYx(0);
}
 
/*
 * prints out a character to the lcd display
 */
void lcdChar(unsigned char letter){
    setGPIO(GPIOA_ADDRESS,0x80); // RS=1, we going to send data to be displayed
    Delay10TCYx(0); // let things settle down
    setGPIO(GPIOB_ADDRESS,letter); // send display character
    // Now we need to toggle the enable pin (EN) for the display to take effect
    setGPIO(GPIOA_ADDRESS, 0xc0); // RS=1, EN=1
    Delay10TCYx(0); // let things settle down, this time just needs to be long enough for the chip to detect it as high
    setGPIO(GPIOA_ADDRESS,0x00); // RS=0, EN=0 // this completes the enable pin toggle
    Delay10TCYx(0);
}
 
/*
 * the parameter is the position of the cursor according to the HD44780 specs
 * for the lcd display our board has the top row's position range is 01 to
 */
void lcdGoTo(char pos){
    // add 0x80 to be able to use HD44780 position convention
    lcdCommand(0x80+pos);
}
 
void lcdWriteString(unsigned char *s){
    while(*s)
    lcdChar(*s++);
}
