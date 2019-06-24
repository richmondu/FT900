#ifndef _IOT_CONFIG_CLOUD_H_
#define _IOT_CONFIG_CLOUD_H_

#include <iot_config.h>

#if (USE_MQTT_BROKER == MQTT_BROKER_UBIDOTS)



///////////////////////////////////////////////////////////////////////////////////
//
// MQTT CREDENTIALS
//   Ubidots
//     MQTT_BROKER = “things.ubidots.com”
//     MQTT_BROKER_PORT = 8883
//     MQTT_CLIENT_NAME = ANY
//     MQTT_CLIENT_USER = TOKEN    // Check Ubidots dashboard
//     MQTT_CLIENT_PASS = ANY
//
///////////////////////////////////////////////////////////////////////////////////

#define USE_DEVICE_ID             "ft900device1"                        // UPDATE ME
#define MQTT_BROKER_PORT          MQTT_TLS_PORT
#define MQTT_BROKER               "things.ubidots.com"
#define MQTT_CLIENT_NAME          USE_DEVICE_ID
#define MQTT_CLIENT_USER          "BBFF-WoaxFjBfhh7ZSLwzEwd6UwuRmfeKE7" // UPDATE ME
#define MQTT_CLIENT_PASS          " "

///////////////////////////////////////////////////////////////////////////////////



#endif // #if (USE_MQTT_BROKER == MQTT_BROKER_ADAFRUITIO)

#endif /* _IOT_CONFIG_CLOUD_H_ */
