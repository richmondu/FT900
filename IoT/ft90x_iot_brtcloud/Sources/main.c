/*
 * ============================================================================
 * History
 * =======
 * 18 Jun 2019 : Created
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

#include <ft900.h>
#include "tinyprintf.h"

/* FreeRTOS Headers. */
#include "FreeRTOS.h"
#include "task.h"

/* netif Abstraction Header. */
#include "net.h"

/* IOT Headers. */
#include <iot_config.h>
#include "iot/iot.h"
#include "iot/iot_utils.h"

/* RPC Headers. */
#include "rpc.h"

#include <string.h>
#include <stdlib.h>



#define PREPEND_REPLY_TOPIC "server/"



///////////////////////////////////////////////////////////////////////////////////
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {CRITICAL_SECTION_BEGIN;tfp_printf(__VA_ARGS__);CRITICAL_SECTION_END;} while (0)
#else
#define DEBUG_PRINTF(...)
#endif
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
/* Default network configuration. */
#define USE_DHCP 1       // 1: Dynamic IP, 0: Static IP
static ip_addr_t ip      = IPADDR4_INIT_BYTES( 0, 0, 0, 0 );
static ip_addr_t gateway = IPADDR4_INIT_BYTES( 0, 0, 0, 0 );
static ip_addr_t mask    = IPADDR4_INIT_BYTES( 0, 0, 0, 0 );
static ip_addr_t dns     = IPADDR4_INIT_BYTES( 0, 0, 0, 0 );
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
/* Task configurations. */
#define IOT_APP_TASK_NAME                        "iot_task"
#define IOT_APP_TASK_PRIORITY                    (2)
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    #define IOT_APP_TASK_STACK_SIZE              (512)
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    #define IOT_APP_TASK_STACK_SIZE              (1536 + 32)
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
    #if (MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)
        #define IOT_APP_TASK_STACK_SIZE          (1536 + 16)
    #elif (MAZ_AUTH_TYPE == AUTH_TYPE_X509CERT)
        #define IOT_APP_TASK_STACK_SIZE          (1024 + 64)
    #endif
#else
#define IOT_APP_TASK_STACK_SIZE                  (512)
#endif
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
/* IoT application function */
static void iot_app_task(void *pvParameters);

#if USE_MQTT_SUBSCRIBE
static inline char* user_generate_subscribe_topic();
static void user_subscribe_receive_cb(
    iot_subscribe_rcv* mqtt_subscribe_recv );
#endif // USE_MQTT_SUBSCRIBE
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
static iot_handle g_handle = NULL;
///////////////////////////////////////////////////////////////////////////////////



static void myputc( void* p, char c )
{
    uart_write( (ft900_uart_regs_t*) p, (uint8_t) c );
}

static inline void uart_setup()
{
    /* enable uart */
    sys_enable( sys_device_uart0 );
    gpio_function( 48, pad_func_3 );
    gpio_function( 49, pad_func_3 );

    uart_open(
        UART0, 1,
        UART_DIVIDER_19200_BAUD,
        uart_data_bits_8,
        uart_parity_none,
        uart_stop_bits_1
        );

    /* Enable tfp_printf() functionality... */
    init_printf( UART0, myputc );
}

static inline void ethernet_setup()
{
    net_setup();
}

int main( void )
{
    sys_reset_all();
    interrupt_disable_globally();
    uart_setup();
    ethernet_setup();

    uart_puts( UART0,
            "\x1B[2J" /* ANSI/VT100 - Clear the Screen */
            "\x1B[H" /* ANSI/VT100 - Move Cursor to Home */
            "Copyright (C) Bridgetek Pte Ltd\r\n"
            "-------------------------------------------------------\r\n"
            "Welcome to IoT Device Controller example...\r\n\r\n"
            "Demonstrate remote access of FT900 via Bridgetek IoT Cloud\r\n"
            "-------------------------------------------------------\r\n\r\n" );

    if (xTaskCreate( iot_app_task,
            IOT_APP_TASK_NAME,
            IOT_APP_TASK_STACK_SIZE,
            NULL,
            IOT_APP_TASK_PRIORITY,
            NULL ) != pdTRUE ) {
        DEBUG_PRINTF( "xTaskCreate failed\r\n" );
    }

    vTaskStartScheduler();
    DEBUG_PRINTF( "Should never reach here!\r\n" );
}

static inline void display_network_info()
{
    uint8_t* mac = net_get_mac();
    DEBUG_PRINTF( "MAC=%02X:%02X:%02X:%02X:%02X:%02X\r\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );

    ip_addr_t addr = net_get_ip();
    DEBUG_PRINTF( "IP=%s\r\n", inet_ntoa(addr) );
    addr = net_get_gateway();
    DEBUG_PRINTF( "GW=%s\r\n", inet_ntoa(addr) );
    addr = net_get_netmask();
    DEBUG_PRINTF( "MA=%s\r\n", inet_ntoa(addr) );
    vTaskDelay( pdMS_TO_TICKS(1000) );
}

static void iot_app_task( void *pvParameters )
{
    (void) pvParameters;
    iot_handle handle = NULL;


    /* Initialize network */
    net_init( ip, gateway, mask, USE_DHCP, dns, NULL, NULL );

    iot_utils_init();

    init_rtc();

    while (1) {
        /*
         * Wait until network is ready then display network info
         *
         * */
        DEBUG_PRINTF( "Waiting for configuration..." );
        int i = 0;
        while ( !net_is_ready() ) {
            vTaskDelay( pdMS_TO_TICKS(1000) );
            DEBUG_PRINTF( "." );
            if (i++ > 30) {
                DEBUG_PRINTF( "Could not recover. Do reboot.\r\n" );
                chip_reboot();
            }
        }
        vTaskDelay( pdMS_TO_TICKS(1000) );
        DEBUG_PRINTF( "\r\n" );
        display_network_info();


        /*
         * IoT process
         *
         * */
        DEBUG_PRINTF( "Starting...\r\n\r\n" );

        /* connect to server using TLS certificates and MQTT credentials
         * sample call back functions, iot_utils_getcertificates & iot_utils_getcredentials,
         *     are provided in iot_utils.c to retrieve information from iot_config.h.
         *     These have been tested to work with Amazon AWS, Google Cloud and Microsoft Azure.
         */
        handle = iot_connect(
            iot_utils_getcertificates, iot_utils_getcredentials );
        if ( !handle ) {
            /* make sure to replace the dummy certificates in the Certificates folder */
            DEBUG_PRINTF( "Error! Please check your certificates and credentials.\r\n\r\n" );
            vTaskDelay( pdMS_TO_TICKS(1000) );
            continue;
        }

        /* subscribe and publish from/to server */
#if USE_MQTT_SUBSCRIBE
        char* topic_sub = user_generate_subscribe_topic();

        if ( iot_subscribe( handle, topic_sub, user_subscribe_receive_cb, 1 ) == 0 ) {
            DEBUG_PRINTF( "\r\nSUB: %s\r\n\r\n", topic_sub );
        }
        g_handle = handle;

        do  {
            vTaskDelay( pdMS_TO_TICKS(1000) );
        } while ( net_is_ready() && iot_is_connected( handle ) == 0 );

        iot_unsubscribe( handle, topic_sub );
#endif // USE_MQTT_SUBSCRIBE

        /* disconnect from server */
        iot_disconnect( handle );
    }

    iot_utils_free();
}



///////////////////////////////////////////////////////////////////////////////////
// IOT SUBSCRIBE
///////////////////////////////////////////////////////////////////////////////////

#if USE_MQTT_SUBSCRIBE

static inline char* user_generate_subscribe_topic()
{
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    // Lets subscribe to the messages we published
    return "device/#";
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    // Google Cloud does not seem to support MQTT subscribe for telemetry events, only for config
    static char topic[64] = {0};
    tfp_snprintf( topic, sizeof(topic),
        "/devices/%s/config", (char*)iot_utils_getdeviceid() );
    //tfp_snprintf(topic, sizeof(topic),
    //    "/devices/%s/events", (char*)iot_getdeviceid());
    return topic;
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
    static char topic[64] = {0};
    tfp_snprintf( topic, sizeof(topic),
        "devices/%s/messages/devicebound/#", (char*)iot_utils_getdeviceid() );
    //tfp_snprintf(topic, sizeof(topic),
    //    "devices/%s/messages/events/#", (char*)iot_getdeviceid());
    return topic;
#else
    static char* topic = NULL;
    if ( !topic ) {
        int len = strlen(CUSTOMER_ID) + 1 + strlen((char*)iot_utils_getdeviceid()) + 1 + 1 + 1;
        topic = pvPortMalloc( len );
        if ( topic ) {
            memset( topic, 0, len );
            tfp_snprintf( topic, len, "%s/#", (char*)iot_utils_getdeviceid() );
        }
    }

    return topic;
#endif
}

#endif // USE_MQTT_SUBSCRIBE



///////////////////////////////////////////////////////////////////////////////////
// PROCESS MQTT SUBSCRIBED PACKETS
///////////////////////////////////////////////////////////////////////////////////

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

static void user_subscribe_receive_cb( iot_subscribe_rcv* mqtt_subscribe_recv )
{
    char topic[64] = {0};
    char payload[32] = {0};


    char* ptr = user_generate_subscribe_topic();
    char* ptr2 = strrchr(ptr,  '/');
    int len = ptr2 - ptr;

    if (strncmp(ptr, mqtt_subscribe_recv->topic, len)!=0) {
        return;
    }

    DEBUG_PRINTF( "\r\nRECV: %s [%d]\r\n",
        mqtt_subscribe_recv->topic, (unsigned int)mqtt_subscribe_recv->payload_len );
    DEBUG_PRINTF( "%s\r\n", mqtt_subscribe_recv->payload );

    iot_unsubscribe( g_handle, ptr );

    // get api
    ptr = (char*)mqtt_subscribe_recv->topic + len + 1;
    len = strlen(ptr);



    ///////////////////////////////////////////////////////////////////////////////////
    // GPIO
    ///////////////////////////////////////////////////////////////////////////////////
    if ( strncmp( ptr, API_GET_GPIO, len ) == 0 ) {

        ptr = (char*)mqtt_subscribe_recv->payload;

        uint32_t number = parse_gpio_int(ptr, "\"number\": ", '}');
        //DEBUG_PRINTF( "%d\r\n", number );
        uint32_t value = get_gpio(number);

        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
        tfp_snprintf( payload, sizeof(payload), "{\"number\": %d, \"value\": %d}", (int)number, (int)value );
        iot_publish( g_handle, topic, payload, strlen(payload), 1 );

    }
    else if ( strncmp( ptr, API_SET_GPIO, len ) == 0 ) {

        ptr = (char*)mqtt_subscribe_recv->payload;

        uint32_t value  = parse_gpio_int(ptr, "\"value\": ",  '}');
        uint32_t number = parse_gpio_int(ptr, "\"number\": ", ',');
        //DEBUG_PRINTF( "%d %d\r\n", number, value );
        set_gpio(number, value);

        value = get_gpio(number);
        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
        tfp_snprintf( payload, sizeof(payload), "{\"number\": %d, \"value\": %d}", (int)number, (int)value );
        iot_publish( g_handle, topic, payload, strlen(payload), 1 );

    }


    ///////////////////////////////////////////////////////////////////////////////////
    // RTC
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_RTC, len ) == 0 ) {

        uint32_t value = get_rtc();
        DEBUG_PRINTF( "%d\r\n", (int)value );

        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic);
        tfp_snprintf( payload, sizeof(payload), "{\"value\": %d}", (int)value );
        iot_publish( g_handle, topic, payload, strlen(payload), 1 );

    }
    else if ( strncmp( ptr, API_SET_RTC, len ) == 0 ) {

        ptr = (char*)mqtt_subscribe_recv->payload;

        uint32_t value  = parse_gpio_int(ptr, "\"value\": ",  '}');
        //DEBUG_PRINTF( "%d\r\n", value );
        set_rtc(value);

        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic);
        tfp_snprintf( payload, sizeof(payload), "{\"value\": %d}", (int)value );
        iot_publish( g_handle, topic, payload, strlen(payload), 1 );

    }


    ///////////////////////////////////////////////////////////////////////////////////
    // STATUS
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_STATUS, len ) == 0 ) {

        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic);
        tfp_snprintf( payload, sizeof(payload), "{\"status\": \"%s\"}", API_STATUS_RUNNING);
        iot_publish( g_handle, topic, payload, strlen(payload), 1 );
        DEBUG_PRINTF( "PUB:  %s %s\r\n", topic, payload );
    }
    else if ( strncmp( ptr, API_SET_STATUS, len ) == 0 ) {

        ptr = (char*)mqtt_subscribe_recv->payload;

        char* status = parse_gpio_str(ptr, "\"status\": ",  '}');
        //DEBUG_PRINTF( "%s\r\n", status );

        if ( strncmp( status, API_STATUS_RESTART, strlen(API_STATUS_RESTART)) == 0 ) {
            tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
            tfp_snprintf( payload, sizeof(payload), "{\"status\": \"%s\"}", API_STATUS_RESTARTING );
            iot_publish( g_handle, topic, payload, strlen(payload), 1 );
            xTaskCreate( restart_task, "restart_task", 64, NULL, 3, NULL );
        }

    }


    ///////////////////////////////////////////////////////////////////////////////////
    // MAC ADDRESS
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_MAC, len ) == 0 ) {

        uint8_t mac[6] = {0};
        get_mac(mac);

        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic);
        tfp_snprintf( payload, sizeof(payload), "{\"value\": \"%02X:%02X:%02X:%02X:%02X:%02X\"}",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
        iot_publish( g_handle, topic, payload, strlen(payload), 1 );

    }
#if 0
    else if ( strncmp( ptr, API_SET_MAC, len ) == 0 ) {

        ptr = (char*)mqtt_subscribe_recv->payload;

        uint8_t* mac_str = parse_gpio_str(ptr, "\"value\": ",  '}');
        set_mac(mac_str);
        uint8_t mac[6] = {0};
        get_mac(mac);

        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic);
        tfp_snprintf( payload, sizeof(payload), "{\"value\": \"%02X:%02X:%02X:%02X:%02X:%02X\"}",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
        iot_publish( g_handle, topic, payload, strlen(payload), 1 );

    }
#endif


    ///////////////////////////////////////////////////////////////////////////////////
    // IP ADDRESS
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_IP, len ) == 0 ) {

        ip_addr_t addr = net_get_ip();

        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic);
        tfp_snprintf( payload, sizeof(payload), "{\"value\": \"%s\"}", inet_ntoa(addr) );
        iot_publish( g_handle, topic, payload, strlen(payload), 1 );

    }


    ///////////////////////////////////////////////////////////////////////////////////
    // SUBNET MASK
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_SUBNET, len ) == 0 ) {

        ip_addr_t addr = net_get_netmask();

        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic);
        tfp_snprintf( payload, sizeof(payload), "{\"value\": \"%s\"}", inet_ntoa(addr) );
        iot_publish( g_handle, topic, payload, strlen(payload), 1 );

    }


    ///////////////////////////////////////////////////////////////////////////////////
    // GATEWAY
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_GATEWAY, len ) == 0 ) {

        ip_addr_t addr = net_get_gateway();

        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic);
        tfp_snprintf( payload, sizeof(payload), "{\"value\": \"%s\"}", inet_ntoa(addr) );
        iot_publish( g_handle, topic, payload, strlen(payload), 1 );

    }


    ///////////////////////////////////////////////////////////////////////////////////
    // UART
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_WRITE_UART, len ) == 0 ) {

        ptr = (char*)mqtt_subscribe_recv->payload;

        char* data = parse_gpio_str(ptr, "\"data\": ",  '}');
        //DEBUG_PRINTF( "%s\r\n", status );

         tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
         tfp_snprintf( payload, sizeof(payload), "{\"data\": \"%s\"}", data );
         iot_publish( g_handle, topic, payload, strlen(payload), 1 );

         DEBUG_PRINTF( "%s\r\n", data );
    }


    iot_subscribe( g_handle, user_generate_subscribe_topic(), user_subscribe_receive_cb, 1 );
}

