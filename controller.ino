#include <Arduino.h>
#include <WiFi.h>
#include "DHTesp.h"
#include <HTTPClient.h>

#include "defines.h"
#include "credentials.h"
#include "Controller.h"

Controller controller;
DHTesp dht;
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
    if(controller.isAlarmEnable() && controller.getUsePresenceSensor()) {
      
      controller.intrusionDetected();

      // call API
      char url[100];
      strcpy(url,"http://");
      strcat(url,controller.getServerIP());
      strcat(url,"/intrusion");
      
      HTTPClient http;
      http.begin(url);
      int responseCode = http.POST("");
      http.end();

      if(responseCode == 200 || responseCode == 201) {
        Serial.println("Intrusion successfully transmitted to the server");
      }
      else {
        Serial.println("Failed to transmit the intrusion to the server :'(");
      }
    }
  }
}

void vServerTask(void *pvParameters)
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

              // set the current client as server IP
              char IP[] = "xxx.xxx.xxx.xxx";
              IPAddress ip = client.remoteIP();
              ip.toString().toCharArray(IP, 16);
              controller.setServerIP(IP);

              // routing
              // GET controller
              if(header.indexOf("GET /controller") != -1) {
                Serial.println("Responding to GET controller request");
                
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

              // GET sensors
              else if(header.indexOf("GET /sensors") != -1) {
                Serial.println("Responding to GET sensor request");
                
                // send header
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:application/json");
                client.println("Connection: close");
                client.println();

                StaticJsonDocument<100> json;
                
                if(controller.getUseTemperatureSensor()) {
                  TempAndHumidity newValues = dht.getTempAndHumidity();
                  if (dht.getStatus() != 0) {
                    Serial.println("DHT11 error status: " + String(dht.getStatusString()));
                  }
                  else {
                    float t = newValues.temperature;
                    float h = newValues.humidity;
                    json["temperature"] = t;
                    json["humidity"] = h;
                  }
                }

                char result[100];
                serializeJson(json, result);
                client.println(result);

                client.println();
              }

              // PUT controller
              else if(header.indexOf("PUT /controller") != -1) {
                Serial.println("Responding to PUT controller request");

                // get request body
                String body = "";
                int temp = 0;
                while (temp != -1 && ((char) temp) != '}') {
                  temp = client.read();
                  body += (char) temp;
                }
                
                DynamicJsonDocument doc(1024);
                deserializeJson(doc, body);
                JsonObject obj = doc.as<JsonObject>();

                // name
                if(obj.containsKey("name")) {
                  String nameA = obj["name"];
                  controller.setName(nameA.c_str());
                }

                // useTemperatureSensor
                if(obj.containsKey("useTemperatureSensor")) {
                  bool useTemperatureSensor = obj["useTemperatureSensor"];
                  controller.setUseTemperatureSensor(useTemperatureSensor);
                }

                // usePresenceSensor
                if(obj.containsKey("usePresenceSensor")) {
                  bool usePresenceSensor = obj["usePresenceSensor"];
                  controller.setUsePresenceSensor(usePresenceSensor);
                }

                // maxTemperature
                if(obj.containsKey("maxTemperature")) {
                  int maxTemperature = obj["maxTemperature"];
                  controller.setMaxTemperature((char) maxTemperature);
                }

                // minTemperature
                if(obj.containsKey("minTemperature")) {
                  int minTemperature = obj["minTemperature"];
                  controller.setMinTemperature((char) minTemperature);
                }

                // maxHumidity
                if(obj.containsKey("maxHumidity")) {
                  int maxHumidity = obj["maxHumidity"];
                  controller.setMaxHumidity((char) maxHumidity);
                }

                // minHumidity
                if(obj.containsKey("minHumidity")) {
                  int minHumidity = obj["minHumidity"];
                  controller.setMinHumidity((char) minHumidity);
                }

                // alarmIsEnable
                if(obj.containsKey("alarmIsEnable")) {
                  bool alarmIsEnable = obj["alarmIsEnable"];
                  if(alarmIsEnable
                      && controller.getUsePresenceSensor()
                      && !controller.isAlarmEnable()) {
                    controller.setAlarmEnable();
                  }
                  else {
                    controller.setAlarmDisable();
                  }
                }
                
                // send header
                client.println("HTTP/1.1 200 OK");
                client.println("Connection: close");
                client.println();
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

void vTemperatureCheckTask(void *pvParameters)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  for( ;; )
  {
    if(controller.getUseTemperatureSensor()) {
      TempAndHumidity newValues = dht.getTempAndHumidity();
      if (dht.getStatus() != 0) {
        Serial.println("DHT11 error status: " + String(dht.getStatusString()));
      }
      else {
        float t = newValues.temperature;
        float h = newValues.humidity;
        
        if(t > controller.getMaxTemperature() || t < controller.getMinTemperature()) {
          // call API
          char url[100];
          strcpy(url,"http://");
          strcat(url,controller.getServerIP());
          strcat(url,"/alert");
          
          HTTPClient http;
          http.begin(url);
          http.addHeader("Content-Type", "application/json");
          int responseCode = http.POST("{\"temperature\":" + String(t) + "}");
          http.end();
  
          if(responseCode != 201 && responseCode != 200) {
            Serial.println("Failed to alert the server that the temperature is not OK :'(");
          }
        }
        
        if(h > controller.getMaxHumidity() || h < controller.getMinHumidity()) {
          // call API
          char url[100];
          strcpy(url,"http://");
          strcat(url,controller.getServerIP());
          strcat(url,"/alert");
          
          HTTPClient http;
          http.begin(url);
          http.addHeader("Content-Type", "application/json");
          int responseCode = http.POST("{\"humidity\":" + String(h) + "}");
          http.end();
  
          if(responseCode != 201 && responseCode != 200) {
            Serial.println("Failed to alert the server that the humidity is not OK :'(");
          }
        }
      }
    }
    vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(5000));
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

  // launch web server and temperature sensor
  server.begin();
  Serial.println("Web server started");
  dht.setup(GPIO_TEMP_SENSOR, DHTesp::DHT11);
  Serial.println("DHT initiated\n");

  // tasks creation
  xTaskCreatePinnedToCore(vServerTask, "vServerTask", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(movementDetected, "movementDetected", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(vTemperatureCheckTask, "vTemperatureCheckTask", 10000, NULL, 1, NULL, 0);

  // all is ok
  controller.setAlarmDisable();
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
