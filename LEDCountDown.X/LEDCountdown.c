#include <pic18f8722.h>
#include <delays.h>
#include <usart.h>

#define LEDS PORTD

#pragma config OSC = HS
#pragma config FCMEN = OFF
#pragma config WDT = OFF

void main(void)
{
    
    baud1USART(9600);
    
    TRISD = 0x00;
    OSCCON = OSCCON | 0x70;
    unsigned char x = 255;   
    while(1)
    {
        x -= 1;
        LEDS = x;
        
        Delay1KTCYx(100);
        
        if(x == 0)
        {
            x = 255;
        }
    }
}
