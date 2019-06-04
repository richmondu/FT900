/*
 * ============================================================================
 * History
 * =======
 * 13 Nov 2018 : Created
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

#ifndef _IOT_CONFIG_BROKERS_H_
#define _IOT_CONFIG_BROKERS_H_



///////////////////////////////////////////////////////////////////////////////////

#define MQTT_BROKER_UNKNOWN           0
#define MQTT_BROKER_AWS_IOT           1    // Amazon Web Services IoT cloud
#define MQTT_BROKER_GCP_IOT           2    // Google Cloud Platform IoT cloud
#define MQTT_BROKER_MAZ_IOT           3    // Microsoft Azure IoT cloud
#define MQTT_BROKER_AWS_GREENGRASS    4    // local AWS Greengrass broker
//#define MQTT_BROKER_GCP_EDGE        5    // local Google IoT Edge broker
//#define MQTT_BROKER_MAZ_EDGE        6    // local Microsoft IoT Edge broker
#define MQTT_BROKER_LOCAL             5    // local MQTT brokers (Pivotal RabbitMQ, Eclipse Mosquitto, Apache Kafka)
#define MQTT_BROKER_COUNT             6

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



///////////////////////////////////////////////////////////////////////////////////

// Microsoft IoT support 2 authentication types: SASToken and X509Certificates
#define AUTH_TYPE_SASTOKEN        0
#define AUTH_TYPE_X509CERT        1

///////////////////////////////////////////////////////////////////////////////////



#endif /* _IOT_CONFIG_BROKERS_H_ */
