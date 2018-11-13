#ifndef _IOT_CONFIG_CLOUD_H_
#define _IOT_CONFIG_CLOUD_H_

#include <iot_config.h>

#if (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)



///////////////////////////////////////////////////////////////////////////////////
//
// MQTT CREDENTIALS
//   Microsoft Azure IoT (SAS security token authentication)
//     MQTT_BROKER = “HUB_NAME.azure-devices.net”
//     MQTT_BROKER_PORT = 8883
//     MQTT_CLIENT_NAME = DEVICE_ID
//     MQTT_CLIENT_USER = “HUB_NAME.azure-devices.net/DEVICE_ID/api-version=2016-11-14”
//     MQTT_CLIENT_PASS = SAS security token (generated with shared access key)
//   Microsoft Azure IoT (TLS certificate authentication)
//     MQTT_BROKER = “HUB_NAME.azure-devices.net”
//     MQTT_BROKER_PORT = 8883
//     MQTT_CLIENT_NAME = DEVICE_ID
//     MQTT_CLIENT_USER = “HUB_NAME.azure-devices.net/DEVICE_ID/api-version=2016-11-14”
//     MQTT_CLIENT_PASS = NULL // not needed
//
///////////////////////////////////////////////////////////////////////////////////

// This demo application provides 3 sets of device certificates
#if (USE_MQTT_DEVICE == SAMPLE_DEVICE_1)
#define USE_DEVICE_ID             "ft900device1" // corresponds to ft900device1_cert.pem
#elif (USE_MQTT_DEVICE == SAMPLE_DEVICE_2)
#define USE_DEVICE_ID             "ft900device2" // corresponds to ft900device2_cert.pem
#elif (USE_MQTT_DEVICE == SAMPLE_DEVICE_3)
#define USE_DEVICE_ID             "ft900device3" // corresponds to ft900device3_cert.pem
#endif

#define MQTT_BROKER_PORT          MQTT_TLS_PORT
#define MQTT_BROKER               "FT900IoTHub.azure-devices.net"
#define MQTT_CLIENT_NAME          USE_DEVICE_ID

// Microsoft IoT support 2 authentication types: SASToken and X509Certificates
#define AUTH_TYPE_SASTOKEN        0
#define AUTH_TYPE_X509CERT        1
#if (USE_MQTT_DEVICE == SAMPLE_DEVICE_1)
	// We have set our sample device1 to use SAS Token authentication
	#define MAZ_AUTH_TYPE         AUTH_TYPE_SASTOKEN
#else // SAMPLE_DEVICE_2 and SAMPLE_DEVICE_3
	// We have set our sample device2 and device 3 to use X509 Certificate authentication
	#define MAZ_AUTH_TYPE         AUTH_TYPE_X509CERT
#endif

#if (MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)
    //#define MQTT_CLIENT_USER    // dynamically generated from above info
    //#define MQTT_CLIENT_PASS    // dynamically generated from above info
#elif (MAZ_AUTH_TYPE == AUTH_TYPE_X509CERT)
    //#define MQTT_CLIENT_USER    // dynamically generated from above info
    #define MQTT_CLIENT_PASS      NULL
#endif

///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
//
// TLS CERTIFICATES
//   Sample for Microsoft Azure IoT (using SAS security token authentication)
//     Rootca_azure.pem
//     Ft900device1_sas_azure.pem // used to generate SAS security token
//   Sample for Microsoft Azure IoT (using TLS certificate authentication)
//     Rootca_azure.pem
//     Ft900device1_cert.pem
//     Ft900device1_pkey.pem
//
///////////////////////////////////////////////////////////////////////////////////

// Root CA certificate of all the device certificates below
#if USE_ROOT_CA
    // This certificate refers to rootca_azure.pem
    extern __flash__ uint8_t ca_data[]        asm("rootca_azure_pem");
    extern __flash__ uint8_t ca_data_end[]    asm("rootca_azure_pem_end");
#endif // USE_ROOT_CA

// Device certificates signed by the root CA above
#if (USE_MQTT_DEVICE == SAMPLE_DEVICE_1)
    // This certificate refers to ft900device1_cert.pem
    extern __flash__ uint8_t cert_data[]      asm("ft900device1_cert_pem");
    extern __flash__ uint8_t cert_data_end[]  asm("ft900device1_cert_pem_end");
    // This private key refers to ft900device1_pkey.pem
    extern __flash__ uint8_t pkey_data[]      asm("ft900device1_pkey_pem");
    extern __flash__ uint8_t pkey_data_end[]  asm("ft900device1_pkey_pem_end");
#elif (USE_MQTT_DEVICE == SAMPLE_DEVICE_2)
    // This certificate refers to ft900device2_cert.pem
    extern __flash__ uint8_t cert_data[]      asm("ft900device2_cert_pem");
    extern __flash__ uint8_t cert_data_end[]  asm("ft900device2_cert_pem_end");
    // This private key refers to ft900device2_pkey.pem
    extern __flash__ uint8_t pkey_data[]      asm("ft900device2_pkey_pem");
    extern __flash__ uint8_t pkey_data_end[]  asm("ft900device2_pkey_pem_end");
#elif (USE_MQTT_DEVICE == SAMPLE_DEVICE_3)
    // This certificate refers to ft900device3_cert.pem
    extern __flash__ uint8_t cert_data[]      asm("ft900device3_cert_pem");
    extern __flash__ uint8_t cert_data_end[]  asm("ft900device3_cert_pem_end");
    // This private key refers to ft900device3_pkey.pem
    extern __flash__ uint8_t pkey_data[]      asm("ft900device3_pkey_pem");
    extern __flash__ uint8_t pkey_data_end[]  asm("ft900device3_pkey_pem_end");
#endif

// Shared Access Key for Microsoft Azure IoT
#if (MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)
    // For Microsoft, we have configured our device 1 to use SAS Security Token authentication,
    // not X509 certificate authentication
    // This SharedAccessKey refers to ft900device1_sas_azure.pem
    extern __flash__ uint8_t sas_data[]       asm("ft900device1_sas_azure_pem");
    extern __flash__ uint8_t sas_data_end[]   asm("ft900device1_sas_azure_pem_end");
#endif

///////////////////////////////////////////////////////////////////////////////////



#endif // #if (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)

#endif /* _IOT_CONFIG_CLOUD_H_ */
