/*
 * Amazon FreeRTOS Secure Sockets for STM32_IoT_Discovery_Kit V1.0.0 beta 1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */


/**
 * @file aws_secure_sockets.c
 * @brief WiFi and Secure Socket interface implementation for ST board.
 */
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "tinyprintf.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Socket and WiFi interface includes. */
#include "aws_secure_sockets.h"
#include "aws_clientcredential.h"

/* LWIP includes. */
#include "lwip/sockets.h"
#include "lwip/ip4_addr.h"
#if LWIP_DNS
#include "lwip/dns.h"
#include "lwip/netdb.h"
#endif

/* mbedTLS includes. */
#if USE_TLS
#include "mbedtls_config.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/net.h"
#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"

#endif // USE_TLS





/*-----------------------------------------------------------*/

#ifdef DEBUG
#define DEBUG_CONNECT(...) 			do {tfp_printf(__VA_ARGS__);} while (0)
#define DEBUG_CONNECT_VERBOSE(...) 	//do {tfp_printf(__VA_ARGS__);} while (0)
#define DEBUG_SEND(...)
#define DEBUG_RECV(...)
#else
#define DEBUG_CONNECT(...)
#define DEBUG_CONNECT_VERBOSE(...)
#define DEBUG_SEND(...)
#define DEBUG_RECV(...)
#endif

/*-----------------------------------------------------------*/

typedef struct sslclient_context {

    int socket;

#if USE_TLS
    mbedtls_ssl_context ssl_ctx;
    mbedtls_ssl_config ssl_conf;

    mbedtls_ctr_drbg_context drbg_ctx;
    mbedtls_entropy_context entropy_ctx;

    mbedtls_x509_crt ca_cert;
    mbedtls_x509_crt client_cert;
    mbedtls_pk_context client_key;
#endif // USE_TLS

} sslclient_context;

typedef struct ESPSecureSocket
{
    uint8_t ucInUse;                    /**< Tracks whether the socket is in use or not. */
    char * pcDestination;               /**< Destination URL. Set using SOCKETS_SO_SERVER_NAME_INDICATION option in SOCKETS_SetSockOpt function. */
    sslclient_context sslCtx;

} ESPSecureSocket_t;

static ESPSecureSocket_t xSockets[ 1 ] = {0};

/*-----------------------------------------------------------*/



/*-----------------------------------------------------------*/

#if USE_TLS

static int _TLS_send(void *ctx, const unsigned char *ptr, size_t size)
{
	sslclient_context *ssl_client = &xSockets[0].sslCtx;

	int ret = lwip_send(ssl_client->socket, ptr, size, 0);
	if (ret != size) {
		DEBUG_SEND("_TLS_send lwip_send failed! %d\r\n", ret);
	}

	return ret;
}

static int _TLS_recv(void *ctx, unsigned char *ptr, size_t size)
{
	sslclient_context *ssl_client = &xSockets[0].sslCtx;

	int ret = lwip_recv(ssl_client->socket, ptr, size, 0);
	if (ret <= 0) {
		DEBUG_RECV("_TLS_recv lwip_recv failed! %d\r\n", ret);
	}
	else if (ret != size) {
		DEBUG_RECV("_TLS_recv lwip_recv %d\r\n", ret);
	}

	return ret;
}

static int _TLS_recv_timeout(void *ctx, const unsigned char *ptr, size_t size, uint32_t timeoutMs)
{
	sslclient_context *ssl_client = &xSockets[0].sslCtx;

	if (timeoutMs) {
		struct timeval timeout = {0};
		timeout.tv_sec = timeoutMs >= 1000 ? timeoutMs/1000 : 0;
		timeout.tv_usec = timeoutMs < 1000 ? timeoutMs*1000 : 0;
		lwip_setsockopt(ssl_client->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	}

	int ret = lwip_recv(ssl_client->socket, ptr, size, 0);
	if (ret <= 0) {
		DEBUG_RECV("_TLS_recv _TLS_recv_timeout(%d) failed! ret=%d size=%d\r\n", (int)timeoutMs, ret, size);
	}
	else if (ret != size) {
		DEBUG_RECV("_TLS_recv _TLS_recv_timeout(%d) ret=%d size=%d\r\n", (int)timeoutMs, ret, size);
	}

	return ret;
}

static void generate_random(char *output, size_t len)
{
	int random1 = rand();
	int random2 = rand();
	int random3 = rand();
	int random4 = rand();
	int random5 = rand();
	int random6 = rand();
	tfp_snprintf(output, len, "%08d%08d%08d%08d%08d%08d",
		random1, random2, random3, random4, random5, random6);
}

int mbedtls_hardware_poll( void *data, unsigned char *output, size_t len, size_t *olen )
{
	generate_random((char*)output, len);
    *olen = len;

    return 0;
}

#endif


/*-----------------------------------------------------------*/

Socket_t SOCKETS_Socket( int32_t lDomain,
                         int32_t lType,
                         int32_t lProtocol )
{
    uint32_t ulSocketNumber = 0;

    xSockets[ ulSocketNumber ].ucInUse = 1;
	xSockets[ ulSocketNumber ].pcDestination = NULL;
	memset(&xSockets[ ulSocketNumber ].sslCtx, 0, sizeof(xSockets[ ulSocketNumber ].sslCtx));

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
			tfp_printf("DNS %s: %s\r\n", pcHostName, inet_ntoa(addr));
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

    timeout = 30000;
    lwip_setsockopt(lSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    lwip_setsockopt(lSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    lwip_setsockopt(lSocket, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
    lwip_setsockopt(lSocket, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));

	tfp_printf("Connected to %s:%d\r\n", inet_ntoa(addr), uwPort);
    return lSocket;
}

/*-----------------------------------------------------------*/

int32_t SOCKETS_Connect( Socket_t xSocket,
                         SocketsSockaddr_t * pxAddress,
                         Socklen_t xAddressLength )
{
    uint32_t ulSocketNumber = ( uint32_t ) xSocket; /*lint !e923 cast required for portability. */
    int32_t lRetVal = SOCKETS_SOCKET_ERROR;
    ESPSecureSocket_t * pxSecureSocket;
    int ret = 0;
#if USE_TLS
#if USE_ROOTCA
	const char *ca_cert = clientcredentialROOTCA_CERTIFICATE_PEM;
#else // USE_ROOTCA
	const char *ca_cert = NULL;
#endif // USE_ROOTCA
	const char *cli_cert = clientcredentialCLIENT_CERTIFICATE_PEM;
	const char *cli_key = clientcredentialCLIENT_PRIVATE_KEY_PEM;
#endif // USE_TLS


    if( xSockets[ ulSocketNumber ].ucInUse ) {
        pxSecureSocket = &( xSockets[ ulSocketNumber ] );

    	sslclient_context *ssl_client = &pxSecureSocket->sslCtx;

        ssl_client->socket = socketConnect(pxSecureSocket->pcDestination, pxAddress->usPort);
        if (ssl_client->socket < 0) {
        	DEBUG_PRINTF("ERROR opening socket\r\n");
            return SOCKETS_SOCKET_ERROR;
        }

#if USE_TLS
        const char *pers = "ft90x-bridgetek-tls";
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

#if USE_ROOTCA
        if (ca_cert != NULL) {
        	DEBUG_CONNECT_VERBOSE("Loading CA cert %d\r\n", strlen(rootCABuff));
            mbedtls_x509_crt_init(&ssl_client->ca_cert);

#if 1
            mbedtls_ssl_conf_authmode(&ssl_client->ssl_conf, MBEDTLS_SSL_VERIFY_NONE);
#else
            mbedtls_ssl_conf_authmode(&ssl_client->ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
#endif

            ret = mbedtls_x509_crt_parse(&ssl_client->ca_cert, (const unsigned char *)ca_cert, strlen(ca_cert) + 1);
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
#else
        mbedtls_ssl_conf_authmode(&ssl_client->ssl_conf, MBEDTLS_SSL_VERIFY_NONE);
#endif

        if (cli_cert != NULL && cli_key != NULL) {
            mbedtls_x509_crt_init(&ssl_client->client_cert);
            mbedtls_pk_init(&ssl_client->client_key);

            DEBUG_CONNECT_VERBOSE("Loading CRT cert %d\r\n", strlen(cli_cert));
            ret = mbedtls_x509_crt_parse(&ssl_client->client_cert, (const unsigned char *)cli_cert, strlen(cli_cert) + 1);
            if (ret < 0) {
            	DEBUG_PRINTF("mbedtls_x509_crt_parse failed! %d\r\n", ret);
                return SOCKETS_SOCKET_ERROR;
            }

            DEBUG_CONNECT_VERBOSE("Loading private key %d\r\n", strlen(cli_key));
            ret = mbedtls_pk_parse_key(&ssl_client->client_key, (const unsigned char *)cli_key, strlen(cli_key) + 1, NULL, 0);
            if (ret != 0) {
            	DEBUG_PRINTF("mbedtls_pk_parse_key failed! %d\r\n", ret);
                return SOCKETS_SOCKET_ERROR;
            }

            ret = mbedtls_ssl_conf_own_cert(&ssl_client->ssl_conf, &ssl_client->client_cert, &ssl_client->client_key);
            if (ret != 0) {
            	DEBUG_PRINTF("mbedtls_ssl_conf_own_cert failed! %d\r\n", ret);
                return SOCKETS_SOCKET_ERROR;
            }
        }

#if 0
        //DEBUG_PRINTF("Setting hostname for TLS session...\r\n");
        // Hostname set here should match CN in server certificate
        if ((ret = mbedtls_ssl_set_hostname(&ssl_client->ssl_ctx, host)) != 0) {
        	DEBUG_PRINTF("mbedtls_ssl_set_hostname failed! %d\r\n", ret);
            return SOCKETS_SOCKET_ERROR;
        }
#endif

        mbedtls_ssl_conf_rng(&ssl_client->ssl_conf, mbedtls_ctr_drbg_random, &ssl_client->drbg_ctx);
        if ((ret = mbedtls_ssl_setup(&ssl_client->ssl_ctx, &ssl_client->ssl_conf)) != 0) {
        	DEBUG_PRINTF("mbedtls_ssl_setup failed! %d\r\n", ret);
            return SOCKETS_SOCKET_ERROR;
        }

        mbedtls_ssl_set_bio(&ssl_client->ssl_ctx, &ssl_client->socket, _TLS_send, _TLS_recv, _TLS_recv_timeout );


        DEBUG_CONNECT_VERBOSE("SSL/TLS handshake\r\n");

        while ((ret = mbedtls_ssl_handshake(&ssl_client->ssl_ctx)) != 0) {
        	DEBUG_PRINTF("mbedtls_ssl_handshake failed! %d 0x%x\r\n", ret, -1*ret);

            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
    			mbedtls_x509_crt_free(&ssl_client->ca_cert);
    			mbedtls_x509_crt_free(&ssl_client->client_cert);
    			mbedtls_pk_free(&ssl_client->client_key);
    			SOCKETS_Close( xSocket );
                return SOCKETS_SOCKET_ERROR;
            }
            vTaskDelay(100);
        }

        DEBUG_CONNECT_VERBOSE("SSL/TLS handshake successful.\r\n");

        if (cli_cert != NULL && cli_key != NULL) {
        	DEBUG_CONNECT_VERBOSE("Protocol is %s Ciphersuite is %s\r\n", mbedtls_ssl_get_version(&ssl_client->ssl_ctx), mbedtls_ssl_get_ciphersuite(&ssl_client->ssl_ctx));
            if ((ret = mbedtls_ssl_get_record_expansion(&ssl_client->ssl_ctx)) < 0) {
            	DEBUG_PRINTF("Record expansion unknown\r\n");
            }
        }

        DEBUG_CONNECT_VERBOSE("Verifying peer X.509 certificate...\r\n");

        if ((flags = mbedtls_ssl_get_verify_result(&ssl_client->ssl_ctx)) != 0) {
    		char buf[512];
            bzero(buf, sizeof(buf));
            mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);

            DEBUG_PRINTF("Failed to verify peer certificate! verification info: %s\r\n", buf);

            SOCKETS_Close( xSocket );
            return SOCKETS_SOCKET_ERROR;
        }
		DEBUG_CONNECT_VERBOSE("Certificate verified.\r\n");

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

		DEBUG_CONNECT("Secure channel created.\r\n");
#endif // USE_TLS

		lRetVal = SOCKETS_ERROR_NONE;
    }

    return lRetVal;
}

/*-----------------------------------------------------------*/

int32_t SOCKETS_Recv( Socket_t xSocket,
                      void * pvBuffer,
                      size_t xBufferLength,
                      uint32_t ulFlags )
{
    int32_t lReceivedBytes = SOCKETS_SOCKET_ERROR;
    uint32_t ulSocketNumber = ( uint32_t ) xSocket; /*lint !e923 cast required for portability. */
    //ESPSecureSocket_t * pxSecureSocket;

    if( xSockets[ ulSocketNumber ].ucInUse )
    {

    	DEBUG_RECV("Receiving data\r\n");

        sslclient_context *ssl_client = &xSockets[ulSocketNumber].sslCtx;
    	struct timeval timeout = {0};
    	timeout.tv_sec = 1;
    	lwip_setsockopt(ssl_client->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

#if USE_TLS
    	lReceivedBytes = mbedtls_ssl_read(&ssl_client->ssl_ctx, pvBuffer, xBufferLength);
#else // USE_TLS
    	lReceivedBytes = lwip_recv(ssl_client->socket, pvBuffer, xBufferLength, 0);
#endif // USE_TLS

        if (lReceivedBytes > 0)
        {
        	DEBUG_RECV("Received %d bytes\r\n", (int)lReceivedBytes);
        }
        else { //if (lReceivedBytes < 0) {
        	if (errno == EBADF) {
        		tfp_printf("Failed to recv data [ret=%d] [errno=%d]\r\n", (int)lReceivedBytes, errno);
        		lReceivedBytes = SOCKETS_SOCKET_ERROR;
        		SOCKETS_Close( xSocket );
        	}
        	else {
        		DEBUG_RECV("Recv timed out\r\n");
        		lReceivedBytes = SOCKETS_ERROR_NONE;
        	}
        }
    }

    return lReceivedBytes;
}

/*-----------------------------------------------------------*/

int32_t SOCKETS_Send( Socket_t xSocket,
                      const void * pvBuffer,
                      size_t xDataLength,
                      uint32_t ulFlags )
{
    uint32_t ulSocketNumber = ( uint32_t ) xSocket; /*lint !e923 cast required for portability. */
    int32_t lSentBytes = SOCKETS_SOCKET_ERROR;

    if( xSockets[ ulSocketNumber ].ucInUse )
    {
    	DEBUG_SEND("Sending data %d\r\n", xDataLength);

        sslclient_context *ssl_client = &xSockets[ulSocketNumber].sslCtx;

#if USE_TLS
        while ((lSentBytes = mbedtls_ssl_write(&ssl_client->ssl_ctx, pvBuffer, xDataLength)) <= 0)
        {
            if (lSentBytes != MBEDTLS_ERR_SSL_WANT_READ && lSentBytes != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                break;
            }
        }
#else // USE_TLS
        lSentBytes = lwip_send(ssl_client->socket, pvBuffer, xDataLength, 0);
#endif // USE_TLS


		if (xDataLength == lSentBytes)
		{
			DEBUG_SEND("Sent %d bytes\r\n", (int)lSentBytes);
			lSentBytes = xDataLength;
		}
		else
		{
			tfp_printf("Failed to send data [ret: %d] [errno=%d]\r\n", (int)lSentBytes, errno);
			lSentBytes = SOCKETS_SOCKET_ERROR;
			SOCKETS_Close( xSocket );
		}
    }
    return lSentBytes;
}
/*-----------------------------------------------------------*/

int32_t SOCKETS_Close( Socket_t xSocket )
{
    int32_t lRetVal = SOCKETS_ERROR_NONE;
    uint32_t ulSocketNumber = ( uint32_t ) xSocket; /*lint !e923 cast required for portability. */
    if( xSockets[ ulSocketNumber ].ucInUse )
    {
    	xSockets[ ulSocketNumber ].ucInUse = 0;
    	if (xSockets[ ulSocketNumber ].pcDestination)
    	{
    		vPortFree(xSockets[ ulSocketNumber ].pcDestination);
    		xSockets[ ulSocketNumber ].pcDestination = NULL;
    	}

        DEBUG_PRINTF("\r\nCleaning SSL connection.\r\n");

        sslclient_context *ssl_client = &xSockets[ulSocketNumber].sslCtx;

#if 0
        do
        {
        	lRetVal = mbedtls_ssl_close_notify(ssl_client);
        } while(lRetVal == MBEDTLS_ERR_SSL_WANT_WRITE);
#endif

		if (ssl_client->socket >= 0)
		{
			lwip_close(ssl_client->socket);
			lwip_shutdown(ssl_client->socket, SHUT_RDWR);
			ssl_client->socket = -1;
		}

#if USE_TLS
		mbedtls_ssl_free(&ssl_client->ssl_ctx);
		mbedtls_ssl_config_free(&ssl_client->ssl_conf);
		mbedtls_ctr_drbg_free(&ssl_client->drbg_ctx);
		mbedtls_entropy_free(&ssl_client->entropy_ctx);
#endif // USE_TLS

    }

    return lRetVal;
}
/*-----------------------------------------------------------*/

int32_t SOCKETS_SetSockOpt( Socket_t xSocket,
                            int32_t lLevel,
                            int32_t lOptionName,
                            const void * pvOptionValue,
                            size_t xOptionLength )
{
    int32_t lRetVal = SOCKETS_ERROR_NONE;
    uint32_t ulSocketNumber = ( uint32_t ) xSocket; /*lint !e923 cast required for portability. */
    ESPSecureSocket_t * pxSecureSocket;
    //uint32_t lTimeout;

    /* Ensure that a valid socket was passed. */
    if( xSockets[ ulSocketNumber ].ucInUse )
    {
        /* Shortcut for easy access. */
        pxSecureSocket = &( xSockets[ ulSocketNumber ] );

        switch( lOptionName )
        {
            case SOCKETS_SO_SERVER_NAME_INDICATION:

                /* Non-NULL destination string indicates that SNI extension should
                 * be used during TLS negotiation. */
                pxSecureSocket->pcDestination = ( char * ) pvPortMalloc( 1U + xOptionLength );

                if( pxSecureSocket->pcDestination == NULL )
                {
                    lRetVal = SOCKETS_ENOMEM;
                }
                else
                {
                    memcpy( pxSecureSocket->pcDestination, pvOptionValue, xOptionLength );
                    pxSecureSocket->pcDestination[ xOptionLength ] = '\0';
                }

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

