#include "Led.h"

Led::Led(): led(GPIO_LED, GPIO_LED_DATA, 1) {}

void Led::setAlarmEnable()
{
    led.setColorRGB(0, 0, 255, 0);
}

void Led::setAlarmDisable() 
{ 
    led.setColorRGB(0, 0, 0, 255);
}

void Led::setAlarmActive() 
{ 
    led.setColorRGB(0, 255, 0, 0);
}
