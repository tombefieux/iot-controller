#include <Arduino.h>
#include "Alarm.h"

// consts
const float Alarm::note[] = {65.41, 69.30, 73.42, 77.78, 82.41, 87.31, 92.50, 98.00, 103.83, 110.00, 116.54, 123.47};

const int Alarm::melodie[][3] = { 
    {4, 2, 2}, {5, 2, 1}, {7, 2, 3}, {0, 3, 6}, 
    {2, 2, 2}, {4, 2, 1},{5, 2, 8}, 
    {7, 2, 2},  {9, 2, 1},  {11, 2, 3},  {5, 3, 6},
    {9, 2, 2}, {11, 2, 1}, {0, 3, 3}, {2, 3, 3}, {4, 3, 3},
    {4, 2, 2}, {5, 2, 1}, {7, 2, 3}, {0, 3, 6},
    {2, 3, 2}, {4, 3, 1},{5, 3, 8}, 
    {7, 2, 2}, {7, 2, 1}, {4, 3, 3}, {2, 3, 2},
    {7, 2, 1}, {5, 3, 3}, {4, 3, 2}, {2, 3, 1},{0, 3, 8}
};

Alarm::Alarm() {}

void Alarm::startAlarm() {
  if(!alarmAlreadyLunched) {
      alarmAlreadyLunched = true;
      xTaskCreate(runAlarm, "runAlarm", 10000, NULL, 1, NULL);
  }
}

void Alarm::stopAlarm() {
    alarmAlreadyLunched = false;
}

void Alarm::runAlarm(void *pvParameters) {
    for(;;) {
        int frequence;
        for ( int i = 0; i < nombreDeNotes ; i++ ) {
            frequence = round(note[melodie[i][0]] * 2.0 * (melodie[i][1] - 1));
            ledcSetup(0, frequence, 12); 
            ledcWrite(0, 2048);  // rapport cyclique 50
            delay(tempo * melodie[i][2] - 50);
            ledcWrite(0, 0); // rapport cyclique 0% (silence, pour séparer les notes adjacentes)
            delay(50);
        }
    }
}
