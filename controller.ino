#include <Arduino.h>
#include <WiFi.h>

#include "defines.h"
#include "credentials.h"
#include "Controller.h"

Controller controller;

WiFiServer server(80);

SemaphoreHandle_t xBinarySemaphore = NULL;

const long timeoutTime = 2000;

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

void vServerTask( void *pvParameters )
{
  unsigned long currentTime;
  unsigned long previousTime = 0; 
  String header;

  for (;;) {  
    WiFiClient client = server.available();
    if (client) {
      
      currentTime = millis();
      previousTime = currentTime;
      String currentLine = "";
      while (client.connected() && currentTime - previousTime <= timeoutTime) {
        currentTime = millis();
        if (client.available()) {
          char c = client.read();
          header += c;
          if (c == '\n') {
            // end of request so send response
            if (currentLine.length() == 0) {

              // routing
              // controller
              if(header.indexOf("GET /controller") != -1) {
                // send header
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:application/json");
                client.println("Connection: close");
                client.println();

                char result[200];
                controller.getDescription(result);
                client.println(result);

                client.println();
              }

              else {
                client.println("HTTP/1.1 404 NOT FOUND");
                client.println("Connection: close");
                client.println();
              }

              break;
              
            } else {
              currentLine = "";
            }
          } else if (c != '\r') {
            currentLine += c;
          }
        }
      }
      header = "";
      client.stop();
    }
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  infos();

  // pin managment
  pinMode(GPIO_MOVE_SENSOR, INPUT);
  pinMode(GPIO_LED_DATA, OUTPUT);
  pinMode(GPIO_LED, OUTPUT);
  ledcAttachPin(GPIO_SPEAKER, PWM_CANAL);

  // interuptions
  attachInterrupt(digitalPinToInterrupt(GPIO_MOVE_SENSOR), interruptMovement, RISING);

  // semaphore
  xBinarySemaphore = xSemaphoreCreateBinary();

  // init controller
  controller.init();

  // setup wifi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConnected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // launch web server
  server.begin();

  // task creation
  xTaskCreate(vServerTask, "vServerTask", 10000, NULL, 1, NULL);
  xTaskCreate(movementDetected, "movementDetected", 10000, NULL, 1, NULL);

  // all is ok
  controller.setAlarmDisable();

  for(;;);
}

void loop() {}

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
