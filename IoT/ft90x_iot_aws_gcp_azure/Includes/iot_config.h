#ifndef _IOT_CONFIG_H_
#define _IOT_CONFIG_H_


///////////////////////////////////////////////////////////////////////////////////
#define MQTT_BROKER_UNKNOWN           0
#define MQTT_BROKER_AWS_IOT           1    // Amazon Web Services IoT cloud
#define MQTT_BROKER_GCP_IOT           2    // Google Cloud Platform IoT cloud
#define MQTT_BROKER_MAZ_IOT           3    // Microsoft Azure IoT cloud
#define MQTT_BROKER_AWS_GREENGRASS    4    // local AWS Greengrass broker
//#define MQTT_BROKER_GCP_EDGE          5    // local Google IoT Edge broker
//#define MQTT_BROKER_MAZ_EDGE          6    // local Microsoft IoT Edge broker

#define USE_MQTT_BROKER               MQTT_BROKER_AWS_IOT
//#define USE_MQTT_BROKER               MQTT_BROKER_GCP_IOT
//#define USE_MQTT_BROKER               MQTT_BROKER_MAZ_IOT
//#define USE_MQTT_BROKER               MQTT_BROKER_AWS_GREENGRASS
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
// USE_ROOT_CA
// If enabled, two-way authentication
// If disabled, client authentication, no server authentication, prone to man-in-the-middle attacks
// This must be enabled in production environment
#define USE_ROOT_CA                   1

// USE_PAYLOAD_TIMESTAMP
// If enabled, RTC will be used.
// Note that enabling this increases memory footprint
#define USE_PAYLOAD_TIMESTAMP         1
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
#define SAMPLE_DEVICE_1               1
#define SAMPLE_DEVICE_2               2
#define SAMPLE_DEVICE_3               3

#define USE_MQTT_DEVICE               SAMPLE_DEVICE_1

#if (USE_MQTT_DEVICE == SAMPLE_DEVICE_1)
#define USE_DEVICE_ID                 "ft900device1"
#elif (USE_MQTT_DEVICE == SAMPLE_DEVICE_2)
#define USE_DEVICE_ID                 "ft900device2"
#elif (USE_MQTT_DEVICE == SAMPLE_DEVICE_3)
#define USE_DEVICE_ID                 "ft900device3"
#endif
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
#define MQTT_BROKER_PORT              MQTT_TLS_PORT

#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT)
    #define MQTT_BROKER               "amasgua12bmkv.iot.us-east-1.amazonaws.com"
    #define DEVICE_ID                 USE_DEVICE_ID
    #define MQTT_CLIENT_NAME          DEVICE_ID
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    #define MQTT_BROKER               "mqtt.googleapis.com"
    #define DEVICE_ID                 USE_DEVICE_ID
    #define PROJECT_ID                "FT900IoTProject"
    #define LOCATION_ID               "us-central1"
    #define REGISTRY_ID               "ft900registryid"
    #define USERNAME_ID               "unused"
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
    #define MQTT_BROKER               "ft900iot.azure-devices.net"
    #define DEVICE_ID                 USE_DEVICE_ID
    #define MQTT_CLIENT_NAME          DEVICE_ID
    #define SHARED_KEY_ACCESS         "H1VCQmfWxjpuq+NY2d/PFbX9N7tyr9cgB5LCTTG0j+o="
#elif (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    #define MQTT_BROKER               "192.168.22.12" // local Greengrass server
    #define DEVICE_ID                 USE_DEVICE_ID
    #define MQTT_CLIENT_NAME          DEVICE_ID
#endif
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
// Root CA certificate of all the device certificates below
#if USE_ROOT_CA
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    // This certificate refers to rootca_gg.crt
    extern __flash__ uint8_t ca_data[]        asm("rootca_gg_pem");
    extern __flash__ uint8_t ca_data_end[]    asm("rootca_gg_pem_end");
#else
    // This certificate refers to rootca.crt
    extern __flash__ uint8_t ca_data[]        asm("rootca_pem");
    extern __flash__ uint8_t ca_data_end[]    asm("rootca_pem_end");
#endif
#endif // USE_ROOT_CA

// Device certificates signed by the root CA above
#if (USE_MQTT_DEVICE == SAMPLE_DEVICE_1)
    // This certificate refers to ft900device1_cert.crt
    extern __flash__ uint8_t cert_data[]      asm("ft900device1_cert_pem");
    extern __flash__ uint8_t cert_data_end[]  asm("ft900device1_cert_pem_end");
    // This private key refers to ft900device1_pkey.crt
    extern __flash__ uint8_t pkey_data[]      asm("ft900device1_pkey_pem");
    extern __flash__ uint8_t pkey_data_end[]  asm("ft900device1_pkey_pem_end");
#elif (USE_MQTT_DEVICE == SAMPLE_DEVICE_2)
    // This certificate refers to ft900device2_cert.crt
    extern __flash__ uint8_t cert_data[]      asm("ft900device2_cert_pem");
    extern __flash__ uint8_t cert_data_end[]  asm("ft900device2_cert_pem_end");
    // This private key refers to ft900device2_pkey.crt
    extern __flash__ uint8_t pkey_data[]      asm("ft900device2_pkey_pem");
    extern __flash__ uint8_t pkey_data_end[]  asm("ft900device2_pkey_pem_end");
#elif (USE_MQTT_DEVICE == SAMPLE_DEVICE_3)
    // This certificate refers to ft900device3_cert.crt
    extern __flash__ uint8_t cert_data[]      asm("ft900device3_cert_pem");
    extern __flash__ uint8_t cert_data_end[]  asm("ft900device3_cert_pem_end");
    // This private key refers to ft900device3_pkey.crt
    extern __flash__ uint8_t pkey_data[]      asm("ft900device3_pkey_pem");
    extern __flash__ uint8_t pkey_data_end[]  asm("ft900device3_pkey_pem_end");
#endif
///////////////////////////////////////////////////////////////////////////////////



#endif /* _IOT_CONFIG_H_ */
