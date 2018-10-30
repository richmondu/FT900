#include <ft900.h>

#include "tinyprintf.h"     // For tfp_printf
#include "lwip/apps/mqtt.h" // For MQTT_TLS_PORT
#include "FreeRTOS.h"       // For pvPortMalloc
#include "iot_utils.h"      // For USE_MQTT_BROKER



#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



#define IOT_MAX_LEN_CLIENT_ID   128
#define IOT_MAX_LEN_CLIENT_USER 128

#if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
static char* token = NULL;
extern char* token_create_jwt(const char* projectId, const uint8_t* privateKey, size_t privateKeySize, uint32_t timeNow);
extern void  token_free(char** token);
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT) && (MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)
static char* token = NULL;
extern char* token_create_sas(const char* resourceUri, const char* sharedAccessKey, uint32_t timeNow);
extern void  token_free(char** token);
#endif

#if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT) || USE_PAYLOAD_TIMESTAMP
extern void  iot_sntp_start();
extern void  iot_sntp_stop();
//extern void  iot_sntp_set_system_time(uint32_t sec);
extern uint32_t iot_sntp_get_time();
#endif

#if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
extern const uint8_t* iot_certificate_getpkey(size_t* len);
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT) && (MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)
extern const uint8_t* iot_certificate_getca(size_t* len);
#else
extern const uint8_t* iot_certificate_getca(size_t* len);
extern const uint8_t* iot_certificate_getcert(size_t* len);
extern const uint8_t* iot_certificate_getpkey(size_t* len);
#endif

#if (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT) && (MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)
extern const uint8_t* iot_sas_getkey(size_t* len);
#endif



void iot_utils_init()
{
#if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT) || USE_PAYLOAD_TIMESTAMP
    // Google Cloud and Microsoft Azure requires current time as a parameter of the security token
    iot_sntp_start();
    vTaskDelay(pdMS_TO_TICKS(1000));
#endif
}

void iot_utils_free()
{
#if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT) || USE_PAYLOAD_TIMESTAMP
    // Google Cloud and Microsoft Azure requires current time as a parameter of the security token
    iot_sntp_stop();
#endif
}

static inline const char* iot_getbrokername()
{
    return MQTT_BROKER;
}

static inline u16_t iot_getbrokerport()
{
    return MQTT_BROKER_PORT;
}

static inline const char* iot_getid()
{
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    return MQTT_CLIENT_NAME;
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    static char client_id[IOT_MAX_LEN_CLIENT_ID] = {0};
    if (42 + strlen(PROJECT_ID) + strlen(LOCATION_ID) + strlen(REGISTRY_ID) + strlen(DEVICE_ID) > IOT_MAX_LEN_CLIENT_ID)
        return NULL;
    tfp_snprintf(client_id, sizeof(client_id), "projects/%s/locations/%s/registries/%s/devices/%s",
        (char*)PROJECT_ID, (char*)LOCATION_ID, (char*)REGISTRY_ID, (char*)DEVICE_ID);
    return client_id;
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
    return MQTT_CLIENT_NAME;
#else
    return MQTT_CLIENT_NAME;
#endif
}

static inline const char* iot_getusername()
{
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    return MQTT_CLIENT_USER;
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    return MQTT_CLIENT_USER;
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
    static char client_user[IOT_MAX_LEN_CLIENT_USER] = {0};
    tfp_snprintf(client_user, sizeof(client_user), "%s/%s/api-version=2016-11-14", (char*)MQTT_BROKER, (char*)DEVICE_ID);
    return client_user;
#else
    return MQTT_CLIENT_USER;
#endif
}

static inline const char* iot_getpassword()
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

    return MQTT_CLIENT_PASS;

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
    return token;
#elif (MAZ_AUTH_TYPE == AUTH_TYPE_X509CERT)
    return MQTT_CLIENT_PASS;
#endif

#else
    return MQTT_CLIENT_PASS;
#endif
}

const char* iot_utils_getdeviceid()
{
    return DEVICE_ID;
}

int iot_utils_getcertificates( iot_certificates* tls_certificates )
{
    //
    // Initialize certificates
    // Below is an overview of the authentication for Amazon, Google and Azure:
    // 1. Amazon IoT and Greengrass
    // -  supports X509 Certificates for authentication
    // 2. Google IoT
    // -  supports Security Token (JWT) for authentication
    // 3. Microsoft Azure
    // -  supports Security Token (SAS) for authentication
    // -  supports X509 Certificates for authentication
    //
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    // Amazon AWS IoT requires certificate and private key but ca is optional (but recommended)
    tls_certificates->ca = iot_certificate_getca(&tls_certificates->ca_len);
    tls_certificates->cert = iot_certificate_getcert(&tls_certificates->cert_len);
    tls_certificates->pkey = iot_certificate_getpkey(&tls_certificates->pkey_len);
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    // No certificates required to be sent for Google IoT
    // But private key will be used for creating JWT token
    tls_certificates->ca = NULL;
    tls_certificates->ca_len = 0;
    tls_certificates->cert = NULL;
    tls_certificates->cert_len = 0;
    tls_certificates->pkey = NULL;
    tls_certificates->pkey_len = 0;
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
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
#else
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

int iot_utils_getcredentials( iot_credentials* mqtt_credentials )
{
    mqtt_credentials->server_host = iot_getbrokername();
    mqtt_credentials->server_port = iot_getbrokerport();
    mqtt_credentials->client_id = iot_getid();
    mqtt_credentials->client_user = iot_getusername();
    mqtt_credentials->client_pass = iot_getpassword();

    DEBUG_PRINTF("MQTT CREDENTIALS\r\nhost: %s:%d\r\nid: %s\r\nuser: %s\r\npass: %s\r\n\r\n",
        mqtt_credentials->server_host, mqtt_credentials->server_port,
        mqtt_credentials->client_id ? mqtt_credentials->client_id : "NULL",
        mqtt_credentials->client_user ? mqtt_credentials->client_user : "NULL",
        mqtt_credentials->client_pass ? mqtt_credentials->client_pass : "NULL"
        );

    return 0;
}

