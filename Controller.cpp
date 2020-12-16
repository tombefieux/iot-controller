#include "Controller.h"

Controller::Controller(): led(), alarm() {
    led.setAlarmActive();
}

// todo: more informations to store
void Controller::init() {
    // load data from ROM
    if (EEPROM.begin(EEPROM_SIZE)) {
        for (int i = 0; i < EEPROM_SIZE; i++) {
            byte readValue = EEPROM.read(i);
            if (readValue != 0) {
                controllerName[i] = (char) readValue;
            }
        }

        // all is ok
        setAlarmDisable();
    }
}

void Controller::setName(char newName[]) {
    int addr = 0;
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(addr, newName[i]);
        controllerName[i] = newName[i];
        addr += 1;
    }
    EEPROM.commit();
}

char* Controller::getName() {
    return controllerName;
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
