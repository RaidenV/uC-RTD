/* 
 * File:   LCD.h
 * Author: raidenv
 *
 * Created on August 1, 2015, 3:33 PM
 */

#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <spi.h>
#include <delays.h>

#define CS PORTAbits.RA2
#define IODIRA_ADDRESS 0x00
#define IODIRB_ADDRESS 0x01
#define GPIOA_ADDRESS 0x12
#define GPIOB_ADDRESS 0x13

void lcdInit(void);
void setIODIR(char, char);
void setGPIO(char, char);
void lcdCommand(char);
void lcdChar(unsigned char);
void lcdGoTo(char);
void lcdWriteString(unsigned char*);
void lcdPrint(unsigned char*, unsigned char*);
void setGPIO(char address, char value)
{
    CS=0; // we are about to initiate transmission
    // pins A2,A1 and A0 of the MCP23S17 chip are equal to 0 because they are grounded
    // we are just going to be writing so R/W=0 also
    WriteSPI1(0x40);    // write command 0b0100[A2][A1][A0][R/W] = 0b01000000 = 0x40
    WriteSPI1(address); // select register by providing address
    WriteSPI1(value);    // set value
    CS=1; // we are ending the transmission
}

void setIODIR(char address, char dir)
{
    CS=0;
    WriteSPI1(0x40);    // write command (0b0100[A2][A1][A0][R/W]) also equal to 0x40
    WriteSPI1(address); // select IODIRB
    WriteSPI1(dir);    // set direction
    CS=1;
}

void lcdCommand(char command)
{
    setGPIO(GPIOA_ADDRESS,0x00); // E=0
    Delay10TCYx(0);
    setGPIO(GPIOB_ADDRESS, command); // send data
    Delay10TCYx(0);
    setGPIO(GPIOA_ADDRESS,0x40); // E=1
    Delay10TCYx(0);
    setGPIO(GPIOA_ADDRESS,0x00); // E=0
    Delay10TCYx(0);
}

void lcdChar(unsigned char letter)
{
    setGPIO(GPIOA_ADDRESS,0x80); // RS=1, we going to send data to be displayed
    Delay10TCYx(0); // let things settle down
    setGPIO(GPIOB_ADDRESS,letter); // send display character
    // Now we need to toggle the enable pin (EN) for the display to take effect
    setGPIO(GPIOA_ADDRESS, 0xc0); // RS=1, EN=1
    Delay10TCYx(0); // let things settle down, this time just needs to be long enough for the chip to detect it as high
    setGPIO(GPIOA_ADDRESS,0x00); // RS=0, EN=0 // this completes the enable pin toggle
    Delay10TCYx(0);
}

void lcdGoTo(char pos)
{
    // add 0x80 to be able to use HD44780 position convention
    lcdCommand(0x80+pos);
}
 
void lcdWriteString(unsigned char *s)
{
    while(*s)
    lcdChar(*s++);
}

void lcdPrint(unsigned char* xmit8722, unsigned char* xmit8680)
{
    unsigned char temp, temp2;

    lcdGoTo(0x00);
    lcdWriteString("8722: ");
    lcdChar(xmit8722[0]);
    lcdChar('.');
    lcdChar(xmit8722[1]);
    lcdChar(xmit8722[2]);
    lcdChar(xmit8722[3]);
    
    
    lcdGoTo(0x40);
    lcdWriteString("8680: ");
    lcdChar(xmit8680[0]);
    lcdChar('.');
    lcdChar(xmit8680[1]);
    lcdChar(xmit8680[2]);
    lcdChar(xmit8680[3]);
}

void lcdInit(void)
{
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
}
    
    
#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

