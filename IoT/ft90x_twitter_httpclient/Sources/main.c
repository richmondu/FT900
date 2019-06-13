/*
 * ============================================================================
 * History
 * =======
 * 06 Jun 2019 : Created
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
#include "FreeRTOS.h"
#include "net.h"
#include "lwip/sockets.h"
#include "lwip/ip4_addr.h"

#include "mbedtls/md.h"           // For mbedtls_md_xxx
#include "mbedtls/sha256.h"       // For mbedtls_md_xxx
#include "iot_secure_sockets.h"
#include "twitter_config.h"
#include <string.h>
#include <stdio.h>



#define TWITTER_MESSAGE "asdasdasd asdasdasd"



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
#define IOT_APP_TASK_STACK_SIZE                  (1024)
///////////////////////////////////////////////////////////////////////



static void iot_app_task( void *pvParameters );

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
            "Welcome to Twitter Example...\r\n\r\n"
            "Demonstrate posting a tweet to Twitter\r\n"
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

extern uint32_t iot_sntp_get_time();
extern void iot_sntp_start();
extern uint32_t iot_sntp_get_time();



static int signData(unsigned char* pcData, int lDataLen, unsigned char* pcKey, int lKeyLen, unsigned char* pcHash)
{
    int lRet = 0;
    mbedtls_md_context_t md_ctx;
    mbedtls_md_init(&md_ctx);

    if ((lRet = mbedtls_md_setup(&md_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA1), 1)) != 0) {
        DEBUG_PRINTF("HMAC setup failed! returned %d (-0x%04x)\r\n", lRet, -lRet);
        mbedtls_md_free(&md_ctx);
        return 0;
    }

    if ((lRet = mbedtls_md_hmac_starts(&md_ctx, (unsigned char*)pcKey, lKeyLen)) != 0) {
        DEBUG_PRINTF("HMAC starts failed! returned %d (-0x%04x)\r\n", lRet, -lRet);
        mbedtls_md_free(&md_ctx);
        return 0;
    }

    if ((lRet = mbedtls_md_hmac_update(&md_ctx, (unsigned char*)pcData, lDataLen)) != 0) {
        DEBUG_PRINTF("HMAC update failed! returned %d (-0x%04x)\r\n", lRet, -lRet);
        mbedtls_md_free(&md_ctx);
        return 0;
    }
    //DEBUG_PRINTF("mbedtls_md_get_size: %d\r\n", mbedtls_md_get_size(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256)));

    if ((lRet = mbedtls_md_hmac_finish(&md_ctx, pcHash)) != 0) {
        DEBUG_PRINTF("HMAC update failed! returned %d (-0x%04x)\r\n", lRet, -lRet);
        mbedtls_md_free(&md_ctx);
        return 0;
    }
    mbedtls_md_free(&md_ctx);
    return 1;
}

static inline void generate_nonce(unsigned char* pcNonce, int lLen, char* pcTimeStamp)
{
    uint8_t* mac = net_get_mac();
    tfp_snprintf(pcNonce, lLen+1, "%s%02X%02X%02X%02X%02X%s", pcTimeStamp, mac[1], mac[2], mac[3], mac[4], mac[5], pcTimeStamp);
}

static inline void replace_chars(unsigned char* pcStringToSign, int index, int len, char* replacement)
{
    memmove(pcStringToSign+3, pcStringToSign+1, len);
    memcpy(pcStringToSign, replacement, 3);
}

static int base64url_encode(const unsigned char *in, unsigned int inlen, unsigned char *out)
{
    const char base64en[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', '-', '_',
    };
    unsigned int i, j;

    for (i = j = 0; i < inlen; i++) {
        int s = i % 3;
        switch (s) {
            case 0:
                out[j++] = base64en[(in[i] >> 2) & 0x3F];
                continue;
            case 1:
                out[j++] = base64en[((in[i-1] & 0x3) << 4) + ((in[i] >> 4) & 0xF)];
                continue;
            case 2:
                out[j++] = base64en[((in[i-1] & 0xF) << 2) + ((in[i] >> 6) & 0x3)];
                out[j++] = base64en[in[i] & 0x3F];
        }
    }

    i -= 1;
    if ((i % 3) == 0) {
        out[j++] = base64en[(in[i] & 0x3) << 4];
        out[j++] = '=';
        out[j++] = '=';
    } else if ((i % 3) == 1) {
        out[j++] = base64en[(in[i] & 0xF) << 2];
        out[j++] = '=';
    }
    out[j++] = 0;
    return 0;
}

static int url_encode(unsigned char* pcData, int lStart, int lEnd)
{
    for (int i=lStart; i<lEnd; i++) {
        if (pcData[i] == ':') {
            replace_chars(&pcData[i], i, lEnd - i, "%3A");
            i += 2;
            lEnd += 2;
        }
        else if (pcData[i] == '/') {
            replace_chars(&pcData[i], i, lEnd - i, "%2F");
            i += 2;
            lEnd += 2;
        }
        else if (pcData[i] == '=') {
            replace_chars(&pcData[i], i, lEnd - i, "%3D");
            i += 2;
            lEnd += 2;
        }
        else if (pcData[i] == '&') {
            replace_chars(&pcData[i], i, lEnd - i, "%26");
            i += 2;
            lEnd += 2;
        }
        else if (pcData[i] == '%') {
            replace_chars(&pcData[i], i, lEnd - i, "%25");
            i += 2;
            lEnd += 2;
        }
    }

    return lEnd;
}

static int generate_signature(unsigned char* pcSignature, unsigned char* pcNonce, unsigned char* pcTimeStamp, unsigned char* pcRequest)
{
    unsigned char aucStringToSign[384] = {0};
    unsigned char* pucSigningKey = NULL;
    int lSize = 0;
    int lErr = 0;
    int lRet = 0;


    //
    // Generate signing key
    //
    lSize = strlen(CONFIG_TWITTER_CONSUMER_SECRET_KEY) + 1 + strlen(CONFIG_TWITTER_ACCESS_SECRET) + 1;
    pucSigningKey = (unsigned char*)pvPortMalloc(lSize);
    if (!pucSigningKey) {
        DEBUG_PRINTF( "pvPortMalloc failed! pucSigningKey\r\n");
        lErr = 1;
        goto err;
    }
    memset(pucSigningKey, 0, lSize);
    tfp_snprintf(pucSigningKey, lSize, "%s&%s", CONFIG_TWITTER_CONSUMER_SECRET_KEY, CONFIG_TWITTER_ACCESS_SECRET);


    //
    // Generate String to sign
    //
    lRet = tfp_snprintf(aucStringToSign, sizeof(aucStringToSign),
        "%s&https://%s%s&",
        CONFIG_HTTP_METHOD,              // Method
        CONFIG_HOST,                     // Host
        CONFIG_HTTP_API                  // API
        );
    if (lRet < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed!\r\n");
        lErr = 1;
        goto err;
    }
    //DEBUG_PRINTF("Header: %s\r\n", aucStringToSign);
    lSize = url_encode(aucStringToSign, strlen(CONFIG_HTTP_METHOD)+1, strlen(aucStringToSign)-1);
    //DEBUG_PRINTF("Header: %s\r\n", aucStringToSign);
    lRet = lSize;
    // Generate Body
    lSize = tfp_snprintf(aucStringToSign, sizeof(aucStringToSign),
        "%soauth_consumer_key=%s&oauth_nonce=%s&oauth_signature_method=%s&oauth_timestamp=%s&oauth_token=%s&oauth_version=%s&%s",
        aucStringToSign,
        CONFIG_TWITTER_CONSUMER_API_KEY, // oauth_consumer_key
        pcNonce,                         // oauth_nonce
        CONFIG_HTTP_OAUTH_ALGORITHM,     // oauth_signature_method
        pcTimeStamp,                     // oauth_timestamp
        CONFIG_TWITTER_ACCESS_TOKEN,     // oauth_token
        CONFIG_HTTP_OAUTH_VERSION,       // oauth_version
        pcRequest                        // Request
        );
    if (lSize < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed!\r\n");
        lErr = 1;
        goto err;
    }
    //DEBUG_PRINTF("StringToSign: %s\r\n", aucStringToSign);
    lSize = url_encode(aucStringToSign, lRet+1, lSize);
    //DEBUG_PRINTF("StringToSign: %s [%d]\r\n", aucStringToSign, lSize);


    //
    // Generate signature
    //
    unsigned char aucSignature[20+1];
    lRet = signData(aucStringToSign, lSize, pucSigningKey, strlen(pucSigningKey), aucSignature);
    if (lRet == 0) {
        DEBUG_PRINTF( "signData failed!\r\n");
        lErr = 1;
        goto err;
    }
    //DEBUG_PRINTF("Signature: ");
    //for (int i=0; i<20; i++) {
    //    DEBUG_PRINTF("%02x", aucSignature[i]);
    //}
    //DEBUG_PRINTF("\r\n");

    //
    // Generate Base64 signature
    //
    base64url_encode((unsigned char *)aucSignature, 20, pcSignature);
    //DEBUG_PRINTF("Base64 Signature: %s\r\n", pcSignature);

    //
    // Generate URL-encoded Base64 signature
    //
    lSize = url_encode(pcSignature, 0, strlen(pcSignature));
    //DEBUG_PRINTF("URLencoded Base64 Signature: %s\r\n", pcSignature);


err:
    if (!pucSigningKey) {
        vPortFree(pucSigningKey);
    }
    if (lErr) {
        return 0;
    }
    return 1;
}

static int generate_http_request(char* pcPacketBuffer, int lPacketBufferLen, char* pcMessageToSend)
{
    unsigned char aucTimeStamp[10+1] = {0};
    unsigned char aucNonce[30+1] = {0};
    int lRet = 0;
    int lErr = 0;
    int lLen = 0;
    int lSize = 0;


    //
    // Wait for SNTP to complete
    //
    DEBUG_PRINTF("Waiting time request...");
    do {
        vTaskDelay(pdMS_TO_TICKS(1000));
        DEBUG_PRINTF(".");
    }
    while (!iot_sntp_get_time() && net_is_ready());
    DEBUG_PRINTF("done!\r\n\r\n");


    DEBUG_PRINTF("Message: [%s]\r\n", pcMessageToSend);

    //
    // Generate time/date stamps
    //
#if 1
    tfp_snprintf(aucTimeStamp, sizeof(aucTimeStamp), "%d", iot_sntp_get_time());
#else
    tfp_snprintf(aucTimeStamp, sizeof(aucTimeStamp), "1560332973");
#endif
    //DEBUG_PRINTF("Timestamp: %s\r\n", aucTimeStamp);

    //
    // Generate nonce
    //
#if 1
    generate_nonce(aucNonce, 30, aucTimeStamp);
#else
    tfp_snprintf(aucNonce, sizeof(aucNonce), "704004537824812392248604814340");
#endif
    //DEBUG_PRINTF("Nonce: %s\r\n", aucNonce);

    //
    // Generate request
    //
    lLen = strlen(pcMessageToSend);
    lRet = lLen;
    for (int i=0; i<lLen; i++) {
        if (pcMessageToSend[i] == ' ') {
            lRet += 2;
        }
    }
    lSize = strlen("status=") + strlen("&trim_user=1") + lRet + 1;
    unsigned char* pucRequest = (unsigned char*)pvPortMalloc(lSize);
    if (!pucRequest) {
        DEBUG_PRINTF( "pvPortMalloc failed! pucRequest\r\n");
        lErr = 1;
        goto err;
    }
    memset(pucRequest, 0, lSize);
    tfp_snprintf(pucRequest, lSize, "%s", "status=");
    for (int i=0, j=strlen(pucRequest); i<lLen; i++, j++) {
        if (pcMessageToSend[i] == ' ') {
            tfp_snprintf(pucRequest, lSize, "%s%s", pucRequest, "%20");
            j+=2;
        }
        else {
            pucRequest[j] = pcMessageToSend[i];
        }
    }
    tfp_snprintf(pucRequest, lSize, "%s%s", pucRequest, "&trim_user=1");
    //DEBUG_PRINTF("Request: %s\r\n", pucRequest);

    //
    // Generate signature
    //
    unsigned char aucSignature[40+1] = {0};
    lRet = generate_signature(aucSignature, aucNonce, aucTimeStamp, pucRequest);
    if (lRet == 0) {
        DEBUG_PRINTF( "generate_signature failed!\r\n");
        lErr = 1;
        goto err;
    }
    //DEBUG_PRINTF("Signature: %s\r\n", aucSignature);

    //
    // Generate packet
    //
    lRet = tfp_snprintf(pcPacketBuffer, lPacketBufferLen,
        "%s %s HTTP/1.1\r\nConnection:%s\r\nContent-Type:%s\r\nAuthorization:%s oauth_consumer_key=\"%s\",oauth_nonce=\"%s\",oauth_signature=\"%s\",oauth_signature_method=\"%s\",oauth_timestamp=\"%s\",oauth_token=\"%s\",oauth_version=\"%s\"\r\nContent-Length:%d\r\nHost:%s\r\n\r\n%s\r\n",
        CONFIG_HTTP_METHOD,              // Method
        CONFIG_HTTP_API,                 // API
        CONFIG_HTTP_CONNECTION,          // Connection
        CONFIG_HTTP_CONTENT_TYPE,        // Content-Type
        CONFIG_HTTP_AUTHORIZATION,       // Authorization
        CONFIG_TWITTER_CONSUMER_API_KEY, // oauth_consumer_key
        aucNonce,                        // oauth_nonce
        aucSignature,                    // oauth_signature
        CONFIG_HTTP_OAUTH_ALGORITHM,     // oauth_signature_method
        aucTimeStamp,                    // oauth_timestamp
        CONFIG_TWITTER_ACCESS_TOKEN,     // oauth_token
        CONFIG_HTTP_OAUTH_VERSION,       // oauth_version
        strlen(pucRequest),              // Content-Length
        CONFIG_HOST,                     // Host
        pucRequest                       // Request
        );
    if (lRet < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed! aucStringToSign\r\n");
        lErr = 1;
        goto err;
    }
    DEBUG_PRINTF( "\r\n%s [%d]\r\n\r\n", pcPacketBuffer, strlen(pcPacketBuffer) );


err:
    if (!pucRequest) {
        vPortFree(pucRequest);
    }
    if (lErr) {
        return 0;
    }

    return lRet;
}


static void iot_app_task( void *pvParameters )
{
    (void) pvParameters;
    int lRet = 0;
    char buffer[512] = {0};


    /* Initialize network */
    net_init( ip, gateway, mask, USE_DHCP, dns, NULL, NULL );

    /* Wait until network is ready then display network info */
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
    DEBUG_PRINTF( "\r\n\r\n" );

    /* Initialize socket connection with Twitter */
    Socket_t xSocket = SOCKETS_Socket();
    SocketsSockaddr_t xAddress = {0};
    xAddress.pcServer = CONFIG_HOST;
    xAddress.usPort = CONFIG_PORT;
    lRet = SOCKETS_Connect( xSocket, &xAddress );
    if (lRet != SOCKETS_ERROR_NONE) {
        DEBUG_PRINTF( "SOCKETS_Connect failed! %d\r\n", lRet );
        return;
    }
    iot_sntp_start();

    /* Generate request for Twitter */
    lRet = generate_http_request(buffer, sizeof(buffer), TWITTER_MESSAGE);
    if (lRet < 0) {
        DEBUG_PRINTF( "generate_http_request failed! %d\r\n", lRet );
        return;
    }

    /* Send request to Amazon Lambda */
    lRet = SOCKETS_Send( xSocket, buffer, lRet, 0 );
    if (lRet < SOCKETS_ERROR_NONE) {
        DEBUG_PRINTF( "SOCKETS_Send failed! %d\r\n", lRet );
        return;
    }
    DEBUG_PRINTF( "SOCKETS_Send [%d] successful!\r\n\r\n", lRet);

    /* Recv response from Twitter */
    int lFirst = 1;
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        lRet = SOCKETS_Recv( xSocket, buffer, sizeof(buffer), 0 );
        if (lRet > 0) {
            if (lFirst) {
                DEBUG_PRINTF( "SOCKETS_Recv [%d]\r\n%s\r\n", lRet, buffer);
                lFirst = 0;
            }
            else {
                DEBUG_PRINTF( "SOCKETS_Recv [%d]\r\n", lRet);
            }
        }
        else if (lRet < 0) {
            DEBUG_PRINTF( "SOCKETS_Recv failed! %d\r\n", lRet );
            break;
        }
    }

    /* Close socket connection with Twitter */
    iot_sntp_stop();
    SOCKETS_Close( xSocket );

    for (;;);
}
