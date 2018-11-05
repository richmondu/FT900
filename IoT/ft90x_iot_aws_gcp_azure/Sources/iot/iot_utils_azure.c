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
#include "iot_utils.h"      // For USE_MQTT_BROKER



#if (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



#define IOT_MAX_LEN_CLIENT_ID   128
#define IOT_MAX_LEN_CLIENT_USER 128

#if (MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)
static char* token = NULL;
extern char* token_create_sas(const char* resourceUri, const char* sharedAccessKey, uint32_t timeNow);
extern void  token_free(char** token);
#endif

#if (MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN) || USE_PAYLOAD_TIMESTAMP
extern void  iot_sntp_start();
extern void  iot_sntp_stop();
extern uint32_t iot_sntp_get_time();
#endif

extern const uint8_t* iot_certificate_getca(size_t* len);
#if (MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)
extern const uint8_t* iot_sas_getkey(size_t* len);
#elif (MAZ_AUTH_TYPE == AUTH_TYPE_X509CERT)
extern const uint8_t* iot_certificate_getcert(size_t* len);
extern const uint8_t* iot_certificate_getpkey(size_t* len);
#endif



void iot_utils_init()
{
#if (MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN) || USE_PAYLOAD_TIMESTAMP
    iot_sntp_start();
    vTaskDelay(pdMS_TO_TICKS(1000));
#endif
}

void iot_utils_free()
{
#if (MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)
    token_free(&token);
#endif

#if (MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN) || USE_PAYLOAD_TIMESTAMP
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
// Microsoft Azure provides two authentication types: SAS TOKEN and X509 Certificates
#if (MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)
    // Demonstrate authentication using SAS Token
    tls_certificates->ca = iot_certificate_getca(&tls_certificates->ca_len);
    tls_certificates->cert = NULL;
    tls_certificates->cert_len = 0;
    tls_certificates->pkey = NULL;
    tls_certificates->pkey_len = 0;
#elif (MAZ_AUTH_TYPE == AUTH_TYPE_X509CERT)
    // Demonstrate authentication using X509 Certificates
    tls_certificates->ca = iot_certificate_getca(&tls_certificates->ca_len);
    tls_certificates->cert = iot_certificate_getcert(&tls_certificates->cert_len);
    tls_certificates->pkey = iot_certificate_getpkey(&tls_certificates->pkey_len);
#endif


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


    static char client_user[IOT_MAX_LEN_CLIENT_USER] = {0};
    tfp_snprintf(client_user, sizeof(client_user), "%s/%s/api-version=2016-11-14", (char*)MQTT_BROKER, (char*)DEVICE_ID);
    mqtt_credentials->client_user = client_user;


#if (MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)

    static char resourceUri[64] = {0};
    size_t sharedAccessKeyLen = 0;
    // Read the shared access key for device from a file ft900deviceX_sas_azure.pem
    // Reading from a file instead of hardcoded macro makes it easier to deploy for more devices
    const char *sharedAccessKey = (const char *)iot_sas_getkey(&sharedAccessKeyLen);

    tfp_snprintf(resourceUri, sizeof(resourceUri), "%s/devices/%s", (char*)MQTT_BROKER, (char*)DEVICE_ID);

    token_free(&token);

    DEBUG_PRINTF("Waiting time request...");
    do {
        vTaskDelay(pdMS_TO_TICKS(1000));
        DEBUG_PRINTF(".");
    }
    while (!iot_sntp_get_time() && net_is_ready());
    DEBUG_PRINTF("done!\r\n\r\n");

    token = token_create_sas(resourceUri, sharedAccessKey, iot_sntp_get_time());
    vPortFree((char *)sharedAccessKey);
    mqtt_credentials->client_pass = token;

#elif (MAZ_AUTH_TYPE == AUTH_TYPE_X509CERT)

#if USE_PAYLOAD_TIMESTAMP
    DEBUG_PRINTF("Waiting time request...");
    do {
        vTaskDelay(pdMS_TO_TICKS(1000));
        DEBUG_PRINTF(".");
    }
    while (!iot_sntp_get_time() && net_is_ready());
    DEBUG_PRINTF("done!\r\n\r\n");
#endif

    mqtt_credentials->client_pass = MQTT_CLIENT_PASS;
#endif


    DEBUG_PRINTF("MQTT CREDENTIALS\r\nhost: %s:%d\r\nid:   %s\r\nuser: %s\r\npass: %s\r\n\r\n",
        mqtt_credentials->server_host, mqtt_credentials->server_port,
        mqtt_credentials->client_id ? mqtt_credentials->client_id : "NULL",
        mqtt_credentials->client_user ? mqtt_credentials->client_user : "NULL",
        mqtt_credentials->client_pass ? mqtt_credentials->client_pass : "NULL"
        );

    return 0;
}

#endif
