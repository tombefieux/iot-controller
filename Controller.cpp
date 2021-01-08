#include "Controller.h"

Controller::Controller(): led(), alarm() {
    led.setAlarmActive();
}

// todo: more informations to store
void Controller::init() {
    // load data from ROM
    if (EEPROM.begin(NAME_SIZE + SERVER_IP_SIZE + 6)) {
        // name
        for (int i = 0; i < NAME_SIZE; i++) {
            byte readValue = EEPROM.read(i);
            if (readValue != 0) {
                controllerName[i] = (char) readValue;
            }
        }

        // server IP
        for (int i = 0; i < SERVER_IP_SIZE; i++) {
            byte readValue = EEPROM.read(i + NAME_SIZE);
            if (readValue != 0) {
                serverIP[i] = (char) readValue;
            }
        }

        // use temperature
        byte useTemp = EEPROM.read(SERVER_IP_SIZE + NAME_SIZE);
        if(useTemp == (byte) 1) useTemperatureSensor = true;
        else useTemperatureSensor = false;

        // use presence
        byte usePresence = EEPROM.read(SERVER_IP_SIZE + NAME_SIZE + 1);
        if(usePresence == (byte) 1) usePresenceSensor = true;
        else usePresenceSensor = false;

        // max temp
        maxTemperature = (char) EEPROM.read(SERVER_IP_SIZE + NAME_SIZE + 2);

        // min temp
        minTemperature = (char) EEPROM.read(SERVER_IP_SIZE + NAME_SIZE + 3);

        // max humidity
        maxHumidity = (char) EEPROM.read(SERVER_IP_SIZE + NAME_SIZE + 4);

        // min humidity
        minHumidity = (char) EEPROM.read(SERVER_IP_SIZE + NAME_SIZE + 5);
    }
}

void Controller::setName(const char newName[]) {
    for (int i = 0; i < NAME_SIZE; i++) {
        controllerName[i] = newName[i];
    }
    save();
}

void Controller::setServerIP(char newIP[]) {
    for (int i = 0; i < SERVER_IP_SIZE; i++) {
        serverIP[i] = newIP[i];
    }
    save();
}

void Controller::setUseTemperatureSensor(bool value) {
    useTemperatureSensor = value;
    save();
}

void Controller::setMaxTemperature(char value) {
    maxTemperature = value;
    save();
}

void Controller::setMinTemperature(char value) {
    minTemperature = value;
    save();
}

void Controller::setMaxHumidity(char value) {              
    maxHumidity = value;
    save();
}

void Controller::setMinHumidity(char value) {
    minHumidity = value;
    save();
}

void Controller::setUsePresenceSensor(bool value) {
    usePresenceSensor = value;
    save();
}

void Controller::save() const {
    int addr = 0;
    // name 
    for (int i = 0; i < NAME_SIZE; i++) {
        EEPROM.write(addr, controllerName[i]);
        addr++;
    }
    
    // server ip
    for (int i = 0; i < SERVER_IP_SIZE; i++) {
        EEPROM.write(addr, serverIP[i]);
        addr++;
    }
    
    // use temperature
    byte toWrite;
    if(useTemperatureSensor) toWrite = (byte) 1;
    else toWrite = (byte) 0;
    EEPROM.write(addr, toWrite);
    addr++;
    
    // use presence
    if(usePresenceSensor) toWrite = (byte) 1;
    else toWrite = (byte) 0;
    EEPROM.write(addr, toWrite);
    addr++;

    // max temp
    EEPROM.write(addr, maxTemperature);
    addr++;

    // min temp
    EEPROM.write(addr, minTemperature);
    addr++;

    // max humidity
    EEPROM.write(addr, maxHumidity);
    addr++;

    // min humidity
    EEPROM.write(addr, minHumidity);
    addr++;
    
    EEPROM.commit();
}

char Controller::getMaxTemperature() const {
    return maxTemperature;
}

char Controller::getMinTemperature() const {
    return minTemperature;
}

bool Controller::getUseTemperatureSensor() const {
    return useTemperatureSensor;
}

char Controller::getMaxHumidity() const {
    return maxHumidity;
}

char Controller::getMinHumidity() const {
    return minHumidity;
}

bool Controller::getUsePresenceSensor() const {
    return usePresenceSensor;
}

char* Controller::getName() {
    return controllerName;
}

char* Controller::getServerIP() {
    return serverIP;
}

void Controller::getDescription(char* destination) const {
    StaticJsonDocument<200> json;
    
    json["name"] = controllerName;
    json["useTemperatureSensor"] = useTemperatureSensor;
    json["usePresenceSensor"] = usePresenceSensor;
    json["alarmIsEnable"] = alarmEnable;
    json["maxTemperature"] = (int) maxTemperature;
    json["minTemperature"] = (int) minTemperature;
    json["maxHumidity"] = (int) maxHumidity;
    json["minHumidity"] = (int) minHumidity;

    char result[200];
    serializeJson(json, result);
    for (int i = 0; i < 200; i++) {
        destination[i] = result[i];
    }
}

void Controller::setAlarmEnable() {
    alarmEnable = true;
    led.setAlarmEnable();
    alarm.stopAlarm();
}

void Controller::setAlarmDisable() {
    alarmEnable = false;
    led.setAlarmDisable();
    alarm.stopAlarm();
}

void Controller::intrusionDetected() {
    led.setAlarmActive();
    alarm.startAlarm();
}

bool Controller::isAlarmEnable() const {
    return alarmEnable;
}
