#ifndef _IOT_CONFIG_CLOUD_H_
#define _IOT_CONFIG_CLOUD_H_

#include <iot_config.h>

#if (USE_MQTT_BROKER == MQTT_BROKER_THINGSPEAK)



///////////////////////////////////////////////////////////////////////////////////
//
// MQTT CREDENTIALS
//   ThingSpeak
//     MQTT_BROKER = "mqtt.thingspeak.com"
//     MQTT_BROKER_PORT = 8883
//     MQTT_CLIENT_NAME = ANY
//     MQTT_CLIENT_USER = ANY
//     MQTT_CLIENT_PASS = MQTT API KEY   // Check ThingSpeak dashboard
//
///////////////////////////////////////////////////////////////////////////////////

#define USE_DEVICE_ID             "ft900device1"                        // UPDATE ME
#define MQTT_BROKER_PORT          MQTT_TLS_PORT
#define MQTT_BROKER               "mqtt.thingspeak.com"
#define MQTT_CLIENT_NAME          USE_DEVICE_ID
#define MQTT_CLIENT_USER          "user"
#define MQTT_CLIENT_PASS          "7N1MHMHVBZYZS4IV"                    // UPDATE ME

#define THINGSPEAK_CHANNEL_ID     "807960"                              // UPDATE ME
#define THINGSPEAK_WRITE_KEY      "DKIWWUGNXXE6J0GN"                    // UPDATE ME

///////////////////////////////////////////////////////////////////////////////////



#endif // #if (USE_MQTT_BROKER == MQTT_BROKER_ADAFRUITIO)

#endif /* _IOT_CONFIG_CLOUD_H_ */
