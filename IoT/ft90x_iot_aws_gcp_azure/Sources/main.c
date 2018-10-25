/*
 * ============================================================================
 * History
 * =======
 * 16 Sep 2018 : Created
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

/* netif Abstraction Header. */
#include "net.h"

/* IOT Headers. */
#include "iot/iot.h"



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
static ip_addr_t ip      = IPADDR4_INIT_BYTES( 192, 168, 22, 100 );
static ip_addr_t gateway = IPADDR4_INIT_BYTES( 192, 168, 22, 1 );
static ip_addr_t mask    = IPADDR4_INIT_BYTES( 255, 255, 255, 0 );
static ip_addr_t dns     = IPADDR4_INIT_BYTES( 0, 0, 0, 0 );
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
/* Task configurations. */
#define IOT_APP_TASK_NAME                "iot_task"
#define IOT_APP_TASK_PRIORITY            (2)
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
#define IOT_APP_TASK_STACK_SIZE          (1024 + 64)
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
#define IOT_APP_TASK_STACK_SIZE          (1536)
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
#define IOT_APP_TASK_STACK_SIZE          (1536)
#endif
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
/* IoT application function */
static void iot_app_task(void *pvParameters);
static void iot_app_process(void);

/* MQTT-related abstractions called by iot_app_process */
static inline int mqtt_is_connected( mqtt_client_t *client );
static inline err_t mqtt_connect_async( mqtt_client_t *client,
    const char* broker, uint16_t port, struct mqtt_connect_client_info_t *info );
static void mqtt_connect_callback( mqtt_client_t *client,
    void *arg, mqtt_connection_status_t status);
static void mqtt_pubsub_callback( void *arg, err_t result );

#if USE_MQTT_PUBLISH
static inline err_t mqtt_publish_async( mqtt_client_t *client,
    const char* topic, const char* msg, int msg_len );
static inline int user_generate_publish_topic(
    char* topic, int size, const char* param );
static inline int user_generate_publish_payload(
    char* payload, int size, const char* param );
#endif // USE_MQTT_PUBLISH

#if USE_MQTT_SUBSCRIBE
static inline err_t mqtt_subscribe_async(
    mqtt_client_t *client, const char* topic);
static void mqtt_subscribe_recv_topic(
    void *arg, const char *topic, u32_t tot_len);
static void mqtt_subscribe_recv_payload(
    void *arg, const u8_t *data, u16_t len, u8_t flags);
static inline char* user_generate_subscribe_topic();
#endif // USE_MQTT_SUBSCRIBE
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
        UART_DIVIDER_9600_BAUD,
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
            "Copyright (C) Bridgetek Pte Ltd \r\n"
            "----------------------------------------------------- \r\n"
            "Welcome to IoT Cloud Example... \r\n\r\n"
            "Demonstrate secure IoT connectivity to cloud services \r\n"
            "----------------------------------------------------- \r\n" );

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
    for (;;);
}

static void iot_app_task( void *pvParameters )
{
    (void) pvParameters;

    net_init( ip, gateway, mask, USE_DHCP, dns, NULL, NULL );

    while (1)
    {
        /* Wait for valid IP address */
        DEBUG_PRINTF( "Waiting for configuration..." );
        int i = 0;
        while ( !net_is_ready() ) {
            vTaskDelay( pdMS_TO_TICKS(1000) );
            DEBUG_PRINTF( "." );
            if(++i>30) i=1/0;
        }
        vTaskDelay( pdMS_TO_TICKS(1000) );
        DEBUG_PRINTF( "\r\n" );

        /* Display IP information */
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

        /* MQTT application */
        DEBUG_PRINTF( "Starting...\r\n\r\n" );
        iot_app_process();
    }
}





static inline err_t mqtt_connect_async(
    mqtt_client_t *client,
	const char* broker, uint16_t port,
    struct mqtt_connect_client_info_t *info
    )
{
    err_t err = 0;
    ip_addr_t host_addr = {0};
    struct hostent *host = NULL;


    /* Get IP address given host name */
    do {
        host = gethostbyname( broker );
        if (host == NULL) {
            DEBUG_PRINTF( "gethostbyname failed\r\n" );
            vTaskDelay( pdMS_TO_TICKS(1000) );
            continue;
        }
        break;
    }
    while ( net_is_ready() );

    /* copy the network address to sockaddr_in structure */
    if ( (host->h_addrtype == AF_INET) && (host->h_length == sizeof(ip_addr_t)) ) {
        memcpy( &host_addr, host->h_addr_list[0], sizeof(ip_addr_t) );

        DEBUG_PRINTF( "MQTT CONNECT: %s %s:%d\r\n",
            broker, inet_ntoa(host_addr), port );

        DEBUG_PRINTF( "id: %s\r\nuser: %s\r\npass: %s [%d]\r\n\r\n",
            info->client_id,
            info->client_user ? info->client_user : "NULL",
            info->client_pass ? info->client_pass : "NULL",
            info->client_pass ? strlen(info->client_pass) : 0 );

        err = mqtt_client_connect(
            client, &host_addr, port, mqtt_connect_callback, info->tls_config, info );
        if (err != ERR_OK) {
            DEBUG_PRINTF( "mqtt_client_connect failed! %d\r\n", err );
        }
    }
    else {
        DEBUG_PRINTF( "gethostbyname invalid\r\n" );
        return -1;
    }

    return err;
}

static int mqtt_connect_callback_err = 0;

static void mqtt_connect_callback(
    mqtt_client_t *client,
    void *arg,
    mqtt_connection_status_t status
    )
{
    if ( status == MQTT_CONNECT_ACCEPTED ) {
        DEBUG_PRINTF( "MQTT CONNECTED\r\n" );
        mqtt_connect_callback_err = 0;
    }
    else {
        DEBUG_PRINTF( "mqtt_connect_callback failed! %d\r\n\r\n\r\n", status );
        mqtt_connect_callback_err = 1;
    }
}

static inline int mqtt_is_connected( mqtt_client_t *client )
{
#if 0
    return mqtt_client_is_connected( client ) && net_is_ready();
#else
    // Temporarily remove net_is_ready()
    // net_is_ready() calls netif_is_linkup() which is failing often with Rev B (2a)
    return mqtt_client_is_connected( client );
#endif
}

static void mqtt_pubsub_callback( void *arg, err_t result )
{
    if (result != ERR_OK) {
        DEBUG_PRINTF( "MQTT %s result: %d\r\n", (char*)arg, result );
    }
}



///////////////////////////////////////////////////////////////////////////////////
// IOT SUBSCRIBE
///////////////////////////////////////////////////////////////////////////////////

#if USE_MQTT_SUBSCRIBE

static inline err_t mqtt_subscribe_async(
    mqtt_client_t *client, const char* topic)
{
    err_t err;
    u8_t qos = 1;

    err = mqtt_subscribe(
        client, topic, qos, mqtt_pubsub_callback, "SUBSCRIBE" );
    if ( err != ERR_OK ) {
        DEBUG_PRINTF( "\r\nmqtt_subscribe failed! %d\r\n", err );
    }
    else {
        DEBUG_PRINTF( "\r\nMQTT SUBSCRIBE: %s\r\n\r\n", topic );
        mqtt_set_inpub_callback(
            client, mqtt_subscribe_recv_topic, mqtt_subscribe_recv_payload, NULL );
    }

    return err;
}

static char* subscribe_recv = NULL;
static uint8_t subscribe_recv_size = 0;
static uint8_t subscribe_recv_off = 0;

static void mqtt_subscribe_recv_topic(
    void *arg, const char *topic, u32_t tot_len )
{
    DEBUG_PRINTF( "\r\nMQTT RECEIVE: %s [%d]\r\n", topic, (unsigned int)tot_len );
    subscribe_recv_off = 0;
    subscribe_recv_size = tot_len;
    if ( subscribe_recv ) {
        vPortFree( subscribe_recv );
        subscribe_recv = NULL;
    }
    subscribe_recv = pvPortMalloc( tot_len );
    if ( subscribe_recv ) {
        memset( subscribe_recv, 0, tot_len );
    }
}

static void mqtt_subscribe_recv_payload(
    void *arg, const u8_t *data, u16_t len, u8_t flags )
{
    if ( subscribe_recv ) {
        memcpy( subscribe_recv + subscribe_recv_off, data, len );
        subscribe_recv_size -= len;
        subscribe_recv_off += len;
        if ( subscribe_recv_size == 0 ) {
            DEBUG_PRINTF( "%s\r\n\r\n", subscribe_recv );
            vPortFree( subscribe_recv );
            subscribe_recv = NULL;
        }
    }
}

static inline char* user_generate_subscribe_topic()
{
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    // Lets subscribe to the messages we published
    return "device/#";
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    // Google Cloud does not seem to support MQTT subscribe for telemetry events, only for config
    static char topic[64] = {0};
    tfp_snprintf( topic, sizeof(topic),
        "/devices/%s/config", (char*)iot_getdeviceid() );
    //tfp_snprintf(topic, sizeof(topic),
        "/devices/%s/events", (char*)iot_getdeviceid());
    return topic;
#else
    static char topic[64] = {0};
    tfp_snprintf( topic, sizeof(topic),
        "devices/%s/messages/devicebound/#", (char*)iot_getdeviceid() );
    //tfp_snprintf(topic, sizeof(topic),
        "devices/%s/messages/events/#", (char*)iot_getdeviceid());
    return topic;
#endif
}

#endif // USE_MQTT_SUBSCRIBE



///////////////////////////////////////////////////////////////////////////////////
// IOT PUBLISH
///////////////////////////////////////////////////////////////////////////////////

#if USE_MQTT_PUBLISH
static inline err_t mqtt_publish_async(
    mqtt_client_t *client, const char* topic, const char* msg, int msg_len )
{
    err_t err;
    u8_t retain = 0;
    u8_t qos = 0;

    err = mqtt_publish(
        client, topic, msg, msg_len, qos, retain, mqtt_pubsub_callback, "PUBLISH" );
    if ( err != ERR_OK ) {
        DEBUG_PRINTF( "\r\nmqtt_publish failed! %d\r\n", err );
    }
    else {
        DEBUG_PRINTF( "\r\nMQTT PUBLISH: %s [%d]\r\n%s\r\n", topic, msg_len, msg );
    }

    return err;
}

static inline int user_generate_publish_topic(
    char* topic, int size, const char* param )
{
    // Google Cloud IoT and Microsoft Azure IoT have fixed format for MQTT publish topic
    // Amazon AWS IoT supports any format for MQTT publish topic
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    // Any format - can be modified
    return tfp_snprintf( topic, size, "device/%s/devicePayload", param );
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    // Fixed format - do not modify
    return tfp_snprintf( topic, size, "/devices/%s/events", (char*)iot_getdeviceid() );
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
    // Fixed format - do not modify
    return tfp_snprintf( topic, size, "devices/%s/messages/events/", (char*)iot_getdeviceid() );
#else
    return 0;
#endif
}

static inline int user_generate_publish_payload(
    char* payload, int size, const char* param )
{
    int len = 0;

#if USE_PAYLOAD_TIMESTAMP
#if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    int format = 1; // YYYYMMDDHH:mm:SS
#else
    int format = 0; // YYYY-MM-DDTHH:mm:SS.000
#endif


    len = tfp_snprintf( payload, size,
        "{\r\n"
        " \"deviceId\": \"%s\",\r\n"
        " \"timeStampEpoch\": %llu,\r\n"
        " \"timeStampIso\": \"%s\",\r\n"
        " \"sensorReading\": %d,\r\n"
        " \"batteryCharge\": %d,\r\n"
        " \"batteryDischargeRate\": %d\r\n"
        "}",
        param,
        iot_rtc_get_time_epoch(),
        iot_rtc_get_time_iso(format),
        rand() % 10 + 30,
        rand() % 30 - 10,
        rand() % 5 );
#else
    len = tfp_snprintf( payload, size,
        "{\r\n"
        " \"deviceId\": \"%s\",\r\n"
        " \"sensorReading\": %d,\r\n"
        " \"batteryCharge\": %d,\r\n"
        " \"batteryDischargeRate\": %d\r\n"
        "}",
        param,
        rand() % 10 + 30,
        rand() % 30 - 10,
        rand() % 5 );
#endif

    return len;
}

#endif // USE_MQTT_PUBLISH



//
// Initialize TLS certificates
// Initialize MQTT settings/credentials
// Establish secure MQTT connection via TLS
// Subscribe from a topic
// While connected:
//     Publish sensor data to a topic
// Release resources
//
static void iot_app_process( void )
{
    err_t err = ERR_OK;
    mqtt_client_t mqtt = {0};
    struct mqtt_connect_client_info_t info = {0};
    const uint8_t *ca = NULL;
    const uint8_t *cert = NULL;
    const uint8_t *pkey = NULL;
    size_t ca_len = 0;
    size_t cert_len = 0;
    size_t pkey_len = 0;


    memset( &mqtt, 0, sizeof(mqtt_client_t) );
    memset( &info, 0, sizeof(info) );
    iot_init();


    //
    // Initialize TLS certificates
    //
    iot_getcertificates( &ca, &ca_len, &cert, &cert_len, &pkey, &pkey_len );
    if (!cert) {
        info.tls_config = altcp_tls_create_config_client( ca, ca_len );
    }
    else {
        info.tls_config = altcp_tls_create_config_client_2wayauth(
            ca, ca_len, pkey, pkey_len, NULL, 0, cert, cert_len );
    }
    iot_freecertificates( ca, cert, pkey );
    if ( info.tls_config == NULL ) {
        DEBUG_PRINTF( "altcp_tls_create_config_client failed!\r\n" );
        goto close;
    }


    //
    // Initialize MQTT settings/credentials
    //
    info.client_id = iot_getid();
    info.client_user = iot_getusername();
    info.client_pass = iot_getpassword();


    //
    // Establish secure MQTT connection via TLS
    //
    err = mqtt_connect_async(
        &mqtt, iot_getbrokername(), iot_getbrokerport(), &info );
    if ( err != ERR_OK ) {
        DEBUG_PRINTF( "mqtt_connect_async failed! %d\r\n", err );
        goto close;
    }
    do {
        vTaskDelay( pdMS_TO_TICKS(1000) );
    }
    while ( !mqtt_is_connected( &mqtt ) && !mqtt_connect_callback_err );
    if ( mqtt_connect_callback_err ) {
        mqtt_connect_callback_err = 0;
        goto close;
    }
    vTaskDelay( pdMS_TO_TICKS(1000) );



#if USE_MQTT_SUBSCRIBE
    //
    // Subscribe from a topic
    //
    err = mqtt_subscribe_async( &mqtt, user_generate_subscribe_topic() );
    if (err != ERR_OK) {
        DEBUG_PRINTF("mqtt_subscribe_async failed! %d\r\n", err);
        //goto close;
    }
#endif // USE_MQTT_SUBSCRIBE

#if USE_MQTT_PUBLISH
    //
    // Publish sensor data to a topic
    //
    char *devices[3] = { "hopper", "knuth", "turing" };
    int device_count = sizeof( devices )/sizeof( devices[0] );
    char topic[48] = {0};
    char payload[192] = {0};
    int retries = 0;

    while ( mqtt_is_connected( &mqtt ) && err==ERR_OK )
    {
        // Publish sensor data for each sensor device
        // In this demo, there are 3 sensor device - hopper, knuth, turing
        // In normal scenario, there is usually only 1 sensor device
        for ( int i=0; i<device_count && mqtt_is_connected( &mqtt ) && err==ERR_OK; i++ ) {

            // Generate the publish topic and payload
            // Note that the topic usually does not change
            // but in this example, our topic is changing
            int len = user_generate_publish_topic(
                topic, sizeof(topic), devices[i] );
            len = user_generate_publish_payload(
                payload, sizeof(payload), devices[i] );

            // If publish fails, retry a few times
            // Sometimes it succeeds after retrying 1-3 times
            do {
                err = mqtt_publish_async( &mqtt, topic, payload, len );
                if ( err==ERR_OK ) {
                    retries = 0;
                    vTaskDelay( pdMS_TO_TICKS(750) );
                    break;
                }
                else {
                    retries++;
                    vTaskDelay( pdMS_TO_TICKS(1000) );
                }
            } while ( mqtt_is_connected( &mqtt ) && retries<5 );
        }
    }
#elif USE_MQTT_SUBSCRIBE
    while ( mqtt_is_connected( &mqtt ) ) {
        vTaskDelay( pdMS_TO_TICKS(1000) );
    }
#endif // USE_MQTT_PUBLISH


    //
    // Release resources
    //
close:
    DEBUG_PRINTF( "iot_app_process ended! %d\r\n\r\n\r\n", err );
    mqtt_disconnect( &mqtt );
    altcp_tls_free_config( info.tls_config );
    iot_free();
}

