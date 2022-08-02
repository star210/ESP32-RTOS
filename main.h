#pragma once


/*
 * Enums
 */
enum Enum_Initialization
{
    WIFI_RECONNECT = 0,
    MQTT_RECONNECT    ,
    DEVICE_RESET
};
/*
 *  These are the structures
 */
typedef struct Message {
  char body[100];
  int ID;
} gGeneralMessage;
