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



#define ENABLE_USECASE_NEW 0
#define ENABLE_USECASE_OLD 1



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
static TaskHandle_t iot_app_handle;

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



///////////////////////////////////////////////////////////////////////////////////
#define PREPEND_REPLY_TOPIC "server/"

#if ENABLE_USECASE_OLD
#define CONFIG_NOTIFICATION_UART_KEYWORD "Hello World"
#define CONFIG_NOTIFICATION_RECIPIENT "richmond.umagat@brtchip.com"
#define CONFIG_NOTIFICATION_MESSAGE "Hi, How are you today?"
#endif ENABLE_USECASE_OLD
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


#if ENABLE_USECASE_NEW
//////////////////////////////////////////////////////////////////////////
// Sending Notification. STAND-ALONE Use-case (No web/mobile app. All done via UART.)
//////////////////////////////////////////////////////////////////////////
//
// Test #1: send by specifying message and recipient everytime - email, SMS or DeviceID
// iot send message "hello world" recipient "richmond.umagat@gmail.com"
// iot send message "hello world" recipient "+639175900612"
// iot send message "hello world 83f58e3f7e284892f6da7b020e5c982a6ea16dee" recipient "1fbc6613eb4013eca32524d2f3646f786da9bbf9"
//
//////////////////////////////////////////////////////////////////////////
//
// Test #2: set message and recipient once, then send anytime
// iot set message "hello world" recipient "richmond.umagat@gmail.com"
// iot send
// ...
// iot send
//
//////////////////////////////////////////////////////////////////////////
//
// Test #3: set recipient once, then send message anytime
// iot set recipient "richmond.umagat@gmail.com"
// iot send message "hi, world"
// ...
// iot send message "hello, world"
//
//////////////////////////////////////////////////////////////////////////
//
// Test #4: set recipient and message once, enable gpio interrupt once, then toggle gpio anytime
// iot set message "hello world" recipient "richmond.umagat@gmail.com"
// iot gpio 18 enable dir 1 pull 1 edge 0
//
// iot gpio 18 write 1
// iot gpio 18 write 0 // edge falling
// ...
// iot gpio 18 write 1
// iot gpio 18 write 0 // edge falling
//
// iot gpio 18 disable
//
//////////////////////////////////////////////////////////////////////////

#define COMMAND_IOT                 "iot"
#define COMMAND_SET                 "set"
#define COMMAND_SEND                "send"
#define COMMAND_RECIPIENT           "recipient"
#define COMMAND_MESSAGE             "message"
#define COMMAND_QUOTATION           '\"'
#define COMMAND_SPACE               ' '
#define COMMAND_TERMINATOR          '\0'

#define COMMAND_ACTION_UNKNOWN      0
#define COMMAND_ACTION_SET          1
#define COMMAND_ACTION_SEND         2
#define COMMAND_ACTION_GPIO_ENABLE  3
#define COMMAND_ACTION_GPIO_DISABLE 4
#define COMMAND_ACTION_GPIO_WRITE   5

#define COMMAND_GPIO                "gpio"
#define COMMAND_GPIO_ENABLE         "enable"
#define COMMAND_GPIO_DISABLE        "disable"
#define COMMAND_GPIO_WRITE          "write"
#define COMMAND_GPIO_DIR            "dir"
#define COMMAND_GPIO_PULL           "pull"
#define COMMAND_GPIO_EDGE           "edge"


static uint8_t g_aucCommand[128] = {0};
static uint8_t g_ucOffset = 0;
static uint8_t g_ucAvailable = 1;
static uint8_t g_ucGPIOpin = 0;


static inline void command_reset()
{
    memset(g_aucCommand, 0, sizeof(g_aucCommand));
    g_ucOffset = 0;
}

static inline int command_parse_gpio(char *ptr)
{
    int action = COMMAND_ACTION_UNKNOWN;
    int pin = 0;
    int dir = 1;
    int pull = 1;
    int edge = 0;
    int state = 0;

    char* end = ptr;
    while (*end != COMMAND_SPACE) {
        end++;
    }
    pin = strtol(ptr, &end, 10);
    ptr += end-ptr+1;

    while (ptr) {
        if (strncmp(ptr, COMMAND_GPIO_ENABLE, strlen(COMMAND_GPIO_ENABLE)) == 0) {
            ptr += strlen(COMMAND_GPIO_ENABLE) + 1;
            action = COMMAND_ACTION_GPIO_ENABLE;
        }
        else if (strncmp(ptr, COMMAND_GPIO_DISABLE, strlen(COMMAND_GPIO_DISABLE)) == 0) {
            ptr += strlen(COMMAND_GPIO_DISABLE) + 1;
            action = COMMAND_ACTION_GPIO_DISABLE;
        }
        else if (strncmp(ptr, COMMAND_GPIO_WRITE, strlen(COMMAND_GPIO_WRITE)) == 0) {
            ptr += strlen(COMMAND_GPIO_WRITE) + 1;
            action = COMMAND_ACTION_GPIO_WRITE;

            end = ptr;
            while (*end != COMMAND_TERMINATOR) {
                end++;
            }
            state = strtol(ptr, &end, 10);
            //tfp_printf("write %d\r\n", state);
            ptr += end-ptr+1;
        }
        else if (strncmp(ptr, COMMAND_GPIO_DIR, strlen(COMMAND_GPIO_DIR)) == 0) {
            ptr += strlen(COMMAND_GPIO_DIR) + 1;

            end = ptr;
            while (*end != COMMAND_TERMINATOR && *end != COMMAND_SPACE) {
                end++;
            }
            dir = strtol(ptr, &end, 10);
            //tfp_printf("dir %d\r\n", dir);
            ptr += end-ptr+1;
        }
        else if (strncmp(ptr, COMMAND_GPIO_PULL, strlen(COMMAND_GPIO_PULL)) == 0) {
            ptr += strlen(COMMAND_GPIO_PULL) + 1;

            end = ptr;
            while (*end != COMMAND_TERMINATOR && *end != COMMAND_SPACE) {
                end++;
            }
            pull = strtol(ptr, &end, 10);
            //tfp_printf("pull %d\r\n", pull);
            ptr += end-ptr+1;
        }
        else if (strncmp(ptr, COMMAND_GPIO_EDGE, strlen(COMMAND_GPIO_EDGE)) == 0) {
            ptr += strlen(COMMAND_GPIO_EDGE) + 1;

            end = ptr;
            while (*end != COMMAND_TERMINATOR && *end != COMMAND_SPACE) {
                end++;
            }
            edge = strtol(ptr, &end, 10);
            //tfp_printf("edge %d\r\n", edge);
            ptr += end-ptr+1;
        }
        else {
            if (*ptr == 0x00) {
                break;
            }
            ptr++;
        }
    }

    if (action == COMMAND_ACTION_GPIO_ENABLE) {
        //tfp_printf("gpio %d enable dir %d pull %d edge 0\r\n\r\n", pin, dir, pull, edge);
        gpio_dir(pin, dir);
        gpio_pull(pin, pull);
        gpio_interrupt_enable(pin, edge);
        g_ucGPIOpin = pin;
    }
    else if (action == COMMAND_ACTION_GPIO_DISABLE) {
        //tfp_printf("gpio %d disable \r\n\r\n", pin);
        gpio_interrupt_disable(pin);
        g_ucGPIOpin = 0;
    }
    else if (action == COMMAND_ACTION_GPIO_WRITE) {
        //tfp_printf("gpio %d write %d\r\n\r\n", pin, state);
        //gpio_write(pin, !state);
        gpio_write(pin, state);
    }

    return action;
}

static inline int command_parse(char* pcRecipient, int lRecipientSize, char* pcMessage, int lMessageSize)
{
    char *ptr = g_aucCommand;
    int action = COMMAND_ACTION_UNKNOWN;

    if (strncmp(ptr, COMMAND_IOT, strlen(COMMAND_IOT)) != 0) {
        return COMMAND_ACTION_UNKNOWN;
    }
    ptr += strlen(COMMAND_IOT) + 1;

    if (strncmp(ptr, COMMAND_GPIO, strlen(COMMAND_GPIO)) == 0) {
        ptr += strlen(COMMAND_GPIO) + 1;
        return command_parse_gpio(ptr);
    }

    while (ptr) {
        if (strncmp(ptr, COMMAND_SET, strlen(COMMAND_SET)) == 0) {
            ptr += strlen(COMMAND_SET) + 1;
            action = COMMAND_ACTION_SET;
        }
        else if (strncmp(ptr, COMMAND_SEND, strlen(COMMAND_SEND)) == 0) {
            ptr += strlen(COMMAND_SEND) + 1;
            action = COMMAND_ACTION_SEND;
        }
        else if (strncmp(ptr, COMMAND_RECIPIENT, strlen(COMMAND_RECIPIENT)) == 0) {
            ptr += strlen(COMMAND_RECIPIENT) + 1;
            if (*ptr != COMMAND_QUOTATION) {
                return COMMAND_ACTION_UNKNOWN;
            }
            ptr++;
            char* end = ptr;
            while (*end != COMMAND_QUOTATION) {
                end++;
            }
            int len = end-ptr;
            if (len > lRecipientSize-1) {
                len = lRecipientSize-1;
            }
            memcpy(pcRecipient, ptr, len);
            pcRecipient[len] = '\0';
            ptr += len+1+1;
        }
        else if (strncmp(ptr, COMMAND_MESSAGE, strlen(COMMAND_MESSAGE)) == 0) {
            ptr += strlen(COMMAND_MESSAGE) + 1;
            if (*ptr != COMMAND_QUOTATION) {
                return COMMAND_ACTION_UNKNOWN;
            }
            ptr++;
            char* end = ptr;
            while (*end != COMMAND_QUOTATION) {
                end++;
            }
            int len = end-ptr;
            if (len > lMessageSize-1) {
                len = lMessageSize-1;
            }
            memcpy(pcMessage, ptr, len);
            pcMessage[len] = '\0';
            ptr += len+1+1;
        }
        else {
            if (*ptr == 0x00) {
                break;
            }
            ptr++;
        }
    }

    return action;
}

static inline void command_process()
{
    static char recipient[64] = {0};
    static char message[64] = {0};

    g_ucAvailable = 0;

    DEBUG_PRINTF("command: %s [%d]\r\n", g_aucCommand, g_ucOffset);
    if (g_ucOffset == 0) {
        // gpio isr seems to have a different copy of global variable g_aucCommand
        tfp_snprintf(g_aucCommand, sizeof(g_aucCommand), "iot send");
        g_ucOffset = strlen(g_aucCommand);
    }
    int action = command_parse(recipient, sizeof(recipient), message, sizeof(message));
    if (action == 2) {
        //tfp_printf("%d %s %s\r\n", action, recipient, message);
        char topic[80] = {0};
        char payload[128] = {0};
        tfp_snprintf( topic, sizeof(topic), "%s%s/%s", PREPEND_REPLY_TOPIC, (char*)iot_utils_getdeviceid(), API_TRIGGER_NOTIFICATION );
        tfp_snprintf( payload, sizeof(payload), "{\"recipient\": \"%s\", \"message\": \"%s\"}", recipient, message );
        iot_publish( g_handle, topic, payload, strlen(payload), 1 );
        DEBUG_PRINTF("publish: %s %s\r\n\r\n", topic, payload);
    }

    command_reset();

    g_ucAvailable = 1;
}

void uart0_isr()
{
    static uint8_t c;

    if (uart_is_interrupted(UART0, uart_interrupt_rx))
    {
        if (g_ucAvailable == 0) {
            return;
        }

        // read input from UART and store to array
        uart_read(UART0, &c);
        g_aucCommand[g_ucOffset++] = c;
        uart_write(UART0, c);

        // check if command exceeds buffer
        if (g_ucOffset == sizeof(g_aucCommand)) {
            DEBUG_PRINTF("\r\nCommand should be less than %d bytes\r\n", sizeof(g_aucCommand));
            command_reset();
            return;
        }

        // process the command when enter is pressed
        if (c == 0x0D) {
            g_aucCommand[g_ucOffset-1] = '\0'; // Remove the enter key
            g_ucOffset--;
            xTaskNotifyFromISR(iot_app_handle, 0, eNoAction, NULL);
        }
    }
}

static inline void uart_interrupt_setup()
{
    interrupt_attach(interrupt_uart0, (uint8_t) interrupt_uart0, uart0_isr);
    uart_enable_interrupt(UART0, uart_interrupt_rx);
    uart_enable_interrupts_globally(UART0);
}




void gpio_isr()
{
    if (!g_ucGPIOpin) {
        return;
    }

    if (gpio_is_interrupted(g_ucGPIOpin))
    {
        tfp_snprintf(g_aucCommand, sizeof(g_aucCommand), "iot send"); // Remove the enter key
        g_ucOffset = strlen(g_aucCommand);
        //tfp_printf("g_aucCommand=%s g_ucOffset=%d\r\n", g_aucCommand, g_ucOffset);

        xTaskNotifyFromISR(iot_app_handle, 0, eNoAction, NULL);
    }
}

static inline void gpio_interrupt_setup()
{
    interrupt_attach(interrupt_gpio, (uint8_t)interrupt_gpio, gpio_isr);
}

#endif // ENABLE_USECASE_NEW



int main( void )
{
    sys_reset_all();
    interrupt_disable_globally();
    uart_setup();
    ethernet_setup();
#if ENABLE_USECASE_NEW
    uart_interrupt_setup();
    gpio_interrupt_setup();
#endif // ENABLE_USECASE_NEW
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
            &iot_app_handle ) != pdTRUE ) {
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

        do  {
#if ENABLE_USECASE_NEW
            uint32_t ulNotificationValue = 0;
            if (xTaskNotifyWait(0, 0, &ulNotificationValue, pdMS_TO_TICKS(1000)) == pdTRUE) {
                command_process();
            }
#endif // ENABLE_USECASE_NEW
#if ENABLE_USECASE_OLD
            vTaskDelay( pdMS_TO_TICKS(1000) );
#endif // ENABLE_USECASE_OLD

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
    char topic[80] = {0};
    char payload[128] = {0};


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
    // STATUS
    ///////////////////////////////////////////////////////////////////////////////////
    if ( strncmp( ptr, API_GET_STATUS, len ) == 0 ) {

        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic);
        tfp_snprintf( payload, sizeof(payload), "{\"value\": \"%s\"}", API_STATUS_RUNNING);
        iot_publish( g_handle, topic, payload, strlen(payload), 1 );
        DEBUG_PRINTF( "PUB:  %s %s\r\n", topic, payload );
    }
    else if ( strncmp( ptr, API_SET_STATUS, len ) == 0 ) {

        ptr = (char*)mqtt_subscribe_recv->payload;

        char* status = parse_gpio_str(ptr, "\"value\": ",  '}');
        //DEBUG_PRINTF( "%s\r\n", status );

        if ( strncmp( status, API_STATUS_RESTART, strlen(API_STATUS_RESTART)) == 0 ) {
            tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
            tfp_snprintf( payload, sizeof(payload), "{\"value\": \"%s\"}", API_STATUS_RESTARTING );
            iot_publish( g_handle, topic, payload, strlen(payload), 1 );
            xTaskCreate( restart_task, "restart_task", 64, NULL, 3, NULL );
        }
    }


    ///////////////////////////////////////////////////////////////////////////////////
    // TRIGGER NOTIFICATIONS
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_TRIGGER_NOTIFICATION, len ) == 0 ) {

        ptr = (char*)mqtt_subscribe_recv->payload;

        char* recipient = parse_gpio_str(ptr, "\"recipient\": ",  '}');

        if ( strncmp( recipient, iot_utils_getdeviceid(), strlen(iot_utils_getdeviceid())) == 0 ) {
            DEBUG_PRINTF( "%s\r\n", ptr );
        }
        else {
            tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
            iot_publish( g_handle, topic, ptr, strlen(ptr), 1 );
        }

        //DEBUG_PRINTF( "%s\r\n", ptr );
    }

#if ENABLE_USECASE_OLD
    ///////////////////////////////////////////////////////////////////////////////////
    // GPIO
    ///////////////////////////////////////////////////////////////////////////////////
    else if ( strncmp( ptr, API_GET_GPIO, len ) == 0 ) {

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
#endif


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

        char* data = parse_gpio_str(ptr, "\"value\": ",  '}');
        DEBUG_PRINTF( "%s\r\n", data );

        // Trigger an Email/SMS notification when the UART message received contains a specific phrase!
        if (strstr(data, CONFIG_NOTIFICATION_UART_KEYWORD) != NULL) {
            tfp_snprintf( topic, sizeof(topic), "%s%s/%s", PREPEND_REPLY_TOPIC, (char*)iot_utils_getdeviceid(), API_TRIGGER_NOTIFICATION );
            tfp_snprintf( payload, sizeof(payload), "{\"recipient\": \"%s\", \"message\": \"%s\"}", CONFIG_NOTIFICATION_RECIPIENT, CONFIG_NOTIFICATION_MESSAGE );
            iot_publish( g_handle, topic, payload, strlen(payload), 1 );
            DEBUG_PRINTF( "%s\r\n", topic );
            DEBUG_PRINTF( "%s\r\n\r\n", payload );
        }

        tfp_snprintf( topic, sizeof(topic), "%s%s", PREPEND_REPLY_TOPIC, mqtt_subscribe_recv->topic );
        tfp_snprintf( payload, sizeof(payload), "{\"value\": \"%s\"}", data );
        iot_publish( g_handle, topic, payload, strlen(payload), 1 );
        DEBUG_PRINTF( "%s\r\n", topic );
        DEBUG_PRINTF( "%s\r\n\r\n", payload );
    }
#endif // ENABLE_USECASE_OLD


    iot_subscribe( g_handle, user_generate_subscribe_topic(), user_subscribe_receive_cb, 1 );
}

