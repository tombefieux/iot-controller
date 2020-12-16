#include "Led.h"
#include "Alarm.h"

/**
 * Represents the controller.
 */
class Controller 
{
public:
    Controller();

    void setAlarmEnable();
    void setAlarmDisable();
    void intrusionDetected();

    bool isAlarmEnable() const;

private:
    bool alarmEnable = false;
    Led led;
    Alarm alarm;
}; 
