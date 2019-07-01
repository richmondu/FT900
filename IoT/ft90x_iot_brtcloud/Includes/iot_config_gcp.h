#ifndef _IOT_CONFIG_CLOUD_H_
#define _IOT_CONFIG_CLOUD_H_

#include <iot_config.h>

#if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)



///////////////////////////////////////////////////////////////////////////////////
//
// MQTT CREDENTIALS
//   Google Cloud IoT
//     MQTT_BROKER = “mqtt.googleapis.com”
//     MQTT_BROKER_PORT = 8883
//     MQTT_CLIENT_NAME = “projects/PROJECT_ID/locations/LOCATION_ID/registries/REGISTRY_ID/devices/DEVICE_ID”
//     MQTT_CLIENT_USER = “ “ // any
//     MQTT_CLIENT_PASS = JWT security token (generated with private key)
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
#define MQTT_BROKER               "mqtt.googleapis.com"

#define PROJECT_ID                "ft900iotproject"
#define LOCATION_ID               "us-central1"
#define REGISTRY_ID               "ft900registryid"

//#define MQTT_CLIENT_NAME        // dynamically generated from above info
#define MQTT_CLIENT_USER          " "
//#define MQTT_CLIENT_PASS        // dynamically generated from above info

///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
//
// TLS CERTIFICATES
//   Sample for Google Cloud IoT (using TLS certificate/JWT security token authentication)
//     Ft900device1_cert.pem // not used by device, registered in cloud only
//     Ft900device1_pkey.pem // used to generate the JWT security token
//
///////////////////////////////////////////////////////////////////////////////////

// Root CA certificate of all the device certificates below
#if USE_ROOT_CA
    // This certificate refers to rootca.pem
    extern __flash__ uint8_t ca_data[]        asm("rootca_pem");
    extern __flash__ uint8_t ca_data_end[]    asm("rootca_pem_end");
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

///////////////////////////////////////////////////////////////////////////////////



#endif // #if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)

#endif /* _IOT_CONFIG_CLOUD_H_ */
