#ifndef _IOT_CONFIG_CLOUD_H_
#define _IOT_CONFIG_CLOUD_H_

#include <iot_config.h>

#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)



///////////////////////////////////////////////////////////////////////////////////
//
// MQTT CREDENTIALS
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

#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT)
    #define USE_AWS_ATS           1
    #if !USE_AWS_ATS
        #define USE_AWS_VER       1
    #endif
    #if USE_AWS_ATS
        #define MQTT_BROKER       "amasgua12bmkv-ats.iot.us-east-1.amazonaws.com"
    #else
        #define MQTT_BROKER       "amasgua12bmkv.iot.us-east-1.amazonaws.com"
    #endif
#elif (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    #define MQTT_BROKER           "192.168.22.12" // local Greengrass server
#endif

#define MQTT_CLIENT_NAME          USE_DEVICE_ID
#define MQTT_CLIENT_USER          NULL // not needed
#define MQTT_CLIENT_PASS          NULL // not needed

///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
//
// TLS CERTIFICATES
//   Sample for Amazon AWS IoT (using TLS certificate authentication)
//     Rootca.pem - self-signed ca certificate
//     Rootca_aws_ats.pem - server certificate if connecting to ATS endpoint
//     Rootca_aws_ver.pem - server certificate if not connecting to ATS endpoint
//     Ft900device1_cert.pem
//     Ft900device1_pkey.pem
//   Sample for Amazon AWS Greengrass (using TLS certificate authentication)
//     Rootca_gg.pem
//     Ft900device1_cert.pem
//     Ft900device1_pkey.pem
//
///////////////////////////////////////////////////////////////////////////////////

// Root CA certificate of all the device certificates below
#if USE_ROOT_CA
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    // This certificate refers to rootca_gg.pem
    extern __flash__ uint8_t ca_data[]        asm("rootca_gg_pem");
    extern __flash__ uint8_t ca_data_end[]    asm("rootca_gg_pem_end");
#elif (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT)
#if USE_AWS_ATS
    // This certificate refers to rootca_aws_ats.pem
    extern __flash__ uint8_t ca_data[]        asm("rootca_aws_ats_pem");
    extern __flash__ uint8_t ca_data_end[]    asm("rootca_aws_ats_pem_end");
#elif USE_AWS_VER
    // This certificate refers to rootca_aws_ver.pem
    extern __flash__ uint8_t ca_data[]        asm("rootca_aws_ver_pem");
    extern __flash__ uint8_t ca_data_end[]    asm("rootca_aws_ver_pem_end");
#else
    // This certificate refers to rootca.pem
    extern __flash__ uint8_t ca_data[]        asm("rootca_pem");
    extern __flash__ uint8_t ca_data_end[]    asm("rootca_pem_end");
#endif // USE_ATS
#endif
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



#endif // #if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)

#endif /* _IOT_CONFIG_CLOUD_H_ */
