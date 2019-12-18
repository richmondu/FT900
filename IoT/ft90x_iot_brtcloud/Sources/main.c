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

/* IoT Modem */
#include "iot_modem.h"
#include "json.h"


#include <string.h>
#include <stdlib.h>



#define ENABLE_USECASE_NEW 0



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
#define IOT_APP_TASK_STACK_SIZE                  (768)
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

TaskHandle_t g_iot_app_handle;
iot_handle g_handle = NULL;
static uint8_t g_exit = 0;
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////


uint32_t g_ulDeviceStatus = DEVICE_STATUS_RUNNING;

//
// UART
//

#if ENABLE_UART

static UART_PROPERTIES g_oUartProperties = {
    7, // points to UART_DIVIDER_19200_BAUD
    uart_parity_none,
    uart_flow_none,
    uart_stop_bits_1,
    uart_data_bits_8
};
static uint8_t g_ucUartEnabled = 1;
#endif // ENABLE_UART


//
// GPIO
//

#if ENABLE_GPIO
GPIO_PROPERTIES g_oGpioProperties[GPIO_COUNT] = { {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0} };
uint8_t g_ucGpioEnabled[GPIO_COUNT] = {0, 0, 0, 0};
static uint8_t g_ucGpioStatus[GPIO_COUNT] = {1, 1, 1, 1};    // ["Low", "High"]
static uint8_t g_ucGpioVoltage = 0;                 // ["3.3 V", "5 V"]
#endif // ENABLE_GPIO


//
// I2C
//

#if ENABLE_I2C
uint8_t g_ucI2cEnabled[4] = {1, 1, 1, 1};
#endif // ENABLE_I2C

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

    iot_modem_uart_enable(&g_oUartProperties, 1, 0);

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
    iot_modem_uart_enable_interrupt();
    iot_modem_gpio_init(g_ucGpioVoltage);
    iot_modem_gpio_enable_interrupt();
    interrupt_enable_globally();

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
            &g_iot_app_handle ) != pdTRUE ) {
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

    /* Initialize IoT library */
    iot_init();
    iot_utils_init();

    /* Initialize rtc */
    // MM900Ev1b (RevC) has an internal RTC
    // IoTBoard does not have internal RTC
    // When using IoTBoard, this must be disabled to prevent crash
    // TODO: support external RTC to work on IoTBoard
#if 0 // TEMPORARILY DISABLED FOR THE NEW FT900 IOT BOARD
    init_rtc();
#endif


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

        DEBUG_PRINTF( "Device is now ready! Control this device from IoT Portal https://%s\r\n\r\n", MQTT_BROKER );

        /* set device status to running */
        g_ulDeviceStatus = DEVICE_STATUS_RUNNING;

        /* display the UART commands */
        iot_modem_uart_command_help();

        do  {
            uint32_t ulNotificationValue = 0;
            if (xTaskNotifyWait(0, TASK_NOTIFY_CLEAR_BITS, &ulNotificationValue, pdMS_TO_TICKS(1000)) == pdTRUE) {
            	//DEBUG_PRINTF( "xTaskNotifyWait %d\r\n", ulNotificationValue );

#if ENABLE_UART_ATCOMMANDS
            	/* process UART */
            	if (TASK_NOTIFY_FROM_UART(ulNotificationValue)) {
            		iot_modem_uart_command_process();
            	}
#endif // ENABLE_UART_ATCOMMANDS

#if ENABLE_GPIO
            	/* process GPIO */
            	for (i=0; i<GPIO_COUNT; i++) {
					if (TASK_NOTIFY_FROM_GPIO(ulNotificationValue, i)) {
						iot_modem_gpio_process(i+1);
					}
            	}
#endif // ENABLE_GPIO

#if ENABLE_I2C
            	/* process I2C */
            	for (i=0; i<4; i++) {
					if (TASK_NOTIFY_FROM_I2C(ulNotificationValue, i)) {
						// TODO
					}
            	}
#endif // ENABLE_I2C

            }
//            vTaskDelay( pdMS_TO_TICKS(1000) );
        } while ( net_is_ready() && iot_is_connected( handle ) == 0 && !g_exit );

        g_exit = 0;
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
        int len = strlen((char*)iot_utils_getdeviceid()) + 1 + 1 + 1;
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



void restart_task( void *param )
{
    vTaskDelay( pdMS_TO_TICKS(1000) );
    chip_reboot();
}

///////////////////////////////////////////////////////////////////////////////////
// PROCESS MQTT SUBSCRIBED PACKETS
///////////////////////////////////////////////////////////////////////////////////

#define IS_API(api) (strncmp( ptr, api, len ) == 0)

static void user_subscribe_receive_cb( iot_subscribe_rcv* mqtt_subscribe_recv )
{
    char topic[MQTT_MAX_TOPIC_SIZE] = {0};
    char payload[MQTT_MAX_PAYLOAD_SIZE] = {0};
    int ret = 0;


    //DEBUG_PRINTF( "\r\nRECV: %s [%d]\r\n", mqtt_subscribe_recv->topic, (unsigned int)mqtt_subscribe_recv->payload_len );
    //DEBUG_PRINTF( "%s [%d]\r\n", mqtt_subscribe_recv->payload, strlen(mqtt_subscribe_recv->payload) );

    // get api
    char* ptr = strrchr(mqtt_subscribe_recv->topic, '/') + 1;
    int len = strlen(ptr);


    ///////////////////////////////////////////////////////////////////////////////////
    // STATUS
    ///////////////////////////////////////////////////////////////////////////////////
    if ( IS_API(API_GET_STATUS) ) {
        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic);
        tfp_snprintf( payload, sizeof(payload), PAYLOAD_API_GET_STATUS, g_ulDeviceStatus, VERSION_MAJOR, VERSION_MINOR);
        ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
        //DEBUG_PRINTF( "PUB:  %s %s\r\n", topic, payload );
    }
    else if ( IS_API(API_SET_STATUS) ) {
        uint32_t ulDeviceStatus = json_parse_int(mqtt_subscribe_recv->payload, "status");
        switch (ulDeviceStatus) {
            case DEVICE_STATUS_RESTART: {
                g_ulDeviceStatus = DEVICE_STATUS_RESTARTING;
                tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
                tfp_snprintf( payload, sizeof(payload), PAYLOAD_API_SET_STATUS, g_ulDeviceStatus );
                ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
                //DEBUG_PRINTF( "PUB:  %s %s\r\n", topic, payload );
                xTaskCreate( restart_task, "restart_task", 64, NULL, 3, NULL );
                //DEBUG_PRINTF( "DEVICE_STATUS_RESTARTING\r\n" );
                break;
            }
            case DEVICE_STATUS_STOP: {
                if (g_ulDeviceStatus != DEVICE_STATUS_STOPPING && g_ulDeviceStatus != DEVICE_STATUS_STOPPED) {
                    g_ulDeviceStatus = DEVICE_STATUS_STOPPING;
                    tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
                    tfp_snprintf( payload, sizeof(payload), PAYLOAD_API_SET_STATUS, g_ulDeviceStatus );
                    ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
                    //DEBUG_PRINTF( "PUB:  %s %s\r\n", topic, payload );
                    // TODO
                    g_ulDeviceStatus = DEVICE_STATUS_STOPPED;
                    DEBUG_PRINTF( "STOPPED\r\n" );
                    break;
                }
            }
            case DEVICE_STATUS_START: {
                if (g_ulDeviceStatus != DEVICE_STATUS_STARTING && g_ulDeviceStatus != DEVICE_STATUS_RUNNING) {
                    g_ulDeviceStatus = DEVICE_STATUS_STARTING;
                    tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
                    tfp_snprintf( payload, sizeof(payload), PAYLOAD_API_SET_STATUS, g_ulDeviceStatus );
                    ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
                    //DEBUG_PRINTF( "PUB:  %s %s\r\n", topic, payload );
                    // TODO
                    g_ulDeviceStatus = DEVICE_STATUS_RUNNING;
                    DEBUG_PRINTF( "RUNNING\r\n" );
                    break;
                }
            }
            default: {
                tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
                tfp_snprintf( payload, sizeof(payload), PAYLOAD_API_SET_STATUS, g_ulDeviceStatus );
                ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
                break;
            }
        }
    }


#if ENABLE_UART
    ///////////////////////////////////////////////////////////////////////////////////
    // UART
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( IS_API(API_GET_UARTS) ) {
        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
        tfp_snprintf( payload, sizeof(payload), PAYLOAD_API_GET_UARTS, g_ucUartEnabled);
        ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
        //DEBUG_PRINTF( "PUB:  %s %s\r\n", topic, payload );
    }
    else if ( IS_API(API_GET_UART_PROPERTIES) ) {
        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
        tfp_snprintf( payload, sizeof(payload), PAYLOAD_API_GET_UART_PROPERTIES,
            g_oUartProperties.m_ucBaudrate,
            g_oUartProperties.m_ucParity,
            // uart_flow_xon_xoff is the max value but uart_flow_dtr_dsr is not exposed
            g_oUartProperties.m_ucFlowcontrol == uart_flow_xon_xoff ? uart_flow_dtr_dsr : g_oUartProperties.m_ucFlowcontrol,
               // uart_stop_bits_2 is the max value but uart_stop_bits_1_5 is not exposed
            g_oUartProperties.m_ucStopbits == uart_stop_bits_2 ? uart_stop_bits_1_5 : g_oUartProperties.m_ucStopbits,
               // only uart_data_bits_7 and uart_data_bits_8 are exposed
            g_oUartProperties.m_ucDatabits - uart_data_bits_7 // subtract offset
            );
        ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
        //DEBUG_PRINTF( "PUB:  %s %s\r\n", topic, payload );
    }
    else if ( IS_API(API_SET_UART_PROPERTIES) ) {
        // get the parameter values
        uint8_t ucDatabits    = (uint8_t)json_parse_int(mqtt_subscribe_recv->payload, "databits");
        uint8_t ucStopbits    = (uint8_t)json_parse_int(mqtt_subscribe_recv->payload, "stopbits");
        uint8_t ucFlowcontrol = (uint8_t)json_parse_int(mqtt_subscribe_recv->payload, "flowcontrol");
        uint8_t ucParity      = (uint8_t)json_parse_int(mqtt_subscribe_recv->payload, "parity");
        uint8_t ucBaudrate    = (uint8_t)json_parse_int(mqtt_subscribe_recv->payload, "baudrate");
        //DEBUG_PRINTF( "RCV ucBaudrate=%d ucParity=%d ucFlowcontrol=%d, ucStopbits=%d, ucDatabits=%d\r\n",
        //    ucBaudrate, ucParity, ucFlowcontrol, ucStopbits, ucDatabits );

        // baudrate index should be valid
        if (ucBaudrate < UART_PROPERTIES_BAUDRATE_COUNT) {
            g_oUartProperties.m_ucBaudrate = ucBaudrate;
        }
        // uart_parity_even is the max value, ergo use <=
        if (ucParity <= uart_parity_even) {
            g_oUartProperties.m_ucParity = ucParity;
        }
        // uart_flow_xon_xoff is the max value but uart_flow_dtr_dsr is not exposed, ergo use <
        if (ucFlowcontrol < uart_flow_xon_xoff) {
            if (ucFlowcontrol == uart_flow_dtr_dsr) {
                g_oUartProperties.m_ucFlowcontrol = uart_flow_xon_xoff;
            }
            else {
                g_oUartProperties.m_ucFlowcontrol = ucFlowcontrol;
            }
        }
        // uart_stop_bits_2 is the max value but uart_stop_bits_1_5 is not exposed, ergo use <
        if (ucStopbits < uart_stop_bits_2) {
            if (ucStopbits == uart_stop_bits_1_5) {
                g_oUartProperties.m_ucStopbits = uart_stop_bits_2;
            }
            else {
                g_oUartProperties.m_ucStopbits = ucStopbits;
            }
        }
        // only uart_data_bits_7 and uart_data_bits_8 are exposed
        if (ucDatabits < 2) {
            g_oUartProperties.m_ucDatabits = ucDatabits + uart_data_bits_7; // add offset
        }

        //DEBUG_PRINTF( "UPD ucBaudrate=%d ucParity=%d ucFlowcontrol=%d, ucStopbits=%d, ucDatabits=%d\r\n",
        //        g_oUartProperties.m_ucBaudrate,
        //        g_oUartProperties.m_ucParity,
        //        g_oUartProperties.m_ucFlowcontrol,
        //        g_oUartProperties.m_ucStopbits,
        //        g_oUartProperties.m_ucDatabits );
        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );

        tfp_snprintf( payload, sizeof(payload), PAYLOAD_EMPTY );
        ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
        //DEBUG_PRINTF( "PUB:  %s %s\r\n", topic, payload );

        // configure UART with the new values, uart_soft_reset is needed to avoid distorted text when changing databits or parity
        iot_modem_uart_enable(&g_oUartProperties, 1, 1);
    }
    else if ( IS_API(API_ENABLE_UART) ) {
        uint8_t ucEnabled = (uint8_t)json_parse_int(mqtt_subscribe_recv->payload, "enable");
        //DEBUG_PRINTF( "ucEnabled=%d\r\n", ucEnabled );

        if ( g_ucUartEnabled != ucEnabled ) {
            if (ucEnabled == 0) {
                // Disable UART by closing the UART
                iot_modem_uart_enable(&g_oUartProperties, 0, 1);
            }
            else {
                // Enable UART by opening the UART
                iot_modem_uart_enable(&g_oUartProperties, 1, 0);
            }
            g_ucUartEnabled = ucEnabled;
        }
        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
        tfp_snprintf( payload, sizeof(payload), PAYLOAD_EMPTY );
        ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
        //DEBUG_PRINTF( "PUB:  %s %s\r\n", topic, payload );
    }
#endif // ENABLE_UART



#if ENABLE_GPIO
    ///////////////////////////////////////////////////////////////////////////////////
    // GPIO
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( IS_API(API_GET_GPIOS) ) {
        // TODO: make sure g_ucGpioDirection is updated
        iot_modem_gpio_get_status(g_ucGpioStatus, &g_oGpioProperties, g_ucGpioEnabled);

        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
        tfp_snprintf( payload, sizeof(payload), PAYLOAD_API_GET_GPIOS, g_ucGpioVoltage,
            g_ucGpioEnabled[0], g_oGpioProperties[0].m_ucDirection, g_ucGpioStatus[0],
            g_ucGpioEnabled[1], g_oGpioProperties[1].m_ucDirection, g_ucGpioStatus[1],
            g_ucGpioEnabled[2], g_oGpioProperties[2].m_ucDirection, g_ucGpioStatus[2],
            g_ucGpioEnabled[3], g_oGpioProperties[3].m_ucDirection, g_ucGpioStatus[3]
            );
        ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
        DEBUG_PRINTF( "PUB:  %s[%d] %s[%d]\r\n", topic, strlen(topic), payload, strlen(payload));
    }
    else if ( IS_API(API_GET_GPIO_PROPERTIES) ) {
        uint8_t ucNumber = (uint8_t)json_parse_int(mqtt_subscribe_recv->payload, "number") - 1;
        DEBUG_PRINTF( "ucNumber=%d\r\n", ucNumber );
        if (ucNumber < GPIO_COUNT) {
            tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
            tfp_snprintf( payload, sizeof(payload), PAYLOAD_API_GET_GPIO_PROPERTIES,
                g_oGpioProperties[ucNumber].m_ucDirection,
                g_oGpioProperties[ucNumber].m_ucMode,
                g_oGpioProperties[ucNumber].m_ucAlert,
                g_oGpioProperties[ucNumber].m_ulAlertperiod,
                g_oGpioProperties[ucNumber].m_ucPolarity,
                g_oGpioProperties[ucNumber].m_ulWidth,
                g_oGpioProperties[ucNumber].m_ulMark,
                g_oGpioProperties[ucNumber].m_ulSpace
                );
            ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
            DEBUG_PRINTF( "PUB:  %s[%d] %s[%d]\r\n", topic, strlen(topic), payload, strlen(payload));
        }
    }
    else if ( IS_API(API_SET_GPIO_PROPERTIES) ) {
        uint8_t  ucNumber      = (uint8_t) json_parse_int(mqtt_subscribe_recv->payload, "number") - 1;
        uint32_t ulSpace       = (uint32_t)json_parse_int(mqtt_subscribe_recv->payload, "space");
        uint32_t ulMark        = (uint32_t)json_parse_int(mqtt_subscribe_recv->payload, "mark");
        uint32_t ulWidth       = (uint32_t)json_parse_int(mqtt_subscribe_recv->payload, "width");
        uint8_t  ucPolarity    = (uint8_t) json_parse_int(mqtt_subscribe_recv->payload, "polarity");
        uint32_t ulAlertperiod = (uint32_t)json_parse_int(mqtt_subscribe_recv->payload, "alertperiod");
        uint8_t  ucAlert       = (uint8_t) json_parse_int(mqtt_subscribe_recv->payload, "alert");
        uint8_t  ucMode        = (uint8_t) json_parse_int(mqtt_subscribe_recv->payload, "mode");
        uint8_t  ucDirection   = (uint8_t) json_parse_int(mqtt_subscribe_recv->payload, "direction");
        DEBUG_PRINTF( "ucNumber=%d ucDirection=%d ucMode=%d, ucAlert=%d, ulAlertperiod=%d ucPolarity=%d ulWidth=%d ulMark=%d ulSpace=%d\r\n",
            ucNumber, ucDirection, ucMode, ucAlert, ulAlertperiod, ucPolarity, ulWidth, ulMark, ulSpace );

        if (ucNumber < GPIO_COUNT) {
            g_oGpioProperties[ucNumber].m_ucDirection   = ucDirection;
            g_oGpioProperties[ucNumber].m_ucMode        = ucMode;
            g_oGpioProperties[ucNumber].m_ucAlert       = ucAlert;
            g_oGpioProperties[ucNumber].m_ulAlertperiod = ulAlertperiod;
            g_oGpioProperties[ucNumber].m_ucPolarity    = ucPolarity;
            g_oGpioProperties[ucNumber].m_ulWidth       = ulWidth;
            g_oGpioProperties[ucNumber].m_ulMark        = ulMark;
            g_oGpioProperties[ucNumber].m_ulSpace       = ulSpace;

            // When user sets the configuration, it will be disabled by default
            // User has to explicitly enable it
            g_ucGpioEnabled[ucNumber] = 0;
        }
        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
        tfp_snprintf( payload, sizeof(payload), PAYLOAD_EMPTY );
        ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
    }
    else if ( IS_API(API_ENABLE_GPIO) ) {
        uint8_t ucNumber = (uint8_t)json_parse_int(mqtt_subscribe_recv->payload, "number") - 1;
        uint8_t ucEnabled = (uint8_t)json_parse_int(mqtt_subscribe_recv->payload, "enable");
        if (ucNumber < GPIO_COUNT && ucEnabled < 2) {
            if ( g_ucGpioEnabled[ucNumber] != ucEnabled ) {
            	// order matters
            	if (ucEnabled) {
                    if (iot_modem_gpio_enable(g_oGpioProperties, (int)ucNumber, (int)ucEnabled)) {
                    	g_ucGpioEnabled[ucNumber] = ucEnabled;
                    }
            	}
            	else {
            		g_ucGpioEnabled[ucNumber] = ucEnabled;
                    iot_modem_gpio_enable(g_oGpioProperties, (int)ucNumber, (int)ucEnabled);
            	}
            }
        }
        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
        tfp_snprintf( payload, sizeof(payload), PAYLOAD_EMPTY );
        ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
    }
    else if ( IS_API(API_GET_GPIO_VOLTAGE) ) {
        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
        tfp_snprintf( payload, sizeof(payload), PAYLOAD_API_GET_GPIO_VOLTAGE, g_ucGpioVoltage);
        ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
    }
    else if ( IS_API(API_SET_GPIO_VOLTAGE) ) {
        uint8_t ucVoltage = (uint8_t)json_parse_int(mqtt_subscribe_recv->payload, "voltage");
        if (ucVoltage < 2) {
            if ( g_ucGpioVoltage != ucVoltage ) {
                iot_modem_gpio_set_voltage(ucVoltage);
                g_ucGpioVoltage = ucVoltage;
            }
        }
        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
        tfp_snprintf( payload, sizeof(payload), PAYLOAD_EMPTY );
        ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
    }
#endif // ENABLE_GPIO



#if ENABLE_I2C
    ///////////////////////////////////////////////////////////////////////////////////
    // I2C
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( IS_API(API_GET_I2CS) ) {
        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
        tfp_snprintf( payload, sizeof(payload), PAYLOAD_API_GET_I2CS,
            g_ucI2cEnabled[0], g_ucI2cEnabled[1], g_ucI2cEnabled[2], g_ucI2cEnabled[3]
            );
        ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
    }
    else if ( IS_API(API_GET_I2C_DEVICE_PROPERTIES) ) {
        DEBUG_PRINTF( "NOT YET SUPPORTED\r\n" );
    }
    else if ( IS_API(API_SET_I2C_DEVICE_PROPERTIES) ) {
        DEBUG_PRINTF( "NOT YET SUPPORTED\r\n" );
    }
    else if ( IS_API(API_ENABLE_I2C) ) {
        ptr = (char*)mqtt_subscribe_recv->payload;

        DEBUG_PRINTF( "%s\r\n", ptr );
        // note: python dict maintains insertion order so number will always be the last key
        uint8_t ucNumber = (uint8_t)json_parse_int(ptr, "number") - 1;
        uint8_t ucEnabled = (uint8_t)json_parse_int(ptr, "enable");
        DEBUG_PRINTF( "ucEnabled=%d ucNumber=%d\r\n", ucEnabled, ucNumber );

        if (ucNumber < 4 && ucEnabled < 2) {
            if ( g_ucI2cEnabled[ucNumber] != ucEnabled ) {
                if (ucEnabled == 0) {
                    // TODO: disable
                }
                else {
                    // TODO: enable
                }
                g_ucI2cEnabled[ucNumber] = ucEnabled;
            }
            tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
            tfp_snprintf( payload, sizeof(payload), PAYLOAD_EMPTY );
            ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
        }
    }
#endif // ENABLE_I2C



#if ENABLE_NOTIFICATIONS
    ///////////////////////////////////////////////////////////////////////////////////
    // NOTIFICATIONS
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( IS_API(API_TRIGGER_NOTIFICATION) ) {
        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
        tfp_snprintf( payload, sizeof(payload), "%s", mqtt_subscribe_recv->payload );
        ret = iot_publish( g_handle, topic, payload, strlen(payload), 1 );
    }
    else if ( IS_API(API_RECEIVE_NOTIFICATION) ) {
        int iParamLen = 0;
        char* pcParam = NULL;

        pcParam = json_parse_str(mqtt_subscribe_recv->payload, "message", &iParamLen);
        char message[UART_ATCOMMAND_MAX_MESSAGE_SIZE] = {0};
        strncpy(message, pcParam, iParamLen);

        pcParam = json_parse_str(mqtt_subscribe_recv->payload, "sender", &iParamLen);
        char sender[16+1] = {0};
        strncpy(sender, pcParam, iParamLen);

        DEBUG_PRINTF( "From %s:\r\n", sender );
        DEBUG_PRINTF( "%s\r\n\r\n", message );
    }
    else if ( IS_API(API_STATUS_NOTIFICATION) ) {
        int iParamLen = 0;
        char* pcParam = NULL;

        pcParam = json_parse_str(mqtt_subscribe_recv->payload, "status", &iParamLen);
        char status[UART_ATCOMMAND_MAX_STATUS_SIZE] = {0};
        strncpy(status, pcParam, iParamLen);
        DEBUG_PRINTF( "\r\n%s\r\n\r\n", status );
    }
#endif // ENABLE_NOTIFICATIONS



    else {
        DEBUG_PRINTF( "UNKNOWN\r\n" );
    }

    if (ret < 0) {
        g_exit = 1;
    }
}

