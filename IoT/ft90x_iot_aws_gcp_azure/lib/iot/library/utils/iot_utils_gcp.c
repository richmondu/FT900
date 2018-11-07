/*
 * ============================================================================
 * History
 * =======
 * 5 Nov 2018 : Created
 *
 * Copyright (C) Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 * ("Bridgetek") subject to the licence terms set out
 * http://brtchip.com/BRTSourceCodeLicenseAgreement/ ("the Licence Terms").
 * You must read the Licence Terms before downloading or using the Software.
 * By installing or using the Software you agree to the Licence Terms. If you
 * do not agree to the Licence Terms then do not download or use the Software.
 *
 * Without prejudice to the Licence Terms, here is a summary of some of the key
 * terms of the Licence Terms (and in the event of any conflict between this
 * summary and the Licence Terms then the text of the Licence Terms will
 * prevail).
 *
 * The Software is provided "as is".
 * There are no warranties (or similar) in relation to the quality of the
 * Software. You use it at your own risk.
 * The Software should not be used in, or for, any medical device, system or
 * appliance. There are exclusions of Bridgetek liability for certain types of loss
 * such as: special loss or damage; incidental loss or damage; indirect or
 * consequential loss or damage; loss of income; loss of business; loss of
 * profits; loss of revenue; loss of contracts; business interruption; loss of
 * the use of money or anticipated savings; loss of information; loss of
 * opportunity; loss of goodwill or reputation; and/or loss of, damage to or
 * corruption of data.
 * There is a monetary cap on Bridgetek's liability.
 * The Software may have subsequently been amended by another user and then
 * distributed by that other user ("Adapted Software").  If so that user may
 * have additional licence terms that apply to those amendments. However, Bridgetek
 * has no liability in relation to those amendments.
 * ============================================================================
 */

#include <ft900.h>

#include "tinyprintf.h"     // For tfp_printf
#include "lwip/apps/mqtt.h" // For MQTT_TLS_PORT
#include "FreeRTOS.h"       // For pvPortMalloc
#include "../../include/iot/iot_utils.h" // For USE_MQTT_BROKER



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


    DEBUG_PRINTF("MQTT CREDENTIALS\r\nhost: %s:%d\r\nid:   %s\r\nuser: %s\r\npass: %s\r\n\r\n",
        mqtt_credentials->server_host, mqtt_credentials->server_port,
        mqtt_credentials->client_id ? mqtt_credentials->client_id : "NULL",
        mqtt_credentials->client_user ? mqtt_credentials->client_user : "NULL",
        mqtt_credentials->client_pass ? mqtt_credentials->client_pass : "NULL"
        );

    return 0;
}

#endif // #if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
