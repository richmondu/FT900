#include <ft900.h>

#include "tinyprintf.h"     // For tfp_printf
#include "lwip/apps/mqtt.h" // For MQTT_TLS_PORT
#include "FreeRTOS.h"       // For pvPortMalloc
#include "iot_utils.h"      // For USE_MQTT_BROKER



#if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



#define IOT_MAX_LEN_CLIENT_ID   128
#define IOT_MAX_LEN_CLIENT_USER 128



static char* token = NULL;
extern char* token_create_jwt(const char* projectId, const uint8_t* privateKey, size_t privateKeySize, uint32_t timeNow);
extern void  token_free(char** token);

extern void  iot_sntp_start();
extern void  iot_sntp_stop();
extern uint32_t iot_sntp_get_time();

extern const uint8_t* iot_certificate_getpkey(size_t* len);



void iot_utils_init()
{
    iot_sntp_start();
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void iot_utils_free()
{
    token_free(&token);
    iot_sntp_stop();
}

const char* iot_utils_getdeviceid()
{
    return DEVICE_ID;
}

/** @brief Gets the TLS certificates. Support Amazon AWS, Google GCP and Microsoft Azure.
 *  @param handle tls_certificates Pointer to structure to contain the TLS certificates.
 *  @returns Returns returns 0 if successful.
 */
int iot_utils_getcertificates( iot_certificates* tls_certificates )
{
    // No certificates required to be sent for Google IoT
    // But private key will be used for creating JWT token
    tls_certificates->ca = NULL;
    tls_certificates->ca_len = 0;
    tls_certificates->cert = NULL;
    tls_certificates->cert_len = 0;
    tls_certificates->pkey = NULL;
    tls_certificates->pkey_len = 0;

    DEBUG_PRINTF("TLS CERTIFICATES\r\nca: %p [%d]\r\ncert: %p [%d]\r\npkey: %p [%d]\r\n\r\n",
        tls_certificates->ca, tls_certificates->ca_len,
        tls_certificates->cert, tls_certificates->cert_len,
        tls_certificates->pkey, tls_certificates->pkey_len
        );

    return 0;
}

/** @brief Gets the MQTT credentials. Support Amazon AWS, Google GCP and Microsoft Azure.
 *  @param handle mqtt_credentials Pointer to structure to contain the MQTT credentials.
 *  @returns Returns returns 0 if successful.
 */
int iot_utils_getcredentials( iot_credentials* mqtt_credentials )
{
    mqtt_credentials->server_host = MQTT_BROKER;
    mqtt_credentials->server_port = MQTT_BROKER_PORT;


    static char client_id[IOT_MAX_LEN_CLIENT_ID] = {0};
    if (42 + strlen(PROJECT_ID) + strlen(LOCATION_ID) + strlen(REGISTRY_ID) + strlen(DEVICE_ID) > IOT_MAX_LEN_CLIENT_ID)
        return -1;
    tfp_snprintf(client_id, sizeof(client_id), "projects/%s/locations/%s/registries/%s/devices/%s",
        (char*)PROJECT_ID, (char*)LOCATION_ID, (char*)REGISTRY_ID, (char*)DEVICE_ID);
    mqtt_credentials->client_id = client_id;

    mqtt_credentials->client_user = MQTT_CLIENT_USER;

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
    mqtt_credentials->client_pass = token;


    DEBUG_PRINTF("MQTT CREDENTIALS\r\nhost: %s:%d\r\nid: %s\r\nuser: %s\r\npass: %s\r\n\r\n",
        mqtt_credentials->server_host, mqtt_credentials->server_port,
        mqtt_credentials->client_id ? mqtt_credentials->client_id : "NULL",
        mqtt_credentials->client_user ? mqtt_credentials->client_user : "NULL",
        mqtt_credentials->client_pass ? mqtt_credentials->client_pass : "NULL"
        );

    return 0;
}

#endif // #if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
