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
#include "rpc.h"



/*-----------------------------------------------------------*/

#if 1
#define CONFIG_NOTIFICATION_UART_KEYWORD 	"Hello World"
#define CONFIG_NOTIFICATION_RECIPIENT 		"richmond.umagat@brtchip.com"
#define CONFIG_NOTIFICATION_MESSAGE 		"Hi, How are you today?"
#endif

#define IOT_APP_PAYLOAD_LENGTH      iot_MAX_BUFFER_SIZE
#define IOT_APP_TOPIC_LENGTH        43
#define IOT_APP_TERMINATE           { for (;;) ; }
//#define IOT_APP_TERMINATE         { int x=1/0; } // force a crash
#define PREPEND_REPLY_TOPIC         "server/"

/*-----------------------------------------------------------*/

static inline void display_network_info()
{
    uint8_t* mac = net_get_mac();
    DEBUG_MINIMAL( "MAC=%02X:%02X:%02X:%02X:%02X:%02X\r\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );

    ip_addr_t addr = net_get_ip();
    DEBUG_MINIMAL( "IP=%s\r\n", inet_ntoa(addr) );
    addr = net_get_gateway();
    DEBUG_MINIMAL( "GW=%s\r\n", inet_ntoa(addr) );
    addr = net_get_netmask();
    DEBUG_MINIMAL( "MA=%s\r\n", inet_ntoa(addr) );
    vTaskDelay( pdMS_TO_TICKS(1000) );
}

static inline char* user_generate_subscribe_topic()
{
    static char* topic = NULL;
    if ( !topic ) {
        int len = strlen(IOT_CLIENTCREDENTIAL_CLIENT_ID) + 1 + 1 + 1;
        topic = pvPortMalloc( len );
        if ( topic ) {
            memset( topic, 0, len );
            tfp_snprintf( topic, len, "%s/#", IOT_CLIENTCREDENTIAL_CLIENT_ID );
        }
    }

    return topic;
}

static uint32_t parse_gpio_int( char* ptr, char* str, char end )
{
    char* temp = NULL;

    temp = strchr(ptr, end);
    if (!temp) {
        return -1;
    }
    ptr[temp-ptr] = '\0';

    temp = strstr(ptr, str);
    if (!temp) {
        return -1;
    }
    ptr += (temp-ptr) + strlen(str);

    uint32_t val = strtoul(ptr, NULL, 10);

    return val;
}

static char* parse_gpio_str( char* ptr, char* str, char end )
{
    char* temp = NULL;

    temp = strchr(ptr, end);
    if (!temp) {
        return NULL;
    }
    ptr[temp-ptr] = '\0';
    ptr[temp-ptr-1] = '\0';

    temp = strstr(ptr, str);
    if (!temp) {
        return NULL;
    }
    ptr += (temp-ptr) + strlen(str);
    ptr ++;

    return ptr;
}

MQTTBool_t user_subscribe_receive_cb( void * pvContext, const MQTTPublishData_t * const pxPublishData )
{
    char topic[80] = {0};
    char payload[80] = {0};
	iot_publish_params_t xPublishParam = {topic, sizeof(topic)-1, 1, payload, sizeof(payload)-1 };


    DEBUG_MINIMAL( "callback [%s] [%s]\r\n", (char*)pxPublishData->pucTopic, (char*)pxPublishData->pvData );

    char* ptr = user_generate_subscribe_topic();
    char* ptr2 = strrchr(ptr,  '/');
    int len = ptr2 - ptr;

    if (strncmp(ptr, (char*)pxPublishData->pucTopic, len)!=0) {
        return eMQTTTrue;
    }

    // get api
    ptr = (char*)pxPublishData->pucTopic + len + 1;
    len = strlen(ptr);


#if 0
    ///////////////////////////////////////////////////////////////////////////////////
    // GPIO
    ///////////////////////////////////////////////////////////////////////////////////
    if ( strncmp( ptr, API_GET_GPIO, len ) == 0 ) {

        ptr = (char*)pxPublishData->pvData;

        uint32_t number = parse_gpio_int(ptr, "\"number\": ", '}');
        //DEBUG_PRINTF( "%d\r\n", number );
        uint32_t value = get_gpio(number);

        tfp_snprintf( xPublishParam.pucTopic, xPublishParam.usTopicLength+1, "%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic );
        tfp_snprintf( xPublishParam.pvData, xPublishParam.ulDataLength+1, "{\"number\": %d, \"value\": %d}", (int)number, (int)value );
        iot_publish( &xPublishParam );

    }
    else if ( strncmp( ptr, API_SET_GPIO, len ) == 0 ) {

        ptr = (char*)pxPublishData->pvData;

        uint32_t value  = parse_gpio_int(ptr, "\"value\": ",  '}');
        uint32_t number = parse_gpio_int(ptr, "\"number\": ", ',');
        //DEBUG_PRINTF( "%d %d\r\n", number, value );
        set_gpio(number, value);

        value = get_gpio(number);
        tfp_snprintf( xPublishParam.pucTopic, xPublishParam.usTopicLength+1, "%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic );
        tfp_snprintf( xPublishParam.pvData, xPublishParam.ulDataLength+1, "{\"number\": %d, \"value\": %d}", (int)number, (int)value );
        iot_publish( &xPublishParam );

    }
#endif


    /* Process rtc */
    // MM900Ev1b (RevC) has an internal RTC
    // IoTBoard does not have internal RTC
    // When using IoTBoard, this must be disabled to prevent crash
    // TODO: support external RTC to work on IoTBoard
#if 0 // TEMPORARILY DISABLED FOR THE NEW FT900 IOT BOARD
    ///////////////////////////////////////////////////////////////////////////////////
    // RTC
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_RTC, len ) == 0 ) {

        uint32_t value = get_rtc();
        DEBUG_PRINTF( "%d\r\n", (int)value );

        tfp_snprintf( xPublishParam.pucTopic, xPublishParam.usTopicLength+1, "%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        tfp_snprintf( xPublishParam.pvData, xPublishParam.ulDataLength+1, "{\"value\": %d}", (int)value );
        iot_publish( &xPublishParam );

    }
    else if ( strncmp( ptr, API_SET_RTC, len ) == 0 ) {

        ptr = (char*)pxPublishData->pvData;

        uint32_t value  = parse_gpio_int(ptr, "\"value\": ",  '}');
        //DEBUG_PRINTF( "%d\r\n", value );
        set_rtc(value);

        tfp_snprintf( xPublishParam.pucTopic, xPublishParam.usTopicLength+1, "%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        tfp_snprintf( xPublishParam.pvData, xPublishParam.ulDataLength+1, "{\"value\": %d}", (int)value );
        iot_publish( &xPublishParam );

    }
#endif


    ///////////////////////////////////////////////////////////////////////////////////
    // STATUS
    ///////////////////////////////////////////////////////////////////////////////////
    //else
    if ( strncmp( ptr, API_GET_STATUS, len ) == 0 ) {


        tfp_snprintf( (uint8_t*)xPublishParam.pucTopic, xPublishParam.usTopicLength+1, "%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        tfp_snprintf( (uint8_t*)xPublishParam.pvData, xPublishParam.ulDataLength+1, "{\"value\": \"%s\"}", API_STATUS_RUNNING);
        iot_publish( &xPublishParam );
        DEBUG_PRINTF( "PUB:  %s %s\r\n", topic, payload );
    }
    else if ( strncmp( ptr, API_SET_STATUS, len ) == 0 ) {

        ptr = (char*)pxPublishData->pvData;

        char* status = parse_gpio_str(ptr, "\"value\": ",  '}');
        //DEBUG_PRINTF( "%s\r\n", status );

        if ( strncmp( status, API_STATUS_RESTART, strlen(API_STATUS_RESTART)) == 0 ) {
            tfp_snprintf( xPublishParam.pucTopic, xPublishParam.usTopicLength+1, "%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic );
            tfp_snprintf( xPublishParam.pvData, xPublishParam.ulDataLength+1, "{\"value\": \"%s\"}", API_STATUS_RESTARTING );
            iot_publish( &xPublishParam );
            //xTaskCreate( restart_task, "restart_task", 64, NULL, 3, NULL );
        }

    }

#if 0
    ///////////////////////////////////////////////////////////////////////////////////
    // MAC ADDRESS
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_MAC, len ) == 0 ) {

        uint8_t mac[6] = {0};
        get_mac(mac);

        tfp_snprintf( xPublishParam.pucTopic, xPublishParam.usTopicLength+1, "%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        tfp_snprintf( xPublishParam.pvData, xPublishParam.ulDataLength+1, "{\"value\": \"%02X:%02X:%02X:%02X:%02X:%02X\"}",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
        iot_publish( &xPublishParam );

    }
#if 0
    else if ( strncmp( ptr, API_SET_MAC, len ) == 0 ) {

        ptr = (char*)pxPublishData->pvData;

        uint8_t* mac_str = parse_gpio_str(ptr, "\"value\": ",  '}');
        set_mac(mac_str);
        uint8_t mac[6] = {0};
        get_mac(mac);

        tfp_snprintf( xPublishParam.pucTopic, xPublishParam.usTopicLength+1, "%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        tfp_snprintf( xPublishParam.pvData, xPublishParam.ulDataLength+1, "{\"value\": \"%02X:%02X:%02X:%02X:%02X:%02X\"}",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
        iot_publish( &xPublishParam );

    }
#endif


    ///////////////////////////////////////////////////////////////////////////////////
    // IP ADDRESS
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_IP, len ) == 0 ) {

        ip_addr_t addr = net_get_ip();

        tfp_snprintf( xPublishParam.pucTopic, xPublishParam.usTopicLength+1, "%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        tfp_snprintf( xPublishParam.pvData, xPublishParam.ulDataLength+1, "{\"value\": \"%s\"}", inet_ntoa(addr) );
        iot_publish( &xPublishParam );

    }


    ///////////////////////////////////////////////////////////////////////////////////
    // SUBNET MASK
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_SUBNET, len ) == 0 ) {

        ip_addr_t addr = net_get_netmask();

        tfp_snprintf( xPublishParam.pucTopic, xPublishParam.usTopicLength+1, "%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        tfp_snprintf( xPublishParam.pvData, xPublishParam.ulDataLength+1, "{\"value\": \"%s\"}", inet_ntoa(addr) );
        iot_publish( &xPublishParam );

    }


    ///////////////////////////////////////////////////////////////////////////////////
    // GATEWAY
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_GATEWAY, len ) == 0 ) {

        ip_addr_t addr = net_get_gateway();

        tfp_snprintf( xPublishParam.pucTopic, xPublishParam.usTopicLength+1, "%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        tfp_snprintf( xPublishParam.pvData, xPublishParam.ulDataLength+1, "{\"value\": \"%s\"}", inet_ntoa(addr) );
        iot_publish( &xPublishParam );

    }


    ///////////////////////////////////////////////////////////////////////////////////
    // UART
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_WRITE_UART, len ) == 0 ) {

        ptr = (char*)pxPublishData->pvData;

        char* data = parse_gpio_str(ptr, "\"value\": ",  '}');
        DEBUG_PRINTF( "%s\r\n", data );

        // Trigger an Email/SMS notification when the UART message received contains a specific phrase!
        if (strstr(data, CONFIG_NOTIFICATION_UART_KEYWORD) != NULL) {
			tfp_snprintf( xPublishParam.pucTopic, xPublishParam.usTopicLength+1, "%s%s/%s", PREPEND_REPLY_TOPIC, IOT_CLIENTCREDENTIAL_CLIENT_ID, API_TRIGGER_NOTIFICATION );
			tfp_snprintf( xPublishParam.pvData, xPublishParam.ulDataLength+1, "{\"recipient\": \"%s\", \"message\": \"%s\"}", CONFIG_NOTIFICATION_RECIPIENT, CONFIG_NOTIFICATION_MESSAGE );
			iot_publish( &xPublishParam );
	        DEBUG_PRINTF( "%s\r\n", topic );
	        DEBUG_PRINTF( "%s\r\n\r\n", payload );
        }

		tfp_snprintf( xPublishParam.pucTopic, xPublishParam.usTopicLength+1, "%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic );
		tfp_snprintf( xPublishParam.pvData, xPublishParam.ulDataLength+1, "{\"value\": \"%s\"}", data );
		iot_publish( &xPublishParam );
        DEBUG_PRINTF( "%s\r\n", topic );
        DEBUG_PRINTF( "%s\r\n\r\n", payload );
    }


    ///////////////////////////////////////////////////////////////////////////////////
    // TRIGGER NOTIFICATIONS
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_TRIGGER_NOTIFICATION, len ) == 0 ) {

        ptr = (char*)pxPublishData->pvData;

        tfp_snprintf( xPublishParam.pucTopic, xPublishParam.usTopicLength+1, "%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic );
        iot_publish( &xPublishParam );

        //DEBUG_PRINTF( "%s\r\n", ptr );
    }
#endif

    return eMQTTTrue;
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

    DEBUG_MINIMAL( "iot_app_task\r\n" );



    while (1) {


        /*
         * Wait until network is ready then display network info
         *
         * */
        DEBUG_MINIMAL( "Waiting for configuration..." );
        int i = 0;
        while ( !net_is_ready() ) {
            vTaskDelay( pdMS_TO_TICKS(1000) );
            DEBUG_MINIMAL( "." );
            if (i++ > 30) {
            	DEBUG_MINIMAL( "Could not recover. Do reboot.\r\n" );
                chip_reboot();
            }
        }
        vTaskDelay( pdMS_TO_TICKS(1000) );
        DEBUG_MINIMAL( "\r\n" );
        display_network_info();

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
			continue;//goto exit;
		}

		iot_subscribe_params_t xSubscribeParam = {
			(uint8_t *)user_generate_subscribe_topic(), IOT_APP_TOPIC_LENGTH-1, 1, NULL, user_subscribe_receive_cb };
		iot_subscribe( &xSubscribeParam );

		do  {
			vTaskDelay( pdMS_TO_TICKS(1000) );
		} while ( net_is_ready() );

		/* Release memory for iot packet */
		vPortFree( ( char* )xSubscribeParam.pucTopic );

exit:
		/* Disconnect from iot broker */
		iot_disconnect();
    }

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
    iot_setup( iot_app_task );
    return 0;
}
