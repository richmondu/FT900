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
static ip_addr_t ip      = IPADDR4_INIT_BYTES(0, 0, 0, 0);
static ip_addr_t gateway = IPADDR4_INIT_BYTES(0, 0, 0, 0);
static ip_addr_t mask    = IPADDR4_INIT_BYTES(0, 0, 0, 0);
static ip_addr_t dns     = IPADDR4_INIT_BYTES(0, 0, 0, 0);
static char hostname[]   = "FT9xx";
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
#define IOT_APP_MODE_PUBLISH   1

#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
#define IOT_APP_MODE_SUBSCRIBE 0 // Disabled by default; tested working
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
#define IOT_APP_MODE_SUBSCRIBE 0 // Disabled by default; tested working for /config not /events
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
#define IOT_APP_MODE_SUBSCRIBE 0 // Not yet tested
#endif
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
/* Task configurations. */
#define IOT_APP_TASK_NAME                "iot_task"
#define IOT_APP_TASK_PRIORITY            (2)
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
#define IOT_APP_TASK_STACK_SIZE          (1024)
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
static inline int mqtt_is_connected(mqtt_client_t *client);
static inline err_t mqtt_connect_async(mqtt_client_t *client, struct altcp_tls_config *config);
static void mqtt_connect_callback(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
static void mqtt_pubsub_callback(void *arg, err_t result);

#if IOT_APP_MODE_PUBLISH
static inline err_t mqtt_publish_async(mqtt_client_t *client, const char* topic, const char* msg, int msg_len);
static inline int user_generate_publish_topic(char* topic, int size, const char* param);
static inline int user_generate_publish_payload(char* payload, int size, const char* param);
#endif // IOT_APP_MODE_PUBLISH

#if IOT_APP_MODE_SUBSCRIBE
static inline err_t mqtt_subscribe_async(mqtt_client_t *client, const char* topic);
static void mqtt_subscribe_recv_topic(void *arg, const char *topic, u32_t tot_len);
static void mqtt_subscribe_recv_payload(void *arg, const u8_t *data, u16_t len, u8_t flags);
static inline char* user_generate_subscribe_topic();
#endif // IOT_APP_MODE_SUBSCRIBE
///////////////////////////////////////////////////////////////////////////////////





static void myputc(void* p, char c)
{
    uart_write((ft900_uart_regs_t*) p, (uint8_t) c);
}

static inline void uart_setup()
{
    /* enable uart */
    sys_enable(sys_device_uart0);
    gpio_function(48, pad_func_3);
    gpio_function(49, pad_func_3);

    uart_open(UART0, 1,
            UART_DIVIDER_115200_BAUD, uart_data_bits_8, uart_parity_none,
            uart_stop_bits_1);
    /* Enable tfp_printf() functionality... */
    init_printf(UART0, myputc);
}

static inline void ethernet_setup()
{
    /* Set up Ethernet */
    sys_enable(sys_device_ethernet);

#ifdef NET_USE_EEPROM
    /* Set up I2C */
    sys_enable(sys_device_i2c_master);

    /* Setup I2C channel 0 pins */
    /* Use sys_i2c_swop(0) to activate. */
    gpio_function(44, pad_i2c0_scl); /* I2C0_SCL */
    gpio_function(45, pad_i2c0_sda); /* I2C0_SDA */

    /* Setup I2C channel 1 pins for EEPROM */
    /* Use sys_i2c_swop(1) to activate. */
    gpio_function(46, pad_i2c1_scl); /* I2C1_SCL */
    gpio_function(47, pad_i2c1_sda); /* I2C1_SDA */
#endif
}

int main(void)
{
    sys_reset_all();
    interrupt_disable_globally();
    uart_setup();
    ethernet_setup();

    uart_puts(UART0,
            "\x1B[2J" /* ANSI/VT100 - Clear the Screen */
            "\x1B[H" /* ANSI/VT100 - Move Cursor to Home */
            "Copyright (C) Bridgetek Pte Ltd \r\n"
            "--------------------------------------------------------------------- \r\n"
            "Welcome to IoT Example... \r\n\r\n"
            "Demonstrate secure IoT connectivity to IoT cloud services \r\n"
            "--------------------------------------------------------------------- \r\n");

    if (xTaskCreate(iot_app_task,
            IOT_APP_TASK_NAME,
            IOT_APP_TASK_STACK_SIZE,
            NULL,
            IOT_APP_TASK_PRIORITY,
            NULL) != pdTRUE) {
        DEBUG_PRINTF("xTaskCreate failed\r\n");
    }

    vTaskStartScheduler();
    DEBUG_PRINTF("Should never reach here!\r\n");

    for (;;)
        ;
}

static void iot_app_task(void *pvParameters)
{
    (void) pvParameters;

    DEBUG_PRINTF("Task %s started.\r\n", __FUNCTION__);

    net_init(ip, gateway, mask, USE_DHCP, dns, hostname, NULL);

    while (1)
    {
        /* Wait for valid IP address */
        DEBUG_PRINTF("Waiting for configuration...");
        while (!net_is_ready()) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            DEBUG_PRINTF(".");
        }
        DEBUG_PRINTF("\r\n");

        /* Display IP information */
        uint8_t* mac = net_get_mac();
        DEBUG_PRINTF("MAC=%02X:%02X:%02X:%02X:%02X:%02X\r\n",
            mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
        ip_addr_t addr = net_get_ip();
        DEBUG_PRINTF("IP=%s\r\n", inet_ntoa(addr));
        addr = net_get_gateway();
        DEBUG_PRINTF("GW=%s\r\n", inet_ntoa(addr));
        addr = net_get_netmask();
        DEBUG_PRINTF("MA=%s\r\n", inet_ntoa(addr));

        /* MQTT application */
        vTaskDelay(pdMS_TO_TICKS(1000));
        DEBUG_PRINTF("Starting...\r\n\r\n");
        iot_app_process();
    }

    DEBUG_PRINTF("Task %s finished.\r\n", __FUNCTION__);
}





static inline err_t mqtt_connect_async(mqtt_client_t *client, struct altcp_tls_config *config)
{
    struct mqtt_connect_client_info_t ci = {0};
    err_t err = 0;
    ip_addr_t host_addr = {0};
    struct hostent *host = NULL;


    /* Get IP address given host name */
    do {
        host = gethostbyname(iot_getbrokername());
        if (host == NULL)
        {
            DEBUG_PRINTF("MQTT CONNECT: gethostbyname failed\r\n");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        break;
    }
    while (net_is_ready());

    memset(&ci, 0, sizeof(ci));
    ci.tls_config = config;
    ci.client_id = iot_getid();
    ci.client_user = iot_getusername();
    ci.client_pass = iot_getpassword();

    /* copy the network address to sockaddr_in structure */
    if ((host->h_addrtype == AF_INET) && (host->h_length == sizeof(ip_addr_t)))
    {
        memcpy(&host_addr, host->h_addr_list[0], sizeof(ip_addr_t));

        DEBUG_PRINTF("MQTT CONNECT: %s %s:%d\r\n",
                iot_getbrokername(),
                inet_ntoa(host_addr),
                iot_getbrokerport());

        err = mqtt_client_connect(client, &host_addr, iot_getbrokerport(), mqtt_connect_callback, (void *)config, &ci);
        if (err != ERR_OK)
        {
            DEBUG_PRINTF("MQTT CONNECT: mqtt_client_connect failed! err=%d\r\n", err);
        }
    }
    else
    {
        DEBUG_PRINTF("MQTT CONNECT: gethostbyname returned invalid data\r\n");
        return -1;
    }

    return err;
}

static void mqtt_connect_callback(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    static int lExponentialBackoff = 2;

    if (status == MQTT_CONNECT_ACCEPTED)
    {
        DEBUG_PRINTF("MQTT CONNECTED\r\n");
    }
    else
    {
        DEBUG_PRINTF("MQTT CONNECT FAILED! mqtt_connect_callback result: %d\r\n\r\n\r\n", status);

        vTaskDelay(pdMS_TO_TICKS(lExponentialBackoff*1000));
        lExponentialBackoff = (lExponentialBackoff>32) ? 1 : lExponentialBackoff * 2;

        /* Try again to connect. */
        mqtt_connect_async(client, (struct altcp_tls_config *)arg);
    }
}

static inline int mqtt_is_connected(mqtt_client_t *client)
{
    return mqtt_client_is_connected(client) && net_is_ready();
}

static void mqtt_pubsub_callback(void *arg, err_t result)
{
    if (result != ERR_OK)
    {
        DEBUG_PRINTF("MQTT %s result: %d\r\n", (char*)arg, result);
    }
}



///////////////////////////////////////////////////////////////////////////////////
// IOT PUBLISH
///////////////////////////////////////////////////////////////////////////////////

#if IOT_APP_MODE_PUBLISH
static inline err_t mqtt_publish_async(mqtt_client_t *client, const char* topic, const char* msg, int msg_len)
{
    err_t err;
    u8_t retain = 0;
    u8_t qos = 0;

    err = mqtt_publish(client, topic, msg, msg_len, qos, retain, mqtt_pubsub_callback, "PUBLISH");
    if (err != ERR_OK)
    {
        DEBUG_PRINTF("\r\nMQTT PUBLISH: mqtt_publish failed! err=%d\r\n", err);
    }
    else
    {
        DEBUG_PRINTF("\r\nMQTT PUBLISH: %s [%d]\r\n%s\r\n", topic, msg_len, msg);
    }

    return err;
}

static inline int user_generate_publish_topic(char* topic, int size, const char* param)
{
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    return tfp_snprintf(topic, size, "device/%s/devicePayload", param);
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    return tfp_snprintf(topic, size, "/devices/%s/events", (char*)iot_getdeviceid());
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
    return tfp_snprintf(topic, size, "/devices/%s/messages/events", (char*)iot_getdeviceid());
#else
    return 0;
#endif
}

static inline int user_generate_publish_payload(char* payload, int size, const char* param)
{
    int len = 0;
#if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    int format = 1; // YYYYMMDDHH:mm:SS
#else
    int format = 0; // YYYY-MM-DDTHH:mm:SS.000
#endif


#if USE_PAYLOAD_TIMESTAMP
    len = tfp_snprintf(payload, size,
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
        rand() % 5);
#else
    len = tfp_snprintf(payload, size,
        "{\r\n"
        " \"deviceId\": \"%s\",\r\n"
        " \"sensorReading\": %d,\r\n"
        " \"batteryCharge\": %d,\r\n"
        " \"batteryDischargeRate\": %d\r\n"
        "}",
        param,
        rand() % 10 + 30,
        rand() % 30 - 10,
        rand() % 5);
#endif

    return len;
}

#endif // IOT_APP_MODE_PUBLISH



///////////////////////////////////////////////////////////////////////////////////
// IOT SUBSCRIBE
///////////////////////////////////////////////////////////////////////////////////

#if IOT_APP_MODE_SUBSCRIBE
static inline err_t mqtt_subscribe_async(mqtt_client_t *client, const char* topic)
{
    err_t err;
    u8_t qos = 1;

    err = mqtt_subscribe(client, topic, qos, mqtt_pubsub_callback, "SUBSCRIBE");
    if (err != ERR_OK)
    {
        DEBUG_PRINTF("\r\nMQTT SUBSCRIBE: mqtt_subscribe failed! err=%d\r\n", err);
    }
    else
    {
        DEBUG_PRINTF("\r\nMQTT SUBSCRIBE: %s\r\n\r\n", topic);
        mqtt_set_inpub_callback(client, mqtt_subscribe_recv_topic, mqtt_subscribe_recv_payload, NULL);
    }

    return err;
}

static char subscribe_recv[128] = {0};
static uint8_t subscribe_recv_size = 0;
static uint8_t subscribe_recv_off = 0;
static uint8_t subscribe_recv_process = 0;

static void mqtt_subscribe_recv_topic(void *arg, const char *topic, u32_t tot_len)
{
    DEBUG_PRINTF("\r\nMQTT RECEIVE: %s [%d]\r\n", topic, (unsigned int)tot_len);
    memset(subscribe_recv, 0, sizeof(subscribe_recv));
    subscribe_recv_off = 0;
    subscribe_recv_size = tot_len;
    subscribe_recv_process = (tot_len < sizeof(subscribe_recv)) ? 1 : 0;
}

static void mqtt_subscribe_recv_payload(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    if (subscribe_recv_process) {
        memcpy(subscribe_recv + subscribe_recv_off, data, len);
        subscribe_recv_size -= len;
        subscribe_recv_off += len;
        if (subscribe_recv_size == 0) {
            DEBUG_PRINTF("%s\r\n\r\n", subscribe_recv);
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
    tfp_snprintf(topic, sizeof(topic), "/devices/%s/config", (char*)iot_getdeviceid());
    //tfp_snprintf(topic, sizeof(topic), "/devices/%s/events", (char*)iot_getdeviceid());
    return topic;
#else
    return NULL;
#endif
}

#endif // IOT_APP_MODE_SUBSCRIBE





static void iot_app_process(void)
{
    err_t err = ERR_OK;
    mqtt_client_t mqtt = {0};
    struct altcp_tls_config *config = NULL;
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    const uint8_t *ca = NULL;
    const uint8_t *cert = NULL;
    const uint8_t *pkey = NULL;
    size_t ca_len = 0;
    size_t cert_len = 0;
    size_t pkey_len = 0;
#endif

    iot_init();


    //
    // Initialize certificates
    //

#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    // Amazon AWS IoT requires certificate and private key but ca is optional (but recommended)
    ca = iot_certificate_getca(&ca_len);
    cert = iot_certificate_getcert(&cert_len);
    pkey = iot_certificate_getpkey(&pkey_len);
    config = altcp_tls_create_config_client_2wayauth(ca, ca_len, pkey, pkey_len, NULL, 0, cert, cert_len);
    vPortFree((uint8_t *)cert);
    vPortFree((uint8_t *)pkey);
    vPortFree((uint8_t *)ca);
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
    // Google GCP IoT requires JWT token (created with private key) as MQTT password; no certificate needs to be sent
    config = altcp_tls_create_config_client_2wayauth(NULL, 0, NULL, 0, NULL, 0, NULL, 0);
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
    // Microsoft Azure IoT requires SAS token (created with shared access key) as MQTT password; no certificates need to be sent
    config = altcp_tls_create_config_client_2wayauth(NULL, 0, NULL, 0, NULL, 0, NULL, 0);
#endif
    if (config == NULL)
    {
        DEBUG_PRINTF("MQTT CERTIFICATE error\r\n");
        goto close;
    }


    //
    // Establish secure MQTT connection
    //

    memset(&mqtt, 0, sizeof(mqtt_client_t));
    err = mqtt_connect_async(&mqtt, config);
    if (err != ERR_OK)
    {
        DEBUG_PRINTF("MQTT CONNECT: mqtt_connect_async failed! err=%d\r\n", err);
        goto close;
    }
    do
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    while (!mqtt_is_connected(&mqtt));
    vTaskDelay(pdMS_TO_TICKS(1000));


#if IOT_APP_MODE_SUBSCRIBE
    //
    // Subscribe from MQTT server
    //

    err = mqtt_subscribe_async(&mqtt, user_generate_subscribe_topic());
    if (err != ERR_OK)
    {
        DEBUG_PRINTF("MQTT CONNECT: mqtt_subscribe_async failed! err=%d\r\n", err);
        //goto close;
    }
#endif // IOT_APP_MODE_SUBSCRIBE


#if IOT_APP_MODE_PUBLISH
    //
    // Publish sensor data to MQTT server
    //

    char *devices[3] = {"hopper", "knuth", "turing"};
    int device_count = sizeof(devices)/sizeof(devices[0]);
    char topic[48] = {0};
    char payload[192] = {0};

    while (mqtt_is_connected(&mqtt) && err==ERR_OK)
    {
        // Publish sensor data for each sensor device
        // In normal scenario, there should be only 1 device
        for (int i=0; i<device_count && mqtt_is_connected(&mqtt) && err==ERR_OK; i++)
        {
            int len = user_generate_publish_topic(topic, sizeof(topic), devices[i]);
            len = user_generate_publish_payload(payload, sizeof(payload), devices[i]);

            err = mqtt_publish_async(&mqtt, topic, payload, len);
            if (err==ERR_OK) {
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }
    }
#else // IOT_APP_MODE_PUBLISH
    while (mqtt_is_connected(&mqtt));
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
#endif // IOT_APP_MODE_PUBLISH


    //
    // Release resources
    //

close:
    DEBUG_PRINTF("MQTT Application ended...\r\n\r\n\r\n");
    mqtt_disconnect(&mqtt);
    altcp_tls_free_config(config);
    iot_free();
}

