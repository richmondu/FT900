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

#define MAX_TOPIC_SIZE 80
#define MAX_PAYLOAD_SIZE 128
extern TaskHandle_t g_hSystemTask;

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

iot_publish_params_t xPublishParam = {0};

void notify_task( void *pvContext )
{
	iot_publish_params_t* pxPublishParam = (iot_publish_params_t*)&xPublishParam;


    vTaskDelay( pdMS_TO_TICKS(3000) );
	tfp_printf("notify\r\n");

	memset((char*)pxPublishParam->pucTopic, 0, MAX_TOPIC_SIZE);
	memset((char*)pxPublishParam->pvData, 0, MAX_PAYLOAD_SIZE);
	pxPublishParam->usTopicLength = tfp_snprintf( (char*)pxPublishParam->pucTopic, MAX_TOPIC_SIZE,
			"%s%s/%s", PREPEND_REPLY_TOPIC, IOT_CLIENTCREDENTIAL_CLIENT_ID, API_TRIGGER_NOTIFICATION );
	pxPublishParam->ulDataLength = tfp_snprintf( (char*)pxPublishParam->pvData, MAX_PAYLOAD_SIZE,
			"{\"recipient\": \"%s\", \"message\": \"%s\"}", CONFIG_NOTIFICATION_RECIPIENT, CONFIG_NOTIFICATION_MESSAGE );
	tfp_printf("notify %s\r\n", pxPublishParam->pucTopic);
	xTaskNotify(g_hSystemTask, 0, eNoAction );
	tfp_printf("notify %s\r\n", pxPublishParam->pvData);
}

MQTTBool_t user_subscribe_receive_cb( void * pvContext, const MQTTPublishData_t * const pxPublishData )
{
    //taskENTER_CRITICAL();
	iot_publish_params_t* pxPublishParam = (iot_publish_params_t*)pvContext;


    DEBUG_MINIMAL( "callback [%s] [%s]\r\n", (char*)pxPublishData->pucTopic, (char*)pxPublishData->pvData );


	memset((char*)pxPublishParam->pucTopic, 0, MAX_TOPIC_SIZE);
	memset((char*)pxPublishParam->pvData, 0, MAX_PAYLOAD_SIZE);

    char* ptr = user_generate_subscribe_topic();
    char* ptr2 = strrchr(ptr,  '/');
    int len = ptr2 - ptr;

    if (strncmp(ptr, (char*)pxPublishData->pucTopic, len)!=0) {
        return eMQTTFalse;
    }

    // get api
    ptr = (char*)pxPublishData->pucTopic + len + 1;
    len = strlen(ptr);


    ///////////////////////////////////////////////////////////////////////////////////
    // GPIO
    ///////////////////////////////////////////////////////////////////////////////////
    if ( strncmp( ptr, API_GET_GPIO, len ) == 0 ) {

        ptr = (char*)pxPublishData->pvData;

        uint32_t number = parse_gpio_int(ptr, "\"number\": ", '}');
        uint32_t value = get_gpio(number);

        pxPublishParam->usTopicLength = tfp_snprintf( (char*)pxPublishParam->pucTopic, MAX_TOPIC_SIZE,
        		"%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic );
        pxPublishParam->ulDataLength = tfp_snprintf( (char*)pxPublishParam->pvData, MAX_PAYLOAD_SIZE,
        		"{\"number\": %d, \"value\": %d}", (int)number, (int)value );
        xTaskNotify(g_hSystemTask, 0, eNoAction );
    }
    else if ( strncmp( ptr, API_SET_GPIO, len ) == 0 ) {

        ptr = (char*)pxPublishData->pvData;

        uint32_t value  = parse_gpio_int(ptr, "\"value\": ",  '}');
        uint32_t number = parse_gpio_int(ptr, "\"number\": ", ',');
        set_gpio(number, value);

        value = get_gpio(number);
        pxPublishParam->usTopicLength = tfp_snprintf( (char*)pxPublishParam->pucTopic, MAX_TOPIC_SIZE,
        		"%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic );
        pxPublishParam->ulDataLength = tfp_snprintf( (char*)pxPublishParam->pvData, MAX_PAYLOAD_SIZE,
        		"{\"number\": %d, \"value\": %d}", (int)number, (int)value );
        xTaskNotify(g_hSystemTask, 0, eNoAction );
    }


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

        pxPublishParam->usTopicLength = tfp_snprintf( (char*)pxPublishParam->pucTopic, MAX_TOPIC_SIZE,
        		"%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        pxPublishParam->ulDataLength = tfp_snprintf( (char*)pxPublishParam->pvData, MAX_PAYLOAD_SIZE,
        		"{\"value\": %d}", (int)value );
        xTaskNotify(g_hSystemTask, 0, eNoAction );
    }
    else if ( strncmp( ptr, API_SET_RTC, len ) == 0 ) {

        ptr = (char*)pxPublishData->pvData;

        uint32_t value  = parse_gpio_int(ptr, "\"value\": ",  '}');
        //DEBUG_PRINTF( "%d\r\n", value );
        set_rtc(value);

        pxPublishParam->usTopicLength = tfp_snprintf( (char*)pxPublishParam->pucTopic, MAX_TOPIC_SIZE,
        		"%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        pxPublishParam->ulDataLength = tfp_snprintf( (char*)pxPublishParam->pvData, MAX_PAYLOAD_SIZE,
        		"{\"value\": %d}", (int)value );
        xTaskNotify(g_hSystemTask, 0, eNoAction );
    }
#endif


    ///////////////////////////////////////////////////////////////////////////////////
    // STATUS
    ///////////////////////////////////////////////////////////////////////////////////
    //else
    if ( strncmp( ptr, API_GET_STATUS, len ) == 0 ) {

    	pxPublishParam->usTopicLength = tfp_snprintf( (char*)pxPublishParam->pucTopic, MAX_TOPIC_SIZE,
    			"%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
    	pxPublishParam->ulDataLength = tfp_snprintf( (char*)pxPublishParam->pvData, MAX_PAYLOAD_SIZE,
    			"{\"value\": \"%s\"}", API_STATUS_RUNNING);
    	xTaskNotify(g_hSystemTask, 0, eNoAction );

   		vTaskDelay( pdMS_TO_TICKS(1000) );
    }
    else if ( strncmp( ptr, API_SET_STATUS, len ) == 0 ) {

        ptr = (char*)pxPublishData->pvData;

        char* status = parse_gpio_str(ptr, "\"value\": ",  '}');
        if ( strncmp( status, API_STATUS_RESTART, strlen(API_STATUS_RESTART)) == 0 ) {
        	pxPublishParam->usTopicLength = tfp_snprintf( (char*)pxPublishParam->pucTopic, MAX_TOPIC_SIZE,
        			"%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic );
            pxPublishParam->ulDataLength = tfp_snprintf( (char*)pxPublishParam->pvData, MAX_PAYLOAD_SIZE,
            		"{\"value\": \"%s\"}", API_STATUS_RESTARTING );
            xTaskNotify(g_hSystemTask, 0, eNoAction );
            xTaskCreate( restart_task, "restart_task", 64, NULL, 3, NULL );
        }
    }


    ///////////////////////////////////////////////////////////////////////////////////
    // MAC ADDRESS
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_MAC, len ) == 0 ) {

        uint8_t mac[6] = {0};
        get_mac(mac);

        pxPublishParam->usTopicLength = tfp_snprintf( (char*)pxPublishParam->pucTopic, MAX_TOPIC_SIZE,
        		"%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        pxPublishParam->ulDataLength = tfp_snprintf( (char*)pxPublishParam->pvData, MAX_PAYLOAD_SIZE,
        		"{\"value\": \"%02X:%02X:%02X:%02X:%02X:%02X\"}", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
        xTaskNotify(g_hSystemTask, 0, eNoAction );
    }
#if 0
    else if ( strncmp( ptr, API_SET_MAC, len ) == 0 ) {

        ptr = (char*)pxPublishData->pvData;

        uint8_t* mac_str = parse_gpio_str(ptr, "\"value\": ",  '}');
        set_mac(mac_str);
        uint8_t mac[6] = {0};
        get_mac(mac);

        pxPublishParam->usTopicLength = tfp_snprintf( (char*)pxPublishParam->pucTopic, MAX_TOPIC_SIZE,
        		"%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        pxPublishParam->ulDataLength = tfp_snprintf( (char*)pxPublishParam->pvData, MAX_PAYLOAD_SIZE,
        		"{\"value\": \"%02X:%02X:%02X:%02X:%02X:%02X\"}",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
        xTaskNotify(g_hSystemTask, 0, eNoAction );

    }
#endif


    ///////////////////////////////////////////////////////////////////////////////////
    // IP ADDRESS
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_IP, len ) == 0 ) {

        ip_addr_t addr = net_get_ip();

        pxPublishParam->usTopicLength = tfp_snprintf( (char*)pxPublishParam->pucTopic, MAX_TOPIC_SIZE,
        		"%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        pxPublishParam->ulDataLength = tfp_snprintf( (char*)pxPublishParam->pvData, MAX_PAYLOAD_SIZE,
        		"{\"value\": \"%s\"}", inet_ntoa(addr) );
        xTaskNotify(g_hSystemTask, 0, eNoAction );
    }


    ///////////////////////////////////////////////////////////////////////////////////
    // SUBNET MASK
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_SUBNET, len ) == 0 ) {

        ip_addr_t addr = net_get_netmask();

        pxPublishParam->usTopicLength = tfp_snprintf( (char*)pxPublishParam->pucTopic, MAX_TOPIC_SIZE,
        		"%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        pxPublishParam->ulDataLength = tfp_snprintf( (char*)pxPublishParam->pvData, MAX_PAYLOAD_SIZE,
        		"{\"value\": \"%s\"}", inet_ntoa(addr) );
        xTaskNotify(g_hSystemTask, 0, eNoAction );
    }


    ///////////////////////////////////////////////////////////////////////////////////
    // GATEWAY
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_GATEWAY, len ) == 0 ) {

        ip_addr_t addr = net_get_gateway();

        pxPublishParam->usTopicLength = tfp_snprintf( (char*)pxPublishParam->pucTopic, MAX_TOPIC_SIZE,
        		"%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic);
        pxPublishParam->ulDataLength = tfp_snprintf( (char*)pxPublishParam->pvData, MAX_PAYLOAD_SIZE,
        		"{\"value\": \"%s\"}", inet_ntoa(addr) );
        xTaskNotify(g_hSystemTask, 0, eNoAction );
    }


    ///////////////////////////////////////////////////////////////////////////////////
    // UART
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_WRITE_UART, len ) == 0 ) {

        ptr = (char*)pxPublishData->pvData;

        char* data = parse_gpio_str(ptr, "\"value\": ",  '}');
        DEBUG_PRINTF( "%s\r\n", data );


        pxPublishParam->usTopicLength = tfp_snprintf( (char*)pxPublishParam->pucTopic, MAX_TOPIC_SIZE,
        		"%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic );
        pxPublishParam->ulDataLength = tfp_snprintf( (char*)pxPublishParam->pvData, MAX_PAYLOAD_SIZE,
        		"{\"value\": \"%s\"}", data );
        xTaskNotify(g_hSystemTask, 0, eNoAction );

        // Trigger an Email/SMS notification when the UART message received contains a specific phrase!
        if (strstr(data, CONFIG_NOTIFICATION_UART_KEYWORD) != NULL) {
        	xTaskCreate( notify_task, "notify_task", 256, NULL, 3, NULL );
        }
    }


    ///////////////////////////////////////////////////////////////////////////////////
    // TRIGGER NOTIFICATIONS
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_TRIGGER_NOTIFICATION, len ) == 0 ) {

        ptr = (char*)pxPublishData->pvData;

        pxPublishParam->usTopicLength = tfp_snprintf( (char*)pxPublishParam->pucTopic, MAX_TOPIC_SIZE,
        		"%s%s", PREPEND_REPLY_TOPIC, (char*)pxPublishData->pucTopic );
        pxPublishParam->ulDataLength = tfp_snprintf( (char*)pxPublishParam->pvData, MAX_PAYLOAD_SIZE, "%s", ptr );
        xTaskNotify(g_hSystemTask, 0, eNoAction );
    }


    taskEXIT_CRITICAL();
    //DEBUG_MINIMAL( "exit\r\n" );
    return eMQTTFalse;
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
    iot_subscribe_params_t xSubscribeParam = {0};
    char topic[MAX_TOPIC_SIZE] = {0};
    char payload[MAX_PAYLOAD_SIZE] = {0};
	/* Default network configuration. */
	#define USE_DHCP 1       // 1: Dynamic IP, 0: Static IP
	ip_addr_t ip      = IPADDR4_INIT_BYTES( 0, 0, 0, 0 );
	ip_addr_t gateway = IPADDR4_INIT_BYTES( 0, 0, 0, 0 );
	ip_addr_t mask    = IPADDR4_INIT_BYTES( 0, 0, 0, 0 );
	ip_addr_t dns     = IPADDR4_INIT_BYTES( 0, 0, 0, 0 );


    /* Initialize network */
    net_init( ip, gateway, mask, USE_DHCP, dns, NULL, NULL );

    xPublishParam.pucTopic = topic;
    xPublishParam.usTopicLength = sizeof(topic)-1;
    xPublishParam.xQoS = 1;
    xPublishParam.pvData = payload;
    xPublishParam.ulDataLength = sizeof(payload)-1;

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
			goto exit;
		}

		xSubscribeParam.pucTopic = (uint8_t *)user_generate_subscribe_topic();
		xSubscribeParam.usTopicLength = IOT_APP_TOPIC_LENGTH-1;
		xSubscribeParam.xQoS = 1;
		xSubscribeParam.pvPublishCallbackContext = &xPublishParam;
		xSubscribeParam.pxPublishCallback = user_subscribe_receive_cb;

		/*
		 * Subscribe messages from iot broker
		 */
		if (iot_subscribe( &xSubscribeParam ) != pdPASS ) {
			DEBUG_MINIMAL( "IOT APP subscribe fail!\r\n" );
			goto exit;
		}

		do  {
			if (xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(1000))) {
				DEBUG_MINIMAL( "iot_publish\r\n" );
				if (iot_publish( &xPublishParam ) != pdPASS ) {
					DEBUG_MINIMAL( "IOT APP publish fail!\r\n" );
				}
				DEBUG_MINIMAL( "iot_publish ok\r\n" );
			}
			vTaskDelay( pdMS_TO_TICKS(1000) );
		} while ( net_is_ready() );
		DEBUG_MINIMAL( "Loop exited!\r\n" );

		/*
		 * Unsubscribe messages from iot broker
		 */
		if (iot_unsubscribe( &xSubscribeParam ) != pdPASS ) {
			DEBUG_MINIMAL( "IOT APP unsubscribe fail!\r\n" );
			goto exit;
		}

exit:
		/* Disconnect from iot broker */
		iot_disconnect();
    }


	/* Release memory for iot packet */
	vPortFree( ( char* )xSubscribeParam.pucTopic );
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
