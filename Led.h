#include <ChainableLED.h>

#include "defines.h"

/**
 * Represents the RGB LED.
 */
class Led 
{
public: 
    Led();
    
    void setAlarmEnable();
    void setAlarmDisable();
    void setAlarmActive();
        
private:
    ChainableLED led;
}; 
