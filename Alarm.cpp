#include <Arduino.h>
#include "Alarm.h"

Alarm::Alarm() {}

void Alarm::startAlarm() {
  if(!alarmAlreadyLunched) {
      alarmAlreadyLunched = true;
      xTaskCreatePinnedToCore(runAlarm, "runAlarm", 10000, NULL, 1, NULL, 0);
  }
}

void Alarm::stopAlarm() {
    alarmAlreadyLunched = false;
}

void Alarm::runAlarm(void *pvParameters) {
    for(;;) {
        int frequence;
        for ( int i = 5; i < 32 ; i++ ) {
            ledcSetup(0, i*100, 12); 
            ledcWrite(0, 2048);
            delay(50);
        }
    }
}
