#include <ft900.h>

#include "tinyprintf.h"     // For tfp_printf
#include "lwip/apps/mqtt.h" // For MQTT_TLS_PORT
#include "FreeRTOS.h"       // For pvPortMalloc
#include "iot.h"            // For USE_MQTT_BROKER



#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



#if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
static char* token = NULL;
extern char* token_create_jwt(const char* projectId, const uint8_t* privateKey, size_t privateKeySize, uint32_t timeNow);
extern void  token_free(char** token);

extern void  iot_sntp_start();
extern void  iot_sntp_stop();
//extern void  iot_sntp_set_system_time(uint32_t sec);
extern uint32_t iot_sntp_get_time();

#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
static char* token = NULL;
extern char* token_create_sas(const char* resourceUri, const char* sharedAccessKey, uint32_t timeNow);
extern void  token_free(char** token);

extern void  iot_sntp_start();
extern void  iot_sntp_stop();
//extern void  iot_sntp_set_system_time(uint32_t sec);
extern uint32_t iot_sntp_get_time();
#endif





void iot_init()
{
#if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT) || USE_PAYLOAD_TIMESTAMP
    // Google Cloud and Microsoft Azure requires current time as a parameter of the security token
    iot_sntp_start();
    vTaskDelay(pdMS_TO_TICKS(1000));
#endif
}

void iot_free()
{
#if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT) || USE_PAYLOAD_TIMESTAMP
    // Google Cloud and Microsoft Azure requires current time as a parameter of the security token
    iot_sntp_stop();
#endif
}

const char* iot_getbrokername()
{
    return MQTT_BROKER;
}

u16_t iot_getbrokerport()
{
    return MQTT_BROKER_PORT;
}

const char* iot_getid()
{
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    return MQTT_CLIENT_NAME;
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    static char client_id[128] = {0};
    tfp_snprintf(client_id, sizeof(client_id), "projects/%s/locations/%s/registries/%s/devices/%s",
        (char*)PROJECT_ID, (char*)LOCATION_ID, (char*)REGISTRY_ID, (char*)DEVICE_ID);
    return client_id;
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
    return MQTT_CLIENT_NAME;
#elif (USE_MQTT_BROKER == MQTT_BROKER_MOSQUITTO)
    return MQTT_CLIENT_NAME;
#else
    return NULL;
#endif
}

const char* iot_getusername()
{
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    return NULL;
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    return USERNAME_ID;
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
    static char client_user[128] = {0};
    tfp_snprintf(client_user, sizeof(client_user), "%s/%s", (char*)MQTT_BROKER, (char*)DEVICE_ID);
    return client_user;
#else
    return NULL;
#endif
}

const char* iot_getpassword()
{
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)

#if USE_PAYLOAD_TIMESTAMP
    DEBUG_PRINTF("Waiting time request...");
    do {
        vTaskDelay(pdMS_TO_TICKS(1000));
        DEBUG_PRINTF(".");
    }
    while (!iot_sntp_get_time() && net_is_ready());
    DEBUG_PRINTF("done!\r\n\r\n");
#endif

    return NULL;
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)

    const uint8_t *pkey = NULL;
    size_t pkey_len = 0;

    token_free(&token);
    pkey = iot_certificate_getpkey(&pkey_len);
    if (pkey) {
        DEBUG_PRINTF("Waiting time request...");
        do {
            vTaskDelay(pdMS_TO_TICKS(1000));
            DEBUG_PRINTF(".");
        }
        while (!iot_sntp_get_time() && net_is_ready());
        DEBUG_PRINTF("done!\r\n\r\n");

        token = token_create_jwt(PROJECT_ID, pkey, pkey_len, iot_sntp_get_time());
        vPortFree((uint8_t *)pkey);
    }

    return token;

#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)

    static char resourceUri[64] = {0};
    tfp_snprintf(resourceUri, sizeof(resourceUri), "%s/devices/%s", (char*)MQTT_BROKER, (char*)DEVICE_ID);

    token_free(&token);

    DEBUG_PRINTF("Waiting time request...");
    do {
        vTaskDelay(pdMS_TO_TICKS(1000));
        DEBUG_PRINTF(".");
    }
    while (!iot_sntp_get_time() && net_is_ready());
    DEBUG_PRINTF("Waiting time request...done!\r\n\r\n");

    token = token_create_sas(resourceUri, SHARED_KEY_ACCESS, iot_sntp_get_time());
    return token;

#else
    return NULL;
#endif
}

const char* iot_getdeviceid()
{
    return DEVICE_ID;
}
