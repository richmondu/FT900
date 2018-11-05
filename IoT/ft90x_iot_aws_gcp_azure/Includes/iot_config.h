#ifndef _IOT_CONFIG_H_
#define _IOT_CONFIG_H_

#include <ft900.h>



///////////////////////////////////////////////////////////////////////////////////
#define MQTT_BROKER_UNKNOWN           0
#define MQTT_BROKER_AWS_IOT           1    // Amazon Web Services IoT cloud
#define MQTT_BROKER_GCP_IOT           2    // Google Cloud Platform IoT cloud
#define MQTT_BROKER_MAZ_IOT           3    // Microsoft Azure IoT cloud
#define MQTT_BROKER_AWS_GREENGRASS    4    // local AWS Greengrass broker
//#define MQTT_BROKER_GCP_EDGE          5    // local Google IoT Edge broker
//#define MQTT_BROKER_MAZ_EDGE          6    // local Microsoft IoT Edge broker

//#define USE_MQTT_BROKER               MQTT_BROKER_UNKNOWN
//#define USE_MQTT_BROKER               MQTT_BROKER_AWS_IOT
#define USE_MQTT_BROKER               MQTT_BROKER_GCP_IOT
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
// Disable this to save some memory footprint for use of sensor
#define USE_PAYLOAD_TIMESTAMP         1

// USE_MQTT_PUBLISH, USE_MQTT_SUBSCRIBE
// By default, just do publish, no subscribe.
#define USE_MQTT_PUBLISH              1
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
#define USE_MQTT_SUBSCRIBE            0 // Disabled by default; tested working
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
#define USE_MQTT_SUBSCRIBE            0 // Disabled by default; tested working for /config not /events
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
#define USE_MQTT_SUBSCRIBE            0 // Azure does not support MQTT subscribe
#else
#define USE_MQTT_SUBSCRIBE            0
#endif

// DEBUG_IOT_API
// Set to enable/disable logs in iot.c
#define DEBUG_IOT_API                 1
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
// This demo application provides 3 sets of device certificates
// Each one will work for Amazon AWS, Google Cloud and Microsoft Azure
#define SAMPLE_DEVICE_1               1 // corresponds to ft900device1_cert.pem
#define SAMPLE_DEVICE_2               2 // corresponds to ft900device2_cert.pem
#define SAMPLE_DEVICE_3               3 // corresponds to ft900device3_cert.pem

#define USE_MQTT_DEVICE               SAMPLE_DEVICE_1

#if (USE_MQTT_DEVICE == SAMPLE_DEVICE_1)
#define USE_DEVICE_ID                 "ft900device1"
#elif (USE_MQTT_DEVICE == SAMPLE_DEVICE_2)
#define USE_DEVICE_ID                 "ft900device2"
#elif (USE_MQTT_DEVICE == SAMPLE_DEVICE_3)
#define USE_DEVICE_ID                 "ft900device3"
#endif

#define DEVICE_ID                     USE_DEVICE_ID
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
//
// MQTT CREDENTIALS
//   Default:
//     MQTT_BROKER
//     MQTT_BROKER_PORT = 8883
//     MQTT_CLIENT_NAME
//     MQTT_CLIENT_USER
//     MQTT_CLIENT_PASS
//   Amazon AWS IoT
//     MQTT_BROKER = “IDENTIFIER.iot.REGION.amazonaws.com”
//     MQTT_BROKER_PORT = 8883
//     MQTT_CLIENT_NAME = DEVICE_ID or THING_NAME
//     MQTT_CLIENT_USER = NULL // not needed
//     MQTT_CLIENT_PASS = NULL // not needed
//   Amazon AWS Greengrass
//     MQTT_BROKER = IP address or host name of local Greengrass device
//     MQTT_BROKER_PORT = 8883
//     MQTT_CLIENT_NAME = DEVICE_ID or THING_NAME
//     MQTT_CLIENT_USER = NULL // not needed
//     MQTT_CLIENT_PASS = NULL // not needed
//   Google Cloud IoT
//     MQTT_BROKER = “mqtt.googleapis.com”
//     MQTT_BROKER_PORT = 8883
//     MQTT_CLIENT_NAME = “projects/PROJECT_ID/locations/LOCATION_ID/registries/REGISTRY_ID/devices/DEVICE_ID”
//     MQTT_CLIENT_USER = “ “ // any
//     MQTT_CLIENT_PASS = JWT security token (generated with private key)
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
// TLS CERTIFICATES
// Authentication with Amazon AWS, Google Cloud and Microsoft Azure
//   Sample for Amazon AWS IoT (using TLS certificate authentication)
//     Rootca.pem
//     Ft900device1_cert.pem
//     Ft900device1_pkey.pem
//   Sample for Amazon AWS Greengrass (using TLS certificate authentication)
//     Rootca_gg.pem
//     Ft900device1_cert.pem
//     Ft900device1_pkey.pem
//   Sample for Google Cloud IoT (using TLS certificate/JWT security token authentication)
//     Ft900device1_cert.pem // not used by device, registered in cloud only
//     Ft900device1_pkey.pem // used to generate the JWT security token
//   Sample for Microsoft Azure IoT (using SAS security token authentication)
//     Rootca_azure.pem
//     Ft900device1_sas_azure.pem // used to generate SAS security token
//   Sample for Microsoft Azure IoT (using TLS certificate authentication)
//     Rootca_azure.pem
//     Ft900device1_cert.pem
//     Ft900device1_pkey.pem
//
///////////////////////////////////////////////////////////////////////////////////

#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    #include <iot_config_aws.h>
    #define USE_MBEDTLS_MAX_SIZES     0 // memory footprint optimization
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    // Google IoT does not need a root CA
    #undef USE_ROOT_CA
    #define USE_ROOT_CA               0 // memory footprint optimization
    #include <iot_config_gcp.h>
    #define USE_MBEDTLS_MAX_SIZES     0 // memory footprint optimization
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
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
    #include <iot_config_azure.h>

    // Microsoft IoT requires a root CA
    #undef USE_ROOT_CA
    #define USE_ROOT_CA               1
#else
    #define MQTT_BROKER_PORT          MQTT_TLS_PORT
    #define MQTT_BROKER               ""
    #define MQTT_CLIENT_NAME          DEVICE_ID
    #define MQTT_CLIENT_USER          NULL
    #define MQTT_CLIENT_PASS          NULL
#endif

///////////////////////////////////////////////////////////////////////////////////



#endif /* _IOT_CONFIG_H_ */
