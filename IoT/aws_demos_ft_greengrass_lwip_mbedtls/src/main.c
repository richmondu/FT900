/*
 * ============================================================================
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

#include "ft900.h" // for sensor integration
#include "FreeRTOSConfig.h" // for pvPortMalloc, vPortFree
#include "tinyprintf.h" // for tfp_snprintf, tfp_printf
#include "iot.h" // for iot interface
#include "iot_config.h" // for optional iot configurables
#include "iot_clientcredential.h" // for required iot configurables



/*-----------------------------------------------------------*/

#define IOT_APP_PAYLOAD_LENGTH        iot_MAX_BUFFER_SIZE
#define IOT_APP_TOPIC_LENGTH          28
#define IOT_APP_DEVICE_HOPPER         "hopper"
#define IOT_APP_DEVICE_KNUTH          "knuth"
#define IOT_APP_DEVICE_TURING         "turing"
#define IOT_APP_TERMINATE             { for (;;) ; }
//#define IOT_APP_TERMINATE             { int x=1/0; } // force a crash

static inline int iot_app_generate_topic( char* pcTopic, int lTopicSize, const char* pcDeviceId )
{
    // Use tfp_snprintf consistently to reduce memory footprint
    int lTopicLen = tfp_snprintf( pcTopic, lTopicSize, "device/%s/devicePayload", pcDeviceId );
    pcTopic[lTopicLen] = '\0';
    return lTopicLen;
}

static inline int iot_app_generate_payload( char* pcPayload, int lPayloadSize, const char* pcDeviceId )
{
    /* Notes:
        1. Let AWS Greengrass (or AWS IoT) set the timestamp and location
           Previously, FT900 adds the timestamp and location
        2. snprintf issue with int64/uint64; tfp_printf works.
        3. tfp_snprintf has floating point issues.
    */

    // Simulate sensor data and packaged it in JSON format
	// In reality, these values should be queried from a real sensor
	// For now, simply use random values
    int lPayloadLen = tfp_snprintf( pcPayload, lPayloadSize,
        "{\r\n"\
        " \"deviceId\":\"%s\",\r\n"\
        " \"sensorReading\":%d,\r\n"\
        " \"batteryCharge\":%d,\r\n"\
        " \"batteryDischargeRate\":%d\r\n"\
        "}",
		pcDeviceId,
        rand() % 10 + 30,        // sensorReading 30-40
        rand() % 30 - 10,        // batteryCharge -10-20
        rand() % 5 );            // batteryDischargeRate 0-5

    // MQTT message must be less than bufferpoolconfigBUFFER_DATA_SIZE
    // Increase bufferpoolconfigBUFFER_DATA_SIZE in aws_bufferpool_config.h if necessary
    if ( lPayloadLen <= 0 || lPayloadLen > IOT_APP_PAYLOAD_LENGTH ) {
        return 0;
    }
    pcPayload[lPayloadLen] = '\0';

    DEBUG_PRINTF( "len = %d\r\n", lPayloadLen );
    return lPayloadLen;
}

/*-----------------------------------------------------------*/

/*
 * User application code that will be triggered by the call to iot_setup()
 * This user application code can securely connect to cloud using iot_connect()
 * and then push data to cloud using iot_publish()
 * Pre-requisites:
 *   1. Certificates should be stored in the certificates folder
 *      with the filenames: ca.crt, cert.crt and cert.key
 *   2. Set the required parameters in iot_clientcredential.h
 *   3. Configure the optional parameters in iot_config.h
 */
void iot_app_task( void * pvParameters )
{
    ( void ) pvParameters;
    BaseType_t xReturned;
    char* pcDevices[3] = { IOT_APP_DEVICE_HOPPER, IOT_APP_DEVICE_KNUTH, IOT_APP_DEVICE_TURING };
    int lDeviceCount = sizeof( pcDevices )/sizeof( pcDevices[0] );


    /* Connect to iot broker
     * Certificates should be stored in the certificates folder
     * with the filenames: ca.crt, cert.crt and cert.key
     * These certificates are compiled using USE_CERTIFICATE_OPTIMIZATION.bat
     *   which is called in the Pre-build step in the "Build Steps" tab in (Properties / C/C++ Build / Settings)
     * These certificates are then linked to the binary by adding the object files in
     *   FT9XX GCC Linker/Miscellaneous/Other objects of "Tool Settings" tab in (Properties / C/C++ Build / Settings)
     * Alternatively, the certificates can be hardcoded in iot_clientcredential.h
     *   by setting IOT_CONFIG_USE_CERT_OPTIMIZATION to 0 in iot_config.h
     */
    iot_connect_params_t xConnectParam = {
        IOT_CLIENTCREDENTIAL_BROKER_ENDPOINT,
        IOT_CLIENTCREDENTIAL_BROKER_PORT,
        IOT_CLIENTCREDENTIAL_CLIENT_ID };
    xReturned = iot_connect( &xConnectParam );
    if ( xReturned != pdPASS ) {
        DEBUG_MINIMAL( "IOT APP connect fail!\r\n" );
        goto exit;
    }

    /* Allocate memory for iot packet */
    iot_publish_params_t xPublishParam = {
        pvPortMalloc( IOT_APP_TOPIC_LENGTH ), 0, 0,
        pvPortMalloc( IOT_APP_PAYLOAD_LENGTH ), 0 };
    if ( !xPublishParam.pucTopic || !xPublishParam.pvData ) {
        DEBUG_MINIMAL( "IOT APP alloc fail!\r\n" );
        goto exit;
    }

    /* Publish to iot broker continuously */
    do {
        for ( int i = 0; i < lDeviceCount && xReturned == pdPASS; i++ ) {

            /* Generate the topic and data payload (payload is ideally in JSON format) */
            xPublishParam.usTopicLength = iot_app_generate_topic(
                ( char* )xPublishParam.pucTopic, IOT_APP_TOPIC_LENGTH, pcDevices[i] );
            xPublishParam.ulDataLength = iot_app_generate_payload(
                ( char* )xPublishParam.pvData, IOT_APP_PAYLOAD_LENGTH, pcDevices[i] );

            DEBUG_MINIMAL( "%s:\r\n%s\r\n\r\n",
                ( char* )xPublishParam.pucTopic, ( char* )xPublishParam.pvData );

            /* Publish the payload on the given topic */
            xReturned = iot_publish( &xPublishParam );
        }
    }
    while ( xReturned == pdPASS );

    /* Release memory for iot packet */
    vPortFree( ( char* )xPublishParam.pucTopic );
    vPortFree( ( char* )xPublishParam.pvData );


exit:
    /* Disconnect from iot broker */
    iot_disconnect();


    DEBUG_MINIMAL( "IOT APP ended.\r\n" );
    IOT_APP_TERMINATE;
}

/*-----------------------------------------------------------*/

/*
 * Troubleshooting tips for common problems:
 * 1. If pvPortMalloc fails, increase configTOTAL_HEAP_SIZE in FreeRTOSConfig.h.
 * 2. If constructing large payloads, increase bufferpoolconfigBUFFER_DATA_SIZE in aws_bufferpool_config.h.
 * 3. If crashes appears, increase TASK_MQTTAPP_STACK_SIZE in iot_setup.c.
 *    Since stack is retrieved from the heap, configTOTAL_HEAP_SIZE should also be increased.
 */
int main( void )
{
    /* Initialize the iot framework and trigger the user-callback function
     * This includes:
     *   Third party libraries (FreeRTOS, LWIP TCP/IP stack, Amazon MQTT, mbedTLS SSL library)
     *   FT900 peripheral libraries (Ethernet, I2C master, UART)
     * and then launch a task for the provided function which will contain the user-application code
     */
    iot_setup( iot_app_task );

    return 0;
}
