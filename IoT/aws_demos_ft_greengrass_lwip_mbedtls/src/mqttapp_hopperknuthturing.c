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

/* Standard includes. */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* MQTT includes. */
#include "aws_bufferpool_config.h"
#include "aws_bufferpool.h"
#include "aws_mqtt_agent_config.h"
#include "aws_mqtt_agent.h"
#include "aws_demo_config.h"

/* Credentials includes. */
#include "aws_clientcredential.h"



/*-----------------------------------------------------------*/

#define HANDLE_RECONNECTION      0

#define hktTOPIC_NAME_PRE        "device/"
#define hktTOPIC_NAME_POST       "/devicePayload"
#define hktTOPIC_NAME_LENGTH     28
#define hktDEVICE_ID_LENGTH      8
#define hktMAX_DATA_LENGTH       bufferpoolconfigBUFFER_DATA_SIZE
#define hktDEVICE_HOPPER         "hopper"
#define hktDEVICE_KNUTH          "knuth"
#define hktDEVICE_TURING         "turing"
#if HANDLE_RECONNECTION
#define htkTERMINATE             { int x=1/0; } // force a crash
#else
#define htkTERMINATE             { for (;;) ; }
#endif

/*-----------------------------------------------------------*/

//static void prvMainTask( void * pvParameters );
static BaseType_t prvConnect( void );
static BaseType_t prvPublish( const char* topic, const char* message );
static BaseType_t prvGenerateTopic( char* topic, int size, const char* device );
static BaseType_t prvGenerateMessage( char* message, int size, const char* device );

static MQTTAgentHandle_t xMQTTHandle = NULL;

/*-----------------------------------------------------------*/



static inline BaseType_t prvConnect( void )
{
    MQTTAgentConnectParams_t xConnectParameters =
    {
        clientcredentialMQTT_BROKER_ENDPOINT, /* The URL of the MQTT broker to connect to. */
        clientcredentialMQTT_BROKER_PORT,     /* Port number on which the MQTT broker is listening. */
        clientcredentialMQTT_CLIENT_ID  	  /* Client Identifier of the MQTT client. It should be unique per broker. */
    };


	if ( MQTT_AGENT_Create( &xMQTTHandle ) != eMQTTAgentSuccess ) {
		DEBUG_PRINTF("Failed to create MQTT Agent.\r\n");
		return pdFAIL;
	}

	DEBUG_PRINTF( "MQTT app connecting to %s:%d...\r\n", clientcredentialMQTT_BROKER_ENDPOINT, clientcredentialMQTT_BROKER_PORT );

	if ( MQTT_AGENT_Connect(xMQTTHandle, &xConnectParameters, democonfigMQTT_TLS_NEGOTIATION_TIMEOUT ) != eMQTTAgentSuccess ) {
		DEBUG_PRINTF( "ERROR:  MQTT application failed to connect.\r\n" );
#if !mqttconfigDISABLE_DELETE
		( void ) MQTT_AGENT_Delete( xMQTTHandle );
#endif
    	return pdFAIL;
	}

	DEBUG_PRINTF( "MQTT app connected.\r\n\r\n" );
    return pdPASS;
}


/*-----------------------------------------------------------*/

static inline BaseType_t prvGenerateTopic(char* topicBuffer, int bufferSize, const char* deviceId)
{
    memset(topicBuffer, 0, bufferSize);
    ( void ) tfp_snprintf( (char* ) topicBuffer, bufferSize, "%s%s%s",
    	hktTOPIC_NAME_PRE, deviceId, hktTOPIC_NAME_POST);

    return pdPASS;
}

static inline BaseType_t prvGenerateMessage(char* msgBuffer, int bufferSize, const char* deviceId)
{
	/* Notes:
		1. Let Greengrass set the timestamp and location
		   Previously, FT900 adds the timestamp and location
		2. snprintf issue with int64/uint64; tfp_printf works.
		3. tfp_snprintf has floating point issues.
	*/

    memset(msgBuffer, 0, bufferSize);
	int len = tfp_snprintf(msgBuffer, bufferSize,
		"{\r\n"\
		" \"deviceId\":\"%s\",\r\n"\
		" \"sensorReading\":%d,\r\n"\
		" \"batteryCharge\":%d,\r\n"\
		" \"batteryDischargeRate\":%d\r\n"\
		"}",
		deviceId,
		rand() % 10 + 30,		// sensorReading 30-40
		rand() % 30 - 10,		// batteryCharge -10-20
		rand() % 5);			// batteryDischargeRate 0-5

	// MQTT message must be less than bufferpoolconfigBUFFER_SIZE
	// Increase bufferpoolconfigBUFFER_SIZE if necessary
	if (len <= 0 || len > bufferpoolconfigBUFFER_DATA_SIZE) {
		return pdFAIL;
	}

	DEBUG_PRINTF("len = %d\r\n", len);
	return pdPASS;
}


/*-----------------------------------------------------------*/

static inline BaseType_t prvPublish( const char* topic, const char* message )
{
    MQTTAgentPublishParams_t* pxPublishParameters = pvPortMalloc(sizeof(MQTTAgentPublishParams_t));
    MQTTAgentReturnCode_t xReturned;


    if (!pxPublishParameters) {
    	DEBUG_PRINTF( "ERROR: Failed to malloc.\r\n");
    	return pdFAIL;
    }

    memset( pxPublishParameters, 0x00, sizeof( MQTTAgentPublishParams_t ) );
    pxPublishParameters->pucTopic = (const uint8_t*) topic;
    pxPublishParameters->pvData = message;
    pxPublishParameters->usTopicLength = ( uint16_t ) strlen( topic );
    pxPublishParameters->ulDataLength = ( uint32_t ) strlen( message );
    pxPublishParameters->xQoS = eMQTTQoS0;

    xReturned = MQTT_AGENT_Publish( xMQTTHandle, pxPublishParameters, democonfigMQTT_TIMEOUT );
    if ( xReturned != eMQTTAgentSuccess ) {
    	vPortFree(pxPublishParameters);
    	DEBUG_PRINTF( "ERROR: Failed to publish.\r\n");
        return pdFAIL;
    }

    vPortFree(pxPublishParameters);
    DEBUG_MINIMAL( "%s:\r\n%s\r\n\r\n", topic, message );
    return pdPASS;
}


/*-----------------------------------------------------------*/

void vMQTTAppTask( void * pvParameters )
{
    ( void ) pvParameters;
    BaseType_t xReturned;
    char* devices[3] = {hktDEVICE_HOPPER, hktDEVICE_KNUTH, hktDEVICE_TURING};
    int deviceCount = sizeof(devices)/sizeof(devices[0]);
    char* pcDataBuffer = NULL;
    char* pcTopicBuffer = NULL;


    DEBUG_PRINTF( "MQTT app has started.\r\n" );

	/* Create the MQTT client object and connect it to the MQTT broker. */
	xReturned = prvConnect();
	if ( xReturned != pdPASS ) {
		DEBUG_PRINTF( "MQTT app could not connect.\r\n" );
		goto exit;
	}

	/* Publish a message every second */
	pcDataBuffer = pvPortMalloc(hktMAX_DATA_LENGTH);
	pcTopicBuffer = pvPortMalloc(hktTOPIC_NAME_LENGTH);
	if ( !pcDataBuffer || !pcTopicBuffer ) {
		DEBUG_PRINTF( "MQTT app could not allocate.\r\n" );
		goto exit;
	}

	do {
		for (int i=0; i<deviceCount && xReturned==pdPASS; i++) {
			prvGenerateTopic(pcTopicBuffer, hktTOPIC_NAME_LENGTH, devices[i]);
			prvGenerateMessage(pcDataBuffer, hktMAX_DATA_LENGTH, devices[i]);
			xReturned = prvPublish( pcTopicBuffer, pcDataBuffer );
		}
	}
	while (xReturned == pdPASS);

	vPortFree(pcDataBuffer);
	vPortFree(pcTopicBuffer);


exit:
	( void ) MQTT_AGENT_Disconnect( xMQTTHandle, democonfigMQTT_TIMEOUT );
#if !mqttconfigDISABLE_DELETE
	( void ) MQTT_AGENT_Delete( xMQTTHandle );
#endif


	DEBUG_MINIMAL( "MQTT app has ended.\r\n" );
	htkTERMINATE;
}


/*----------------------------------------------------------------------------------
 *
1.  FT9XX connects to MQTT broker via secure TLS/SSL connection
    using mbedTLS and LWIP over Ethernet connection.
2.  FT9XX now publishes the sensor data
	on topic 'device/+/devicePayload for Hopper, Knuth and Turing every 1 second.
	{
	   "deviceId": %s,
	   "sensorReading": %d,
	   "batteryCharge": %d,
	   "batteryDischargeRate": %d,
	}

3.  In previous solution, FT900 sets timeStampIso and timeStampEpoch via external RTC
    initialized with datetime value queried via Greengrass (by implementing Lambda)
	{
	   "deviceId": %s,
	   "sensorReading": %d,
	   "batteryCharge": %d,
	   "batteryDischargeRate": %d,
	   "timeStampIso": %s,
	   "timeStampEpoch": %d,
	   "location": {
		  "lat": %f,
		  "lon": %f
	   }
    }

    Removing RTC code provides additional 14kB headroom for integrating sensor code.
    In current solution, the AWS Greengrass gateway will now append the
    timeStampIso, timeStampEpoch, latitude and longitude.

4.  Current size: 184480 (186.4 kB)
     text	   data	    bss	    dec	    hex	filename
     130856	  23332	  30292	 184480	  2d0a0	aws_demos_ft_greengrass_lwip_mbedtls.elf
    Theoretical max size: 262144 (256 kB)
    Remaining size for sensor integration: 77664 (75.8 kB)

------------------------------------------------------------------------------------*/


