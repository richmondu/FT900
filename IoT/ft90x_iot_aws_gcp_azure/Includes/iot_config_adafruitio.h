#ifndef _IOT_CONFIG_CLOUD_H_
#define _IOT_CONFIG_CLOUD_H_

#include <iot_config.h>

#if (USE_MQTT_BROKER == MQTT_BROKER_ADAFRUITIO)



///////////////////////////////////////////////////////////////////////////////////
//
// MQTT CREDENTIALS
//   Adafruit IO
//     MQTT_BROKER = “io.adafruit.com”
//     MQTT_BROKER_PORT = 8883
//     MQTT_CLIENT_NAME = ANY
//     MQTT_CLIENT_USER = Username // Check AdafruitIO dashboard
//     MQTT_CLIENT_PASS = AIO Key  // Check AdafruitIO dashboard
//
///////////////////////////////////////////////////////////////////////////////////

#define USE_DEVICE_ID             "ft900device1"
#define MQTT_BROKER_PORT          MQTT_TLS_PORT
#define MQTT_BROKER               "io.adafruit.com"
#define MQTT_CLIENT_NAME          USE_DEVICE_ID
#define MQTT_CLIENT_USER          "richmondu"
#define MQTT_CLIENT_PASS          "fbecbda926d940b3a7c68742608d358d"

///////////////////////////////////////////////////////////////////////////////////



#endif // #if (USE_MQTT_BROKER == MQTT_BROKER_ADAFRUITIO)

#endif /* _IOT_CONFIG_CLOUD_H_ */
