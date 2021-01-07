#include "Led.h"
#include "Alarm.h"
#include "EEPROM.h"
#include <ArduinoJson.h>


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
    void save() const;
    void getDescription(char*) const;

    // getters and setters
    void setName(const char[]);
    char* getName();

    void setServerIP(char[]);
    char* getServerIP();

    void setUseTemperatureSensor(bool);
    bool getUseTemperatureSensor() const;

    void setUsePresenceSensor(bool);
    bool getUsePresenceSensor() const;

    void setMaxTemperature(char);
    char getMaxTemperature() const;

    void setMinTemperature(char);
    char getMinTemperature() const;

    void setMaxHumidity(char);
    char getMaxHumidity() const;

    void setMinHumidity(char);
    char getMinHumidity() const;
    
    bool isAlarmEnable() const;

private:
    bool alarmEnable = false;
    Led led;
    Alarm alarm;
    char controllerName[NAME_SIZE];
    char serverIP[SERVER_IP_SIZE];
    bool useTemperatureSensor;
    bool usePresenceSensor;
    char maxTemperature;
    char minTemperature;
    char maxHumidity;
    char minHumidity;
}; 
