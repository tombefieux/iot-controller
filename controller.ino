#include <Arduino.h>

#include "defines.h"
#include "Controller.h"

Controller controller;

SemaphoreHandle_t xBinarySemaphore = NULL;

/**
 * Interuption routine used when the movement sensor is triggered.
 */
void IRAM_ATTR interruptMovement()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
  if(xHigherPriorityTaskWoken != pdFALSE)
    portYIELD_FROM_ISR();
}

/**
 * Code ran when the movement sensor is triggered.
 */
void movementDetected(void *pvParameters)
{
  const char *pcTaskName = "Task movementDetected";
  UBaseType_t uxPriority;
  uxPriority = uxTaskPriorityGet(NULL);
  for(;;)
  {
    xSemaphoreTake(xBinarySemaphore, portMAX_DELAY);

    // if enable
    if(controller.isAlarmEnable()) {
      // todo: call API
      controller.intrusionDetected();
      Serial.println("INTRU OMG");
    }
  }
}

void vTaskPeriodic( void *pvParameters )
{
  const char *pcTaskName = "Task periodique";
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  for( ;; )
  {
    // Serial.printf("%s %d\n", pcTaskName, xPortGetCoreID());
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  infos();

  // Pin managment
  pinMode(GPIO_MOVE_SENSOR, INPUT);
  pinMode(GPIO_LED_DATA, OUTPUT);
  pinMode(GPIO_LED, OUTPUT);
  ledcAttachPin(GPIO_SPEAKER, PWM_CANAL);

  // interuptions
  attachInterrupt(digitalPinToInterrupt(GPIO_MOVE_SENSOR), interruptMovement, RISING);

  // semaphore
  xBinarySemaphore = xSemaphoreCreateBinary();

  // task creation
  xTaskCreate(vTaskPeriodic, "vTaskPeriodic", 10000, NULL, 1, NULL);
  xTaskCreate(movementDetected, "movementDetected", 10000, NULL, 1, NULL);

  // init controller
  controller.init();

  // todo: setup wifi

  for(;;);
}

void loop()
{
  
}

void infos()
{
  esp_chip_info_t out_info;
  esp_chip_info(&out_info);
  Serial.print("CPU freq : "); Serial.println(String(ESP.getCpuFreqMHz()) + " MHz");
  Serial.print("CPU cores : ");  Serial.println(String(out_info.cores));
  Serial.print("Flash size : "); Serial.println(String(ESP.getFlashChipSize() / 1000000) + " MB");
  Serial.print("Free RAM : "); Serial.println(String((long)ESP.getFreeHeap()) + " bytes");
  //Serial.print("Min. free seen : "); Serial.println(String((long)esp_get_minimum_free_heap_size()) + " bytes");
  Serial.print("tskIDLE_PRIORITY : "); Serial.println(String((long)tskIDLE_PRIORITY));
  Serial.print("configMAX_PRIORITIES : "); Serial.println(String((long)configMAX_PRIORITIES));
  Serial.print("configTICK_RATE_HZ : "); Serial.println(String(configTICK_RATE_HZ) + " Hz");
  Serial.print("configKERNEL_INTERRUPT_PRIORITY : "); Serial.println(String((long)configKERNEL_INTERRUPT_PRIORITY));
  Serial.print("configMAX_SYSCALL_INTERRUPT_PRIORITY : "); Serial.println(String((long)configMAX_SYSCALL_INTERRUPT_PRIORITY));
  Serial.println();
}
