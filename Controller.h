#include "Led.h"
#include "Alarm.h"
#include "EEPROM.h"

/**
 * Represents the controller.
 */
class Controller 
{
public:
    Controller();

    void init();
    void setAlarmEnable();
    void setAlarmDisable();
    void intrusionDetected();

    void setName(char[]);
    char* getName();
    
    bool isAlarmEnable() const;

private:
    bool alarmEnable = false;
    Led led;
    Alarm alarm;
    char controllerName[EEPROM_SIZE];
}; 
