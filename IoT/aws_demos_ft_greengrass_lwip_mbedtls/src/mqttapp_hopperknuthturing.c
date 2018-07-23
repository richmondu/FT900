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

#define hktTOPIC_NAME_PRE     	 "device/"
#define hktTOPIC_NAME_POST    	 "/devicePayload"
#define hktTOPIC_NAME_LENGTH  	 28
#define hktDEVICE_ID_LENGTH   	 8
#define hktMAX_DATA_LENGTH    	 224
#define hktTIMESTAMP_LENGTH    	 20
#define hktDEVICE_HOPPER 		 "hopper"
#define hktDEVICE_KNUTH  		 "knuth"
#define hktDEVICE_TURING 		 "turing"
#define hktTOPIC_ANSWER_TIME 	 "answer/time"
#define hktTOPIC_QUERY_TIME 	 "query/time"
#define hktTOPIC_QUERY_TIME_DATA "{\"query\":\"time\"}"
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
    MQTTAgentReturnCode_t xReturned;
    MQTTAgentConnectParams_t xConnectParameters =
    {
        clientcredentialMQTT_BROKER_ENDPOINT, /* The URL of the MQTT broker to connect to. */
        clientcredentialMQTT_BROKER_PORT,     /* Port number on which the MQTT broker is listening. */
        clientcredentialMQTT_CLIENT_ID  	  /* Client Identifier of the MQTT client. It should be unique per broker. */
    };

    configASSERT( xMQTTHandle == NULL );

	xReturned = MQTT_AGENT_Create( &xMQTTHandle );
	if ( xReturned != eMQTTAgentSuccess ) {
		DEBUG_PRINTF("Failed to create MQTT Agent.\r\n");
		return pdFAIL;
	}

	DEBUG_PRINTF( "MQTT app connecting to %s:%d...\r\n", clientcredentialMQTT_BROKER_ENDPOINT, clientcredentialMQTT_BROKER_PORT );

	xReturned = MQTT_AGENT_Connect(xMQTTHandle, &xConnectParameters, democonfigMQTT_TLS_NEGOTIATION_TIMEOUT );
	if ( xReturned != eMQTTAgentSuccess ) {
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
    ( void ) snprintf( (char* ) topicBuffer, bufferSize, "%s%s%s",
    	hktTOPIC_NAME_PRE, deviceId, hktTOPIC_NAME_POST);

    return pdPASS;
}

static inline BaseType_t prvGenerateMessage(char* msgBuffer, int bufferSize, const char* deviceId)
{
	int64_t timeStampEpoch = 0;
	char timeStampIso[2] = {0};


	// Let Greengrass set the timestamp
	strcpy(timeStampIso, "0");
	timeStampEpoch = 0;

	/* Notes:
	   1. snprintf issue with int64/uint64; tfp_printf works.
	   2. tfp_snprintf has floating point issues.
	*/
	int sensorReading = rand() % 10 + 30;	// sensorReading 30-40
	int batteryCharge = rand() % 30 - 10;	// batteryCharge -10-20
	int batteryDischargeRate = rand() % 5;	// batteryDischargeRate 0-5
	char latitude[12] = "37.26123";
	char longitude[12] = "-119.62123";

    memset(msgBuffer, 0, bufferSize);
	int len = tfp_snprintf(msgBuffer, bufferSize,
		"{\r\n"\
		"  \"deviceId\":\"%s\",\r\n"\
		"  \"timeStampIso\":%s,\r\n"\
		"  \"timeStampEpoch\":%llu,\r\n"\
		"  \"sensorReading\":%d,\r\n"\
		"  \"batteryCharge\":%d,\r\n"\
		"  \"batteryDischargeRate\":%d,\r\n"\
		"  \"location\":{\r\n"\
		"    \"lat\":%s,\r\n"\
		"    \"lon\":%s\r\n"\
		"  }\r\n"\
		"}",
		deviceId,
		timeStampIso, timeStampEpoch,
		sensorReading, batteryCharge, batteryDischargeRate,
		latitude, longitude);

	// MQTT message must be less than bufferpoolconfigBUFFER_SIZE
	// Increase bufferpoolconfigBUFFER_SIZE if necessary
	if (len > bufferpoolconfigBUFFER_SIZE) {
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
    configASSERT( xMQTTHandle != NULL );


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
	tfp_printf( "MQTT app published to %s:\r\n%s\r\n\r\n", topic, message );
    return pdPASS;
}


/*-----------------------------------------------------------*/

void vMQTTAppTask( void * pvParameters )
{
    BaseType_t xReturned;
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    char* devices[3] = {hktDEVICE_HOPPER, hktDEVICE_KNUTH, hktDEVICE_TURING};
    int deviceCount = sizeof(devices)/sizeof(devices[0]);
    char* pcDataBuffer = NULL;
    char* pcTopicBuffer = NULL;
#endif
    ( void ) pvParameters;


    DEBUG_PRINTF( "MQTT app has started.\r\n" );

	/* Create the MQTT client object and connect it to the MQTT broker. */
	xReturned = prvConnect();
	if ( xReturned != pdPASS ) {
		DEBUG_PRINTF( "MQTT app could not connect.\r\n" );
		goto exit;
	}

#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
	/* Publish a message every second */
	pcDataBuffer = pvPortMalloc(hktMAX_DATA_LENGTH);
	pcTopicBuffer = pvPortMalloc(hktTOPIC_NAME_LENGTH);

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

#elif (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT || USE_MQTT_BROKER == MQTT_BROKER_AWS_MOSQUITTO)

	do {
		xReturned = prvPublish( "Topic", "Hello world!" );
	}
	while (xReturned == pdPASS);

#endif


exit:
#if !mqttconfigDISABLE_DISCONNECT
	( void ) MQTT_AGENT_Disconnect( xMQTTHandle, democonfigMQTT_TIMEOUT );
#endif
#if !mqttconfigDISABLE_DELETE
	( void ) MQTT_AGENT_Delete( xMQTTHandle );
#endif

	tfp_printf( "MQTT app has ended.\r\n" );
	DEBUG_PRINTF( "MQTT app has ended.\r\n" );
	htkTERMINATE;
}


/*-----------------------------------------------------------
 *
1.  FT9XX connects to MQTT broker via secure TLS/SSL connection
    using mbedTLS and LWIP over Ethernet connection.
2.  FT9XX now publishes the sensor data with real timestamps
	on topic 'device/+/devicePayload for Hopper, Knuth and Turing every 1 second.
	{
	   "deviceId": %s,
	   "timeStampIso": %s, // force to 0 so that gateway will add himself
	   "timeStampEpoch": %d, // force to 0 so that gateway will add himself
	   "sensorReading": %d,
	   "batteryCharge": %d,
	   "batteryDischargeRate": %d,
	   "location": {
		  "lat": %f,
		  "lon": %f
	   }
	}

In previous solution, FT900 sets timeStampIso and timeStampEpoch via external RTC
initialized with datetime value queried via Greengrass (by implementing Lambda)
Removing this code provides additional 14kB headroom for integrating sensor code.
In current solution, Greengrass will now set timeStampIso and timeStampEpoch.

Current size: 236232 (230.7kB)
Theoretical max size: 262144 (256kB)
Remaining size for sensor integration: 25912 (25.3kB)

With and without DEBUG_PRINTF enabled in FreeRTOSConfig.h
text	   data	    bss	    dec	    hex	filename
175504	  25576	  35152	 236232	  39ac8	aws_demos_ft_greengrass_lwip_mbedtls.elf // greengrass, w/o debug

-----------------------------------------------------------*/


