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

/* IoT includes. */
#include "iot.h"



/*-----------------------------------------------------------*/

#define MQTT_TLS_NEGOTIATION_TIMEOUT  pdMS_TO_TICKS( 12000 )
#define MQTT_TIMEOUT                  pdMS_TO_TICKS( 2500 )


static MQTTAgentHandle_t xMQTTHandle = NULL;

/*-----------------------------------------------------------*/



/*-----------------------------------------------------------*/

inline iot_status iot_connect( const iot_connect_params_t* param )
{
    MQTTAgentConnectParams_t *pxConnectParameters = (MQTTAgentConnectParams_t *)param;

    if ( MQTT_AGENT_Create( &xMQTTHandle ) != eMQTTAgentSuccess ) {
        DEBUG_PRINTF("Failed to create MQTT Agent.\r\n");
        return pdFAIL;
    }

    DEBUG_PRINTF( "MQTT app connecting to %s:%d...\r\n", pxConnectParameters->pcURL, pxConnectParameters->usPort );

    if ( MQTT_AGENT_Connect(xMQTTHandle, pxConnectParameters, MQTT_TLS_NEGOTIATION_TIMEOUT ) != eMQTTAgentSuccess ) {
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

inline iot_status iot_disconnect( void )
{
    ( void ) MQTT_AGENT_Disconnect( xMQTTHandle, MQTT_TIMEOUT );
    #if !mqttconfigDISABLE_DELETE
    ( void ) MQTT_AGENT_Delete( xMQTTHandle );
    #endif

    DEBUG_PRINTF( "MQTT app connected.\r\n\r\n" );
    return pdPASS;
}

/*-----------------------------------------------------------*/

inline iot_status iot_publish( const iot_publish_params_t* param )
{
    MQTTAgentReturnCode_t xReturned;


    xReturned = MQTT_AGENT_Publish( xMQTTHandle, param, MQTT_TIMEOUT );
    if ( xReturned != eMQTTAgentSuccess ) {
        DEBUG_PRINTF( "ERROR: Failed to publish.\r\n");
        return pdFAIL;
    }

    DEBUG_PRINTF( "MQTT app published.\r\n\r\n" );
    return pdPASS;
}

