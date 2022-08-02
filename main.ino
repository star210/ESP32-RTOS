// For the current project, we are just using a single Core. Dedicating the other core for future use. 
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif


//#define DEBUG

/*
 *  Includes 
 *  
 */

#include "stdlib.h"

#include <PubSubClient.h>
#include <Preferences.h>
#include <string.h>
#include "settings.hpp"
#include "oled.hpp"
#include "outputs.hpp"
#include "inputs.hpp"
#include "timers.hpp"
#include "buttons.hpp"
#include "wifi.hpp"
#include "time.hpp"
#include "watchdog.hpp"
#include "defs.hpp"
#include "sdcard.hpp"
#include "memory.hpp"
#include "main.h"

/*
 * Defines
 */
 
#define DELAY_500MS pdMS_TO_TICKS(500)
#define DELAY_10MS  pdMS_TO_TICKS(10)

// Define Externs
_Timer Timer;
_Input Input;
_Output Output;
_Wifi Wifi;
_Time Time;
_Nvs Nvs;

Buttons buttons;
Oled Oled;
WDT Wdt;
Sdcard Sdcard;


WiFiClient espClient;
PubSubClient client(espClient);
Preferences preferences;


// Global Variable
char *inputMessage = "";
char *payloadAsChar = "";
bool publishInputMessage = false;

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const int mqtt_port = 1883;
const char *mqtt_ID = "esp32a01";
// MQTT Credentials
const char *mqtt_username = "remote2";
const char *mqtt_password = "password2";
const char *mqtt_client = "ESP32A01";

int reconnectCounter = 0;
long lastReconnectAttempt = 0;

long lastUpdate = 0;
int counter = 1;
long lastTachoTime = 0;
int rpm;

bool inputBool = false;

using namespace std;





// Handles for Queues
QueueHandle_t serialWriteQueue;
QueueHandle_t initializationQueue;
QueueHandle_t oledQueue;


// Tasks Priority

// Tasks prototypes
void InitializationTask (void *pvParam);
void InputTask (void *pvParam);
void OutputTask (void *pvParam);
void SerialUartTask (void *pvParam);
void MQTT_Task (void *pvParam);
void OLED_DisplayTask(void *pvParam);
// Handles for Tasks
TaskHandle_t Initialization_Task_Handler , Input_Task_Handler, Output_Task_Handler, Display_Task_Handler, Serial_Task_Handler;
TaskHandle_t MQTT_Task_Handler;






// Global Functions   portMAX_DELAY
void writeQueue(char serialMsg[])
{
    gGeneralMessage msg;
    strcpy(msg.body, serialMsg);
    msg.ID = 1;
    xQueueSend( serialWriteQueue , (void *)&msg, portMAX_DELAY);
}






void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

 
/*
 *  Queues Initialization
 */
    
   // Creation of Serial Uart Queue for writing data to Serial Monitor
  serialWriteQueue = xQueueCreate(100 , sizeof(gGeneralMessage));
  // Creating of Intialization Queue for handling future features
  initializationQueue  = xQueueCreate(100, sizeof(gGeneralMessage));
  // Creating of OLED Queue that is responsible for sending messages to the OLED screen
 // oledQueue = xQueueCreate(10  , sizeof(gGeneralMessage));

  
  // Creating intialization task
  xTaskCreatePinnedToCore (
    InitializationTask,
    "Initialization Task",
    1024 ,
    NULL,
    4,
    &Initialization_Task_Handler,
    app_cpu );

   // Creating Input Task
   xTaskCreatePinnedToCore (InputTask,    "Input Task",    1024 ,    NULL,    3,    &Input_Task_Handler,    app_cpu );
  // Creating Output Task
   xTaskCreatePinnedToCore (OutputTask,    "Output Task",    1024 ,    NULL,   3,    &Output_Task_Handler,    app_cpu );
   // Creating Serial Uart Task 
   xTaskCreatePinnedToCore (SerialUartTask,    "Serial UART Task",    1024 ,    NULL,   3,    &Serial_Task_Handler,    app_cpu );
   // Creating MQTT Task
   xTaskCreatePinnedToCore (MQTT_Task,    "MQTT Task",   4096  ,    NULL,   3,    &MQTT_Task_Handler,    app_cpu );
   // Creating Display Task
   xTaskCreatePinnedToCore (OLED_DisplayTask,    "Display Oled Task",   1024  ,    NULL,   3,    &Display_Task_Handler,    app_cpu );
 
  
}

/*
 *  This is an Initialization Task
 */
void InitializationTask(void *pvParam)
{
  gGeneralMessage receiveMsg;

  // Adding the code in Inialization Task
    Nvs.begin();                                   // Update settings from nvs memory
     
     Wire.begin(SDA_PIN, SCL_PIN);  // this should be after Sensors.begin()
     Serial.print("Running software V");
     Serial.println(settings["version"]);
     Oled.begin();
     delay(500);

    // setup MQTT broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
  
  //  buttons.begin();
  //  Output.begin();
   // Input.begin();
    Wifi.connect();
   // Wdt.begin();
    //Sdcard.begin();
  
  
    // Connect to MQTT broker
     reconnect();
 
   while(1)
  {
    writeQueue ("InitializationTask \r\n");
     
//    strcpy(msg.body, "Starting ...");
//    msg.ID = 1;
  //  xQueueSend( oledQueue , (void *)&msg, portMAX_DELAY);
    // making the task to go into the blocking mode. This functionality
    // is added for future purposes
    if ( xQueueReceive(initializationQueue, (void*)&receiveMsg , portMAX_DELAY) == pdTRUE) 
    {
       // Wait Here until a meessage is sent to the Queue.
       switch(receiveMsg.ID)
       {
          case WIFI_RECONNECT:
            //Wifi.resetAP();
            // Now reconnect to the WiFi
            Wifi.connect();
          break;
          case MQTT_RECONNECT:
              reconnect();
          break;
       }
    }

    // #TODO: Suspend the task here 
  }
}

/*
 * This is an Input Task. This task is responsible
 * for all the input handling of the system
 */
void InputTask (void *pvParam)
{
  while(1)
  {
        //writeQueue ("input Task \r\n");
        vTaskDelay(DELAY_10MS);
  }
}

/*
 * This is an output Task. This task is responsible 
 * for handling all the outputs of the system
 */
void OutputTask (void *pvParam)
{
  while(1)
  {
        //writeQueue ("Output Task \r\n");

        vTaskDelay(DELAY_10MS);
  }
}

/*
 * 
 */
void MQTT_Task (void *pvParam)
{
  while(1)
  {
       // writeQueue ("MQTT Task \r\n");
    vTaskDelay(DELAY_10MS);
    
  }
}

/*
 * 
 */
void OLED_DisplayTask(void *pvParam)
{
  //gGeneralMessage receiveMsg;
  //char tempMessage[150];
  while(1)
  {
      //  writeQueue ("OLED Display Task \r\n");

 /*  if(xQueueReceive ( oledQueue , (void*)&receiveMsg , portMAX_DELAY) == pdTRUE) 
   {
      // Decode the message and send to the OLED Display
      Oled.displayln(receiveMsg.body);
#ifdef DEBUG
      snprintf(tempMessage, 150 , "Message written on oLED %s" , receiveMsg.body);
      writeQueue(tempMessage);
#endif
   
   }*/
   vTaskDelay(DELAY_10MS);
    
  }
}
 
/*
 * This is a Serial Uart Task
 */

void SerialUartTask(void *pvParam)
{

  gGeneralMessage receiveMsg;
  while(1)
  {
      if ( xQueueReceive(serialWriteQueue, (void*)&receiveMsg , portMAX_DELAY) == pdTRUE) 
      {
        Serial.print(receiveMsg.body);
        Serial.println(receiveMsg.ID);
        Serial.flush();
      }

  }
 
}


void callback(char* topic, byte * payload, unsigned int length) {

  //Conver *byte to char*
  payload[length] = '\0';   //First terminate payload with a NULL
  payloadAsChar = (char*)payload;
  // Break topic down
  char *payloadId = strtok(topic, "/");
  char *payloadFunc = strtok('\0', "/");
  // Break payload down
  char *payloadName = strtok(payloadAsChar, "/");
  char *payloadData = strtok('\0', "/");

  Serial.print("ID: ");
  Serial.print(payloadId);
  Serial.print(" Function: ");
  Serial.print(payloadFunc);
  Serial.print(" Name: ");
  Serial.print(payloadName);
  Serial.print(" Data: ");
  Serial.println(payloadData);

  // Convert payload data to int when there is something to convert
  payloadData[sizeof(payloadData)] = '\0';                                          // Make payload a string by NULL terminating it.
  int payloadDataAsInt = atoi((char *)payloadData);

  // If topic is a set
  if (strcmp(payloadFunc, "set") == 0) {
    Nvs.set(payloadName, payloadData);
    char* reply = Nvs.get(payloadName);
    publishMQTT("reply", reply);
  }
  // If topic is a get
  if (strcmp(payloadFunc, "get") == 0) {
    char* reply = Nvs.get(payloadName); 
    publishMQTT("reply", reply);
  }
 
  if (strcmp(payloadFunc, "output") == 0) {
    Output.start(payloadAsChar);
    return;
  }

  if (strcmp(payloadFunc, "timer") == 0) {
    Timer.start(payloadAsChar);
    return;
  }

  if (strcmp(payloadFunc, "system") == 0) {

    if (strcmp(payloadName, "publish") == 0) {
      publishInputMessage = ! publishInputMessage;
      Serial.print("Publish input messages: ");
      Serial.println(publishInputMessage);
    }

    if (strcmp(payloadName, "restart") == 0) {
      Serial.println("Resetting ESP32");
      ESP.restart();
      return;
    }

    if (strcmp(payloadName, "save") == 0) {
      Nvs.save();
    }

    if (strcmp(payloadName, "erase") == 0) {
      Nvs.erase();
    }
  }
  // End of MQTT callback
}

char* rpmToPtr (int rpmAsInt) {
  // Convert to char pointer to publish
  char rpmChr[10];
  snprintf(rpmChr, sizeof rpmChr, "%d", rpmAsInt);
  // Copy char array to a char pointer so return a pointer
  char *rpmPtr = (char *)malloc(strlen(rpmChr) + 1);
  strcpy(rpmPtr, rpmChr);
  Serial.print("RPM: ");
  Serial.println(rpmPtr);
  return rpmPtr;
}


void publishInput() {
  if (publishInputMessage == true) {                     // Only publish if switched on
    char* inputMessage = Input.update();
    if (inputMessage != "none") {
      publishMQTT("input", inputMessage);
    }
  }
}

void publishMQTT(char* topic, char* payload) {
  char dataArray[30];
  snprintf(dataArray, sizeof dataArray, "%s/%s", mqtt_ID, topic);
  char *result = (char *)malloc(strlen(dataArray) + 1);                 // Copy char array to a char pointer so return a pointer
  strcpy(result, dataArray);
  client.publish(result, payload);
  Serial.print(result);
  Serial.print("/");
  Serial.println(payload);

}

void subscribeMQTT(char* topic) {
  char dataArray[30];
  snprintf(dataArray, sizeof dataArray, "%s/%s", mqtt_ID, topic);
  char *result = (char *)malloc(strlen(dataArray) + 1);                 // Copy char array to a char pointer so return a pointer
  strcpy(result, dataArray);
  client.subscribe(result);
  Serial.println(result);
}

void publishSystem () {
  if (Timer.state("timerOne") == false) {
    publishMQTT("wifi", Wifi.getRssiAsQuality());
    publishMQTT("uptime", Time.getUptime());
    publishMQTT("tacho", rpmToPtr(rpm));
    Timer.start("timerOne");                              // Retrigger for next time
  }
}


void reSubscribe () {
  subscribeMQTT("timer");
  subscribeMQTT("output");
  subscribeMQTT("system");
  subscribeMQTT("set");
  subscribeMQTT("get");
  Serial.println("Connected, subscribing... ");
  Oled.displayln("MQTT Connected ");
  Oled.displayln("Subscribing... ");
}

void reconnect() {
  Serial.println("Attempting MQTT connection... ");
  Oled.displayln("Attempting connection ");
  if (client.connect(mqtt_client, mqtt_username, mqtt_password)) {
    reSubscribe();
    reconnectCounter = 0;  // reset counter
  }
  else if (reconnectCounter > 5) {
    Serial.println("Resetting ESP32");
    delay(500);
    // ESP.restart();
  }
  else {
    reconnectCounter ++;
    Serial.print("Attempt: ");
    Serial.print(reconnectCounter);
    Serial.print(" failed, Error: ");
    Serial.print(client.state());
    Serial.print(" Retrying in 5 seconds");
  }
}


void loop() {
  // put your main code here, to run repeatedly:

}
