#include "Controller.h"

Controller::Controller(): led(), alarm() {
    setAlarmDisable();
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
