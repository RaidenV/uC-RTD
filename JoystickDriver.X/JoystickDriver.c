#include <xc.h>
#include "Joystick.h"
#include "MotorControl.h"
#include "LCD.h"

#pragma config OSC = HSPLL
#pragma config WDT = OFF
#pragma config FCMEN = OFF
#pragma config ECCPMX = 1;


void initialize(void);

void main(void)
{
    initialize();
    while (1)
    {
      //  DetectJoystick();
       // if (JSEnableFlag == 1)
          //  ImplementJSMotion(DetectMovement());
        ImplementJSMotion(-50);
    }

}


void initialize(void)
{
    MotorDriverInit();
    JoystickInit();
    lcdInit();
    INTCON = INTCON | 0xC0;
}