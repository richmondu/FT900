/*
 * ============================================================================
 * History
 * =======
 * 29 Oct 2018 : Created
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
#include "../include/iot/iot_utils.h"
#include <iot_config.h>
#include <mbedtls_config.h>



///////////////////////////////////////////////////////////////////////////////////
#if DEBUG_IOT_API
#define DEBUG_PRINTF(...) do {CRITICAL_SECTION_BEGIN;tfp_printf(__VA_ARGS__);CRITICAL_SECTION_END;} while (0)
#else
#define DEBUG_PRINTF(...)
#endif
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
#if defined(MBEDTLS_SSL_ALPN) && defined(ALTCP_MBEDTLS_ALPN_ENABLE)
// TODO: Currently supports AWS IoT. Need to support GCP IoT and Azure IoT
static const char *g_alpn_protocols[] = { "x-amzn-mqtt-ca", NULL };
#endif
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
/* MQTT-related abstractions */
static inline err_t mqtt_connect_async( mqtt_client_t *client,
    const char* broker, uint16_t port, struct mqtt_connect_client_info_t *info );
static void mqtt_connect_callback( mqtt_client_t *client,
    void *arg, mqtt_connection_status_t status);
static void mqtt_pubsub_callback( void *arg, err_t result );

#if USE_MQTT_PUBLISH
static inline err_t mqtt_publish_async( mqtt_client_t *client,
    const char* topic, const char* msg, int msg_len );
#endif // USE_MQTT_PUBLISH

#if USE_MQTT_SUBSCRIBE
static inline err_t mqtt_subscribe_async(
    mqtt_client_t *client, const char* topic, iot_subscribe_cb subscribe_cb);
static void mqtt_subscribe_recv_topic(
    void *arg, const char *topic, u32_t tot_len);
static void mqtt_subscribe_recv_payload(
    void *arg, const u8_t *data, u16_t len, u8_t flags);
#endif // USE_MQTT_SUBSCRIBE
///////////////////////////////////////////////////////////////////////////////////



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
    int trials = 0;
    do {
        host = gethostbyname( broker );
        if (host == NULL) {
            DEBUG_PRINTF( "gethostbyname failed\r\n" );
            vTaskDelay( pdMS_TO_TICKS(1000) );
            trials++;
            continue;
        }
        break;
    }
    while ( net_is_ready() && trials < 5 );

    /* copy the network address to sockaddr_in structure */
    if ( (host->h_addrtype == AF_INET) && (host->h_length == sizeof(ip_addr_t)) ) {
        memcpy( &host_addr, host->h_addr_list[0], sizeof(ip_addr_t) );
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

static iot_subscribe_rcv* subscribe_recv = NULL;

static inline err_t mqtt_subscribe_async(
    mqtt_client_t *client, const char* topic, iot_subscribe_cb subscribe_cb)
{
    err_t err;
    u8_t qos = 1;

    err = mqtt_subscribe(
        client, topic, qos, mqtt_pubsub_callback, "SUBSCRIBE" );
    if ( err != ERR_OK ) {
        DEBUG_PRINTF( "\r\nmqtt_subscribe failed! %d\r\n", err );
    }
    else {
        subscribe_recv = pvPortMalloc( sizeof(iot_subscribe_rcv) );
        memset( subscribe_recv, 0, sizeof(iot_subscribe_rcv) );
        mqtt_set_inpub_callback(
            client, mqtt_subscribe_recv_topic, mqtt_subscribe_recv_payload, subscribe_cb );
    }

    return err;
}

static void mqtt_subscribe_recv_topic(
    void *arg, const char *topic, u32_t tot_len )
{
    //DEBUG_PRINTF( "\r\nMQTT RECEIVE: %s [%d]\r\n", topic, (unsigned int)tot_len );
    if ( !subscribe_recv ) {
        return;
    }
    if ( subscribe_recv->topic ) {
        if ( strncmp(subscribe_recv->topic, topic, strlen(subscribe_recv->topic))!=0 ) {
            vPortFree( (char*)subscribe_recv->topic );
            subscribe_recv->topic = NULL;
        }
        if ( subscribe_recv->payload_size < tot_len + 1 ) {
            vPortFree( (char*)subscribe_recv->payload );
            subscribe_recv->payload_size = tot_len + 1 + 10;
            subscribe_recv->payload = pvPortMalloc( subscribe_recv->payload_size );
        }
        subscribe_recv->payload_len = tot_len;
        subscribe_recv->payload_off = 0;
        memset( (char*)subscribe_recv->payload, 0, subscribe_recv->payload_size );
    }
    if ( !subscribe_recv->topic ) {
        int len = strlen(topic);
        subscribe_recv->topic = pvPortMalloc( len + 1 );
        strncpy( (char*)subscribe_recv->topic, topic, len );
        ((char*)subscribe_recv->topic)[len] = '\0';

        subscribe_recv->payload_len = tot_len;
        subscribe_recv->payload_off = 0;
        if ( subscribe_recv->payload == NULL ) {
            subscribe_recv->payload_size = tot_len + 1 + 10;
            subscribe_recv->payload = pvPortMalloc( subscribe_recv->payload_size );
            memset( (char*)subscribe_recv->payload, 0, subscribe_recv->payload_size );
        }
    }
}

static void mqtt_subscribe_recv_payload(
    void *arg, const u8_t *data, u16_t len, u8_t flags )
{
    if ( subscribe_recv ) {
        if (subscribe_recv->payload) {
            memcpy( (char*)subscribe_recv->payload + subscribe_recv->payload_off, data, len );
            subscribe_recv->payload_off += len;
            if ( subscribe_recv->payload_off == subscribe_recv->payload_len ) {
                //DEBUG_PRINTF( "%s\r\n\r\n", subscribe_recv->payload );
                ((iot_subscribe_cb)arg)(subscribe_recv);
            }
        }
    }
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
        DEBUG_PRINTF( "\r\nmqtt_publish failed! %d ready=%d connected=%d\r\n",
            err, net_is_ready(), mqtt_client_is_connected( client ) );
    }

    return err;
}

#endif // USE_MQTT_PUBLISH



/** @brief IoT Context for internal use only
 */
typedef struct _iot_context {
    struct altcp_tls_config *tls_config;
    mqtt_client_t mqtt;
} iot_context;

/** @brief Establish secure IoT connectivity using TLS certificates and MQTT credentials
 *  @param certificates_cb Callback function for specifying the TLS certificates
 *  @param credentials_cb Callback function for specifying the MQTT credentials
 *  @returns Returns a handle to be used for succeeding IoT calls
 */
void* iot_connect( iot_certificates_cb certificates_cb, iot_credentials_cb credentials_cb )
{
    struct mqtt_connect_client_info_t mqtt_info = {0};
    iot_context* handle = NULL;
    err_t err = ERR_OK;
    iot_certificates tls_certificates = {0};
    iot_credentials mqtt_credentials = {0};


    if ( !certificates_cb|| !credentials_cb ) {
        return NULL;
    }

    memset( &mqtt_info, 0, sizeof( mqtt_info ) );
    handle = pvPortMalloc(sizeof( iot_context ));
    if (!handle) {
        return NULL;
    }
    memset( handle, 0, sizeof( iot_context ) );

    certificates_cb( &tls_certificates );
    if (tls_certificates.ca && tls_certificates.cert) {
        mqtt_info.tls_config = altcp_tls_create_config_client_2wayauth(
            tls_certificates.ca, tls_certificates.ca_len,
            tls_certificates.pkey, tls_certificates.pkey_len, NULL, 0,
            tls_certificates.cert, tls_certificates.cert_len );
    }
    else {
        mqtt_info.tls_config = altcp_tls_create_config_client(
            tls_certificates.ca, tls_certificates.ca_len );
    }
    vPortFree( (u8_t*)tls_certificates.ca );
    vPortFree( (u8_t*)tls_certificates.cert );
    vPortFree( (u8_t*)tls_certificates.pkey );
    if ( mqtt_info.tls_config == NULL ) {
        DEBUG_PRINTF( "altcp_tls_create_config_client failed!\r\n" );
        vPortFree(handle);
        return NULL;
    }

    //
    // Initialize MQTT settings/credentials
    //
    credentials_cb( &mqtt_credentials );
    mqtt_info.client_id = mqtt_credentials.client_id;
    mqtt_info.client_user = mqtt_credentials.client_user;
    mqtt_info.client_pass = mqtt_credentials.client_pass;

#if defined(MBEDTLS_SSL_ALPN) && defined(ALTCP_MBEDTLS_ALPN_ENABLE)
    //
    // Configure ALPN protocols to support MQTT over PORT 443 instead of 8883
    //
    if ( mqtt_credentials.server_port == 443) {
        err = altcp_tls_conf_alpn_protocols(mqtt_info.tls_config, g_alpn_protocols);
        if ( err != ERR_OK ) {
            DEBUG_PRINTF( "altcp_tls_conf_alpn_protocols failed! %d\r\n", err );
            altcp_tls_free_config( mqtt_info.tls_config );
            vPortFree( handle );
            return NULL;
        }
    }
#endif

    //
    // Establish secure MQTT connection via TLS
    //
    err = mqtt_connect_async(
        &handle->mqtt,
        mqtt_credentials.server_host,
        mqtt_credentials.server_port,
        &mqtt_info );
    if ( err != ERR_OK ) {
        DEBUG_PRINTF( "mqtt_connect_async failed! %d\r\n", err );
        altcp_tls_free_config( mqtt_info.tls_config );
        vPortFree( handle );
        return NULL;
    }

    //
    // Wait until connection is established
    //
    do {
        vTaskDelay( pdMS_TO_TICKS(1000) );
    }
    while ( !mqtt_client_is_connected( &handle->mqtt ) && net_is_ready() && !mqtt_connect_callback_err );
    if ( mqtt_connect_callback_err || !net_is_ready() ) {
        mqtt_connect_callback_err = 0;
        altcp_tls_free_config( mqtt_info.tls_config );
        mqtt_disconnect( &handle->mqtt );
        vPortFree( handle );
        return NULL;
    }
    vTaskDelay( pdMS_TO_TICKS(1000) );

    handle->tls_config = mqtt_info.tls_config;
    return handle;
}

/** @brief Disconnects IoT connectivity and cleans up resoures used
 *  @param handle Handle returned by the call to iot_connect()
 *  @returns Returns None
 */
void iot_disconnect(void* handle)
{
	iot_context* _handle = ( iot_context* )handle;

    if ( _handle ) {
        mqtt_disconnect( &_handle->mqtt );
        altcp_tls_free_config( _handle->tls_config );
        vPortFree( _handle );

#if USE_MQTT_SUBSCRIBE
        if ( subscribe_recv ) {
            if ( subscribe_recv->topic ) {
                vPortFree( (char*)subscribe_recv->topic );
                subscribe_recv->topic = NULL;
            }
            if ( subscribe_recv->payload ) {
                vPortFree( (char*)subscribe_recv->payload );
                subscribe_recv->payload = NULL;
            }
            vPortFree( subscribe_recv );
            subscribe_recv = NULL;
        }
#endif // USE_MQTT_SUBSCRIBE
    }
}

/** @brief Register callback function for a specified subscription topic
 *  @param handle Handle returned by the call to iot_connect()
 *  @param topic Topic of data to subscribe from
 *  @param subscribe_cb Callback function to be called when there is data on the specified topic
 *  @returns Returns 0 if success, negative value err_t otherwise
 */
int iot_subscribe( void* handle, const char* topic, iot_subscribe_cb subscribe_cb )
{
    err_t err = ERR_OK;
    iot_context *_handle = ( iot_context * )handle;


    if ( !_handle ) {
        return -1;
    }

#if USE_MQTT_SUBSCRIBE
    //
    // Subscribe from a topic
    //
    err = mqtt_subscribe_async( &_handle->mqtt, topic, subscribe_cb );
#endif // USE_MQTT_SUBSCRIBE

    return (int)err;
}

/** @brief Send/publish data on a specified topic
 *  @param handle Handle returned by the call to iot_connect()
 *  @param topic Topic of data to publish to
 *  @param payload Data to send/publish
 *  @param payload_len Length of data to send/publish
 *  @returns Returns 0 if success, negative value err_t otherwise
 */
int iot_publish( void* handle, const char* topic, const char* payload, int payload_len )
{
    err_t err = ERR_OK;
    iot_context *_handle = ( iot_context * )handle;


    if ( !_handle ) {
        return -1;
    }

#if USE_MQTT_PUBLISH
    // If publish fails with ERR_MEM, retry a few times
    // Sometimes it succeeds after retrying 1-5 times
    int retries = 0;
    do {
        err = mqtt_publish_async( &_handle->mqtt, topic, payload, payload_len );
        if ( err == ERR_MEM ) {
            retries++;
            vTaskDelay( pdMS_TO_TICKS(1000) );
        }
        else {
            break;
        }
    }
    while ( net_is_ready() && retries<5 );
#endif // USE_MQTT_PUBLISH

    return (int)err;
}

