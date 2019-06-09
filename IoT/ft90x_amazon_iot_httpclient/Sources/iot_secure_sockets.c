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

/**
 * @file secure_sockets.c
 * @brief Secure Socket interface implementation for FT900 board using mbedTLS and LWIP
 */

#include <stdint.h>
#include <string.h>
#include <time.h>
#include "tinyprintf.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Socket interface includes. */
#include "lwip/sockets.h"
#include "lwip/ip4_addr.h"
#if LWIP_DNS
#include "lwip/dns.h"
#include "lwip/netdb.h"
#endif

#define IOT_CONFIG_USE_TLS 1
#define IOT_CONFIG_USE_ROOTCA 1
#define IOT_CONFIG_USE_DEVICE_CERTS 0

/* mbedTLS includes. */
#if IOT_CONFIG_USE_TLS
#include "mbedtls_config.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/net.h"
#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#endif // IOT_CONFIG_USE_TLS

/* IoT includes. */
//#include "iot_clientcredential.h"
//#include "iot_config.h"
#include "iot_secure_sockets.h"



/*-----------------------------------------------------------*/

#ifdef DEBUG
#define DEBUG_CONNECT(...)
//#define DEBUG_CONNECT_VERBOSE(...)
//#define DEBUG_CONNECT             DEBUG_MINIMAL
#define DEBUG_CONNECT_VERBOSE     DEBUG_PRINTF
#define DEBUG_SEND(...)
#define DEBUG_RECV(...)
#else
#define DEBUG_CONNECT(...)
#define DEBUG_CONNECT_VERBOSE(...)
#define DEBUG_SEND(...)
#define DEBUG_RECV(...)
#define DEBUG_PRINTF(...) //do {tfp_printf(__VA_ARGS__);} while (0)
#define DEBUG_MINIMAL(...) do {tfp_printf(__VA_ARGS__);} while (0)
#endif

/*-----------------------------------------------------------*/

typedef struct sslclient_context {

    int socket;

#if IOT_CONFIG_USE_TLS
    mbedtls_ssl_context ssl_ctx;
    mbedtls_ssl_config ssl_conf;

    mbedtls_ctr_drbg_context drbg_ctx;
    mbedtls_entropy_context entropy_ctx;

    mbedtls_x509_crt ca_cert;
    mbedtls_x509_crt client_cert;
    mbedtls_pk_context client_key;
#endif // IOT_CONFIG_USE_TLS

} sslclient_context;

typedef struct ESPSecureSocket
{
    uint8_t ucInUse;                    /**< Tracks whether the socket is in use or not. */
    char * pcDestination;               /**< Destination URL. Set using SOCKETS_SO_SERVER_NAME_INDICATION option in SOCKETS_SetSockOpt function. */
    sslclient_context* sslCtx;

} ESPSecureSocket_t;

static ESPSecureSocket_t xSockets[ 1 ] = {0};

/*-----------------------------------------------------------*/

#if IOT_CONFIG_USE_ROOTCA
#if 0
// ATS
static const char IOT_CLIENTCREDENTIAL_CA_CERTIFICATE[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
"rqXRfboQnoZsG4q5WTP468SQvvG5\n"
"-----END CERTIFICATE-----\n";
#else
// Verisign
static const char IOT_CLIENTCREDENTIAL_CA_CERTIFICATE[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIE0zCCA7ugAwIBAgIQGNrRniZ96LtKIVjNzGs7SjANBgkqhkiG9w0BAQUFADCB\n"
"yjELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL\n"
"ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJp\n"
"U2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxW\n"
"ZXJpU2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0\n"
"aG9yaXR5IC0gRzUwHhcNMDYxMTA4MDAwMDAwWhcNMzYwNzE2MjM1OTU5WjCByjEL\n"
"MAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQLExZW\n"
"ZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJpU2ln\n"
"biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxWZXJp\n"
"U2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9y\n"
"aXR5IC0gRzUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvJAgIKXo1\n"
"nmAMqudLO07cfLw8RRy7K+D+KQL5VwijZIUVJ/XxrcgxiV0i6CqqpkKzj/i5Vbex\n"
"t0uz/o9+B1fs70PbZmIVYc9gDaTY3vjgw2IIPVQT60nKWVSFJuUrjxuf6/WhkcIz\n"
"SdhDY2pSS9KP6HBRTdGJaXvHcPaz3BJ023tdS1bTlr8Vd6Gw9KIl8q8ckmcY5fQG\n"
"BO+QueQA5N06tRn/Arr0PO7gi+s3i+z016zy9vA9r911kTMZHRxAy3QkGSGT2RT+\n"
"rCpSx4/VBEnkjWNHiDxpg8v+R70rfk/Fla4OndTRQ8Bnc+MUCH7lP59zuDMKz10/\n"
"NIeWiu5T6CUVAgMBAAGjgbIwga8wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8E\n"
"BAMCAQYwbQYIKwYBBQUHAQwEYTBfoV2gWzBZMFcwVRYJaW1hZ2UvZ2lmMCEwHzAH\n"
"BgUrDgMCGgQUj+XTGoasjY5rw8+AatRIGCx7GS4wJRYjaHR0cDovL2xvZ28udmVy\n"
"aXNpZ24uY29tL3ZzbG9nby5naWYwHQYDVR0OBBYEFH/TZafC3ey78DAJ80M5+gKv\n"
"MzEzMA0GCSqGSIb3DQEBBQUAA4IBAQCTJEowX2LP2BqYLz3q3JktvXf2pXkiOOzE\n"
"p6B4Eq1iDkVwZMXnl2YtmAl+X6/WzChl8gGqCBpH3vn5fJJaCGkgDdk+bW48DW7Y\n"
"5gaRQBi5+MHt39tBquCWIMnNZBU4gcmU7qKEKQsTb47bDN0lAtukixlE0kF6BWlK\n"
"WE9gyn6CagsCqiUXObXbf+eEZSqVir2G3l6BFoMtEMze/aiCKm0oHw0LxOXnGiYZ\n"
"4fQRbxC1lfznQgUy286dUV4otp6F01vvpX1FQHKOtw5rDgb7MzVIcbidJ4vEZV8N\n"
"hnacRHr2lVz2XTIIM6RUthg/aFzyQkqFOFSDX9HoLPKsEdao7WNq\n"
"-----END CERTIFICATE-----\n";
#endif
#endif // IOT_CONFIG_USE_ROOTCA

/*-----------------------------------------------------------*/

#if IOT_CONFIG_USE_TLS

static int _TLS_send(void *ctx, const unsigned char *ptr, size_t size)
{
    sslclient_context *ssl_client = xSockets[0].sslCtx;

    int ret = lwip_send(ssl_client->socket, ptr, size, 0);
    if (ret != size) {
        DEBUG_SEND("_TLS_send lwip_send failed! %d\r\n", ret);
    }

    return ret;
}

static int _TLS_recv(void *ctx, unsigned char *ptr, size_t size)
{
    sslclient_context *ssl_client = xSockets[0].sslCtx;

    int ret = lwip_recv(ssl_client->socket, ptr, size, 0);
    if (ret <= 0) {
        DEBUG_RECV("_TLS_recv lwip_recv failed! %d\r\n", ret);
    }
    else if (ret != size) {
        DEBUG_RECV("_TLS_recv lwip_recv %d\r\n", ret);
    }

    return ret;
}

#endif


/*-----------------------------------------------------------*/

Socket_t SOCKETS_Socket(void)
{
    uint32_t ulSocketNumber = 0;

    xSockets[ ulSocketNumber ].ucInUse = 1;
    xSockets[ ulSocketNumber ].pcDestination = NULL;
    xSockets[ ulSocketNumber ].sslCtx = pvPortMalloc(sizeof(sslclient_context));
    memset(xSockets[ ulSocketNumber ].sslCtx, 0, sizeof(sslclient_context));

    /* If we fail to get a free socket, we return SOCKETS_INVALID_SOCKET. */
    return ( Socket_t ) ulSocketNumber; /*lint !e923 cast required for portability. */
}

/*-----------------------------------------------------------*/

static inline int32_t socketConnect(const char* pcHostName, uint16_t uwPort)
{
    int32_t lSocket = 0;
    struct sockaddr_in serv_addr = {0};
    struct in_addr addr = {0};
    int enable = 1;
    int ret = 0;
    int timeout = 0;


    lSocket = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (lSocket < 0) {
        DEBUG_PRINTF("ERROR opening socket\r\n");
        return SOCKETS_SOCKET_ERROR;
    }

#if LWIP_DNS
    addr.s_addr = ipaddr_addr(pcHostName);
    if (addr.s_addr == IPADDR_NONE) {
        struct hostent* server = lwip_gethostbyname(pcHostName);
        if (server == NULL) {
            DEBUG_PRINTF("ERROR gethostbyname\r\n");
            lwip_close(lSocket);
            return SOCKETS_SOCKET_ERROR;
        }
        if (server->h_addr_list[0] != 0) {
            addr.s_addr = *(u_long *)server->h_addr_list[0];
            DEBUG_MINIMAL("DNS %s: %s\r\n", pcHostName, inet_ntoa(addr));
        }
    }
#else // LWIP_DNS
    addr.s_addr = ipaddr_addr(pcHostName);
    if (addr.s_addr == IPADDR_NONE) {
        DEBUG_PRINTF("ERROR ipaddr_addr\r\n");
        lwip_close(lSocket);
        return SOCKETS_SOCKET_ERROR;
    }
#endif // LWIP_DNS

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_len = sizeof(struct sockaddr_in);
    serv_addr.sin_family = AF_INET;
    memcpy(&(serv_addr.sin_addr), &addr, sizeof(struct in_addr));
    serv_addr.sin_port = htons(uwPort);

    ret = lwip_connect(lSocket, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
    if (ret != 0) {
        DEBUG_PRINTF("Connect failed! %d\r\n", ret);
        lwip_close(lSocket);
        return SOCKETS_SOCKET_ERROR;
    }

    //timeout = 10000;
    //lwip_setsockopt(lSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    //lwip_setsockopt(lSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    lwip_setsockopt(lSocket, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
    lwip_setsockopt(lSocket, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));

    DEBUG_MINIMAL("Connected to %s:%d\r\n", inet_ntoa(addr), uwPort);
    return lSocket;
}


/*-----------------------------------------------------------*/

int32_t SOCKETS_Connect(
    Socket_t xSocket,
    SocketsSockaddr_t * pxAddress )
{
    int32_t lRetVal = SOCKETS_SOCKET_ERROR;
    ESPSecureSocket_t * pxSecureSocket;
    int ret = 0;
#if IOT_CONFIG_USE_TLS
#if IOT_CONFIG_USE_ROOTCA
#if IOT_CONFIG_USE_CERT_OPTIMIZATION
    extern __flash__ uint8_t IOT_CLIENTCREDENTIAL_CA_CERTIFICATE[]      asm(IOT_CLIENTCREDENTIAL_CA_CERTIFICATE_NAME);
    extern __flash__ uint8_t IOT_CLIENTCREDENTIAL_CA_CERTIFICATE_END[]  asm(IOT_CLIENTCREDENTIAL_CA_CERTIFICATE_END_NAME);

    ret = IOT_CLIENTCREDENTIAL_CA_CERTIFICATE_END - IOT_CLIENTCREDENTIAL_CA_CERTIFICATE + 1;
    char *ca_cert = pvPortMalloc(ret + 1);
    if (!ca_cert) {
        return SOCKETS_SOCKET_ERROR;
    }
    memcpy_pm2dat(ca_cert, IOT_CLIENTCREDENTIAL_CA_CERTIFICATE, ret);
    ca_cert[ret] = '\0';
#else // IOT_CONFIG_USE_CERT_OPTIMIZATION
    const char *ca_cert = IOT_CLIENTCREDENTIAL_CA_CERTIFICATE;
#endif // IOT_CONFIG_USE_CERT_OPTIMIZATION
#else // USE_ROOTCA
    const char *ca_cert = NULL;
#endif // USE_ROOTCA
#if IOT_CONFIG_USE_CERT_OPTIMIZATION
    extern __flash__ uint8_t IOT_CLIENTCREDENTIAL_CERTIFICATE[]      asm(IOT_CLIENTCREDENTIAL_CERTIFICATE_NAME);
    extern __flash__ uint8_t IOT_CLIENTCREDENTIAL_CERTIFICATE_END[]  asm(IOT_CLIENTCREDENTIAL_CERTIFICATE_END_NAME);
    extern __flash__ uint8_t IOT_CLIENTCREDENTIAL_PRIVATEKEY[]       asm(IOT_CLIENTCREDENTIAL_PRIVATEKEY_NAME);
    extern __flash__ uint8_t IOT_CLIENTCREDENTIAL_PRIVATEKEY_END[]   asm(IOT_CLIENTCREDENTIAL_PRIVATEKEY_END_NAME);

    ret = IOT_CLIENTCREDENTIAL_CERTIFICATE_END - IOT_CLIENTCREDENTIAL_CERTIFICATE + 1;
    char *cli_cert = pvPortMalloc(ret + 1);
    if (!cli_cert) {
        return SOCKETS_SOCKET_ERROR;
    }
    memcpy_pm2dat(cli_cert, IOT_CLIENTCREDENTIAL_CERTIFICATE, ret);
    cli_cert[ret] = '\0';

    ret = IOT_CLIENTCREDENTIAL_PRIVATEKEY_END - IOT_CLIENTCREDENTIAL_PRIVATEKEY + 1;
    char *cli_key = pvPortMalloc(ret + 1);
    if (!cli_key) {
        return SOCKETS_SOCKET_ERROR;
    }
    memcpy_pm2dat(cli_key, IOT_CLIENTCREDENTIAL_PRIVATEKEY, ret);
    cli_key[ret] = '\0';
#else // IOT_CONFIG_USE_CERT_OPTIMIZATION
#if IOT_CONFIG_USE_DEVICE_CERTS
    const char *cli_cert = IOT_CLIENTCREDENTIAL_CERTIFICATE;
    const char *cli_key = IOT_CLIENTCREDENTIAL_PRIVATEKEY;
#else
    const char *cli_cert = NULL;
    const char *cli_key = NULL;
#endif
#endif // IOT_CONFIG_USE_CERT_OPTIMIZATION
#endif // IOT_CONFIG_USE_TLS


    if( xSockets[ ( uint32_t ) xSocket ].ucInUse ) {
        pxSecureSocket = &( xSockets[ ( uint32_t ) xSocket ] );

        sslclient_context *ssl_client = pxSecureSocket->sslCtx;

        ssl_client->socket = socketConnect(pxAddress->pcServer, pxAddress->usPort);
        if (ssl_client->socket < 0) {
            DEBUG_PRINTF("ERROR opening socket\r\n");
            return SOCKETS_SOCKET_ERROR;
        }

#if IOT_CONFIG_USE_TLS
        const char *pers = "ft90x-bridgetek";
        int flags = 0;


        mbedtls_ssl_init(&ssl_client->ssl_ctx);
        mbedtls_ssl_config_init(&ssl_client->ssl_conf);
        mbedtls_ctr_drbg_init(&ssl_client->drbg_ctx);

        DEBUG_CONNECT_VERBOSE("Seeding the random number generator\r\n");

        mbedtls_entropy_init(&ssl_client->entropy_ctx);
        ret = mbedtls_ctr_drbg_seed(&ssl_client->drbg_ctx, mbedtls_entropy_func,
                                    &ssl_client->entropy_ctx, (const unsigned char *) pers, strlen(pers));
        if (ret < 0) {
            DEBUG_PRINTF("mbedtls_ctr_drbg_seed failed! %d\r\n", ret);
            return SOCKETS_SOCKET_ERROR;
        }

        DEBUG_CONNECT_VERBOSE("Setting up the SSL/TLS structure...\r\n");

        if ((ret = mbedtls_ssl_config_defaults(&ssl_client->ssl_conf,
                                               MBEDTLS_SSL_IS_CLIENT,
                                               MBEDTLS_SSL_TRANSPORT_STREAM,
                                               MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
            DEBUG_PRINTF("mbedtls_ssl_config_defaults failed! %d\r\n", ret);
            return SOCKETS_SOCKET_ERROR;
        }

#if IOT_CONFIG_USE_ROOTCA
        if (ca_cert != NULL) {
            DEBUG_CONNECT_VERBOSE("Loading CA cert %d\r\n", strlen(ca_cert));
            mbedtls_x509_crt_init(&ssl_client->ca_cert);
            mbedtls_ssl_conf_authmode(&ssl_client->ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED);

            ret = mbedtls_x509_crt_parse(&ssl_client->ca_cert, (const unsigned char *)ca_cert, strlen(ca_cert) + 1);
#if IOT_CONFIG_USE_CERT_OPTIMIZATION
            vPortFree(ca_cert);
#endif // IOT_CONFIG_USE_CERT_OPTIMIZATION
            mbedtls_ssl_conf_ca_chain(&ssl_client->ssl_conf, &ssl_client->ca_cert, NULL);
            if (ret < 0) {
                DEBUG_PRINTF("mbedtls_x509_crt_parse failed! %d\r\n", ret);
                return SOCKETS_SOCKET_ERROR;
            }
        }
        else
        {
            mbedtls_ssl_conf_authmode(&ssl_client->ssl_conf, MBEDTLS_SSL_VERIFY_NONE);
            //DEBUG_PRINTF("WARNING: Verify server certificate to prevent man-in-the-middle attacks!\r\n");
        }
#else // USE_ROOTCA
        mbedtls_ssl_conf_authmode(&ssl_client->ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
#endif // USE_ROOTCA

#if IOT_CONFIG_USE_DEVICE_CERTS
        if (cli_cert != NULL && cli_key != NULL) {
            mbedtls_x509_crt_init(&ssl_client->client_cert);
            mbedtls_pk_init(&ssl_client->client_key);

            DEBUG_CONNECT_VERBOSE("Loading CRT cert %d\r\n", strlen(cli_cert));

            ret = mbedtls_x509_crt_parse(&ssl_client->client_cert, (const unsigned char *)cli_cert, strlen(cli_cert) + 1);
#if IOT_CONFIG_USE_CERT_OPTIMIZATION
            vPortFree(cli_cert);
#endif // IOT_CONFIG_USE_CERT_OPTIMIZATION
            if (ret < 0) {
                DEBUG_PRINTF("mbedtls_x509_crt_parse failed! %d\r\n", ret);
                lRetVal = SOCKETS_SOCKET_ERROR;
                goto cleanup;
            }

            DEBUG_CONNECT_VERBOSE("Loading private key %d\r\n", strlen(cli_key));

            ret = mbedtls_pk_parse_key(&ssl_client->client_key, (const unsigned char *)cli_key, strlen(cli_key) + 1, NULL, 0);
#if IOT_CONFIG_USE_CERT_OPTIMIZATION
            vPortFree(cli_key);
#endif // IOT_CONFIG_USE_CERT_OPTIMIZATION
            if (ret != 0) {
                DEBUG_PRINTF("mbedtls_pk_parse_key failed! %d\r\n", ret);
                lRetVal = SOCKETS_SOCKET_ERROR;
                goto cleanup;
            }

            ret = mbedtls_ssl_conf_own_cert(&ssl_client->ssl_conf, &ssl_client->client_cert, &ssl_client->client_key);
            if (ret != 0) {
                DEBUG_PRINTF("mbedtls_ssl_conf_own_cert failed! %d\r\n", ret);
                lRetVal = SOCKETS_SOCKET_ERROR;
                goto cleanup;
            }
        }
#endif // IOT_CONFIG_USE_DEVICE_CERTS

        mbedtls_ssl_conf_rng(&ssl_client->ssl_conf, mbedtls_ctr_drbg_random, &ssl_client->drbg_ctx);
        if ((ret = mbedtls_ssl_setup(&ssl_client->ssl_ctx, &ssl_client->ssl_conf)) != 0) {
            DEBUG_PRINTF("mbedtls_ssl_setup failed! %d\r\n", ret);
            lRetVal = SOCKETS_SOCKET_ERROR;
            goto cleanup;
        }

        mbedtls_ssl_set_bio(&ssl_client->ssl_ctx, &ssl_client->socket, _TLS_send, _TLS_recv, NULL);//_TLS_recv_timeout );

#if 1
        // Setting maximal fragment size for requests with big content length
        if ((ret = mbedtls_ssl_conf_max_frag_len(&ssl_client->ssl_conf, MBEDTLS_SSL_MAX_FRAG_LEN_NONE)) != 0) {
            DEBUG_PRINTF("Failed setting max frag len!\r\n");
        }

        // Get current fragment length
        //tfp_printf("Max fragment length = %d\r\n", mbedtls_ssl_get_max_frag_len(&ssl_client->ssl_ctx));
#endif

        DEBUG_CONNECT_VERBOSE("SSL/TLS handshake\r\n");

        while ((ret = mbedtls_ssl_handshake(&ssl_client->ssl_ctx)) != 0) {
            DEBUG_MINIMAL("TLS handshake failed! 0x%x\r\n", -1*ret);
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
                lRetVal = SOCKETS_SOCKET_ERROR;
                goto cleanup;
            }
            vTaskDelay(100);
        }

        DEBUG_CONNECT_VERBOSE("SSL/TLS handshake successful.\r\n");

        if (cli_cert != NULL && cli_key != NULL) {
            DEBUG_CONNECT_VERBOSE("Protocol is %s Ciphersuite is %s\r\n", mbedtls_ssl_get_version(&ssl_client->ssl_ctx), mbedtls_ssl_get_ciphersuite(&ssl_client->ssl_ctx));
        }

        DEBUG_CONNECT_VERBOSE("Verifying peer X.509 certificate...\r\n");

        if ((flags = mbedtls_ssl_get_verify_result(&ssl_client->ssl_ctx)) != 0) {
            DEBUG_PRINTF("Failed to verify peer certificate!\r\n");
            lRetVal = SOCKETS_SOCKET_ERROR;
            goto cleanup;
        }

        DEBUG_CONNECT_VERBOSE("Certificate verified.\r\n");

        lRetVal = SOCKETS_ERROR_NONE;


cleanup:
        /* Free some unused resource */
        if (ca_cert) {
            mbedtls_x509_crt_free(&ssl_client->ca_cert);
        }
        if (cli_cert) {
            mbedtls_x509_crt_free(&ssl_client->client_cert);
        }
        if (cli_key) {
            mbedtls_pk_free(&ssl_client->client_key);
        }
        if (lRetVal != SOCKETS_ERROR_NONE) {
            SOCKETS_Close( xSocket );
        }
        else {
            DEBUG_MINIMAL("Secure channel created.\r\n\r\n");
        }
#endif // IOT_CONFIG_USE_TLS

    }

    return lRetVal;
}

/*-----------------------------------------------------------*/

int32_t SOCKETS_Recv(
    Socket_t xSocket,
    void * pvBuffer,
    size_t xBufferLength,
    uint32_t ulFlags )
{
    int32_t lReceivedBytes = SOCKETS_SOCKET_ERROR;

    if( xSockets[ ( uint32_t ) xSocket ].ucInUse )
    {
        DEBUG_RECV("Receiving data\r\n");

        sslclient_context *ssl_client = xSockets[( uint32_t ) xSocket].sslCtx;
        //struct timeval timeout = {0};
        //timeout.tv_sec = 5;
        //lwip_setsockopt(ssl_client->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

#if IOT_CONFIG_USE_TLS
        lReceivedBytes = mbedtls_ssl_read(&ssl_client->ssl_ctx, pvBuffer, xBufferLength);
#else // IOT_CONFIG_USE_TLS
        lReceivedBytes = lwip_recv(ssl_client->socket, pvBuffer, xBufferLength, 0);
#endif // IOT_CONFIG_USE_TLS

        if (lReceivedBytes > 0)
        {
            DEBUG_RECV("Received %d bytes\r\n", (int)lReceivedBytes);
        }
        else { //if (lReceivedBytes < 0) {
            if (errno == EBADF || errno == ENOTCONN || errno == EINVAL) {
                DEBUG_MINIMAL("Failed recv [ret=%d][errno=%d]\r\n", (int)lReceivedBytes, errno);
                lReceivedBytes = SOCKETS_SOCKET_ERROR;
                SOCKETS_Close( xSocket );
            }
            else {
                DEBUG_MINIMAL("Recv timed out %d %d\r\n", lReceivedBytes, errno);
                lReceivedBytes = SOCKETS_ERROR_NONE;
            }
        }
    }

    return lReceivedBytes;
}

/*-----------------------------------------------------------*/

int32_t SOCKETS_Send(
    Socket_t xSocket,
    const void * pvBuffer,
    size_t xDataLength,
    uint32_t ulFlags )
{
    int32_t lSentBytes = SOCKETS_SOCKET_ERROR;

    if ( xSockets[ ( uint32_t ) xSocket ].ucInUse )
    {
        DEBUG_SEND("Sending data %d\r\n", xDataLength);

        sslclient_context *ssl_client = xSockets[( uint32_t ) xSocket].sslCtx;

#if IOT_CONFIG_USE_TLS
        while ((lSentBytes = mbedtls_ssl_write(&ssl_client->ssl_ctx, pvBuffer, xDataLength)) <= 0)
        {
            if (lSentBytes != MBEDTLS_ERR_SSL_WANT_READ && lSentBytes != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                break;
            }
        }
#else // IOT_CONFIG_USE_TLS
        lSentBytes = lwip_send(ssl_client->socket, pvBuffer, xDataLength, 0);
#endif // IOT_CONFIG_USE_TLS


        if (xDataLength == lSentBytes)
        {
            DEBUG_SEND("Sent %d bytes\r\n", (int)lSentBytes);
            lSentBytes = xDataLength;
        }
        else
        {
            DEBUG_MINIMAL("Failed send [ret=%d][errno=%d]\r\n", (int)lSentBytes, errno);
            lSentBytes = SOCKETS_SOCKET_ERROR;
            SOCKETS_Close( xSocket );
        }
    }

    return lSentBytes;
}
/*-----------------------------------------------------------*/

int32_t SOCKETS_Close( Socket_t xSocket )
{
    if ( xSockets[ ( uint32_t ) xSocket ].ucInUse )
    {
        xSockets[ ( uint32_t ) xSocket ].ucInUse = 0;
        if (xSockets[ ( uint32_t ) xSocket ].pcDestination)
        {
            vPortFree(xSockets[ ( uint32_t ) xSocket ].pcDestination);
            xSockets[ ( uint32_t ) xSocket ].pcDestination = NULL;
        }

        DEBUG_PRINTF("\r\nCleaning SSL connection.\r\n");

        sslclient_context *ssl_client = xSockets[( uint32_t ) xSocket].sslCtx;

        if (ssl_client->socket >= 0)
        {
            lwip_close(ssl_client->socket);
            lwip_shutdown(ssl_client->socket, SHUT_RDWR);
            ssl_client->socket = -1;
        }

#if IOT_CONFIG_USE_TLS
        mbedtls_ssl_free(&ssl_client->ssl_ctx);
        mbedtls_ssl_config_free(&ssl_client->ssl_conf);
        mbedtls_ctr_drbg_free(&ssl_client->drbg_ctx);
        mbedtls_entropy_free(&ssl_client->entropy_ctx);
#endif // IOT_CONFIG_USE_TLS

        vPortFree(xSockets[( uint32_t ) xSocket].sslCtx);
        xSockets[( uint32_t ) xSocket].sslCtx = NULL;

        DEBUG_PRINTF("Cleaning SSL connection done.\r\n");
    }

    return SOCKETS_ERROR_NONE;
}

/*-----------------------------------------------------------*/

int32_t SOCKETS_SetSockOpt(
    Socket_t xSocket,
    int32_t lLevel,
    int32_t lOptionName,
    const void * pvOptionValue,
    size_t xOptionLength )
{
    int32_t lRetVal = SOCKETS_ERROR_NONE;
    ESPSecureSocket_t * pxSecureSocket;

    /* Ensure that a valid socket was passed. */
    if ( xSockets[ ( uint32_t ) xSocket ].ucInUse )
    {
        /* Shortcut for easy access. */
        pxSecureSocket = &( xSockets[ ( uint32_t ) xSocket ] );

        switch( lOptionName )
        {
            case SOCKETS_SO_SERVER_NAME_INDICATION:

                /* Non-NULL destination string indicates that SNI extension should
                 * be used during TLS negotiation. */
                pxSecureSocket->pcDestination = ( char * ) pvPortMalloc( 1U + xOptionLength );
                if ( pxSecureSocket->pcDestination == NULL )
                {
                    lRetVal = SOCKETS_ENOMEM;
                    break;
                }

                memcpy( pxSecureSocket->pcDestination, pvOptionValue, xOptionLength );
                pxSecureSocket->pcDestination[ xOptionLength ] = '\0';
                break;

            default:

                lRetVal = SOCKETS_ENOPROTOOPT;
                break;
        }
    }
    else
    {
        lRetVal = SOCKETS_SOCKET_ERROR;
    }

    return lRetVal;
}

/*-----------------------------------------------------------*/

