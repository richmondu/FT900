#include <ft900.h>

#include "tinyprintf.h"     // For tfp_printf
#include "lwip/apps/mqtt.h" // For MQTT_TLS_PORT
#include "FreeRTOS.h"       // For pvPortMalloc
#include "iot_utils.h"      // For USE_MQTT_BROKER



#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



#if USE_PAYLOAD_TIMESTAMP
extern void  iot_sntp_start();
extern void  iot_sntp_stop();
extern uint32_t iot_sntp_get_time();
#endif

extern const uint8_t* iot_certificate_getca(size_t* len);
extern const uint8_t* iot_certificate_getcert(size_t* len);
extern const uint8_t* iot_certificate_getpkey(size_t* len);



void iot_utils_init()
{
#if USE_PAYLOAD_TIMESTAMP
    iot_sntp_start();
    vTaskDelay(pdMS_TO_TICKS(1000));
#endif
}

void iot_utils_free()
{
#if USE_PAYLOAD_TIMESTAMP
    iot_sntp_stop();
#endif
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
    // Amazon AWS IoT requires certificate and private key but ca is optional (but recommended)
    tls_certificates->ca = iot_certificate_getca(&tls_certificates->ca_len);
    tls_certificates->cert = iot_certificate_getcert(&tls_certificates->cert_len);
    tls_certificates->pkey = iot_certificate_getpkey(&tls_certificates->pkey_len);


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
    mqtt_credentials->client_id = MQTT_CLIENT_NAME;
    mqtt_credentials->client_user = MQTT_CLIENT_USER;
    mqtt_credentials->client_pass = MQTT_CLIENT_PASS;

#if USE_PAYLOAD_TIMESTAMP
    DEBUG_PRINTF("Waiting time request...");
    do {
        vTaskDelay(pdMS_TO_TICKS(1000));
        DEBUG_PRINTF(".");
    }
    while (!iot_sntp_get_time() && net_is_ready());
    DEBUG_PRINTF("done!\r\n\r\n");
#endif


    DEBUG_PRINTF("MQTT CREDENTIALS\r\nhost: %s:%d\r\nid: %s\r\nuser: %s\r\npass: %s\r\n\r\n",
        mqtt_credentials->server_host, mqtt_credentials->server_port,
        mqtt_credentials->client_id ? mqtt_credentials->client_id : "NULL",
        mqtt_credentials->client_user ? mqtt_credentials->client_user : "NULL",
        mqtt_credentials->client_pass ? mqtt_credentials->client_pass : "NULL"
        );

    return 0;
}

#endif // #if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
