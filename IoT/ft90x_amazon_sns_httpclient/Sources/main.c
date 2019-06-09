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
#include "amazon_sns_config.h"



///////////////////////////////////////////////////////////////////////////////////
// 1 if send text, 0 if send to topic arn [multiple subscribers: SMS/email/lambda/etc]
#define SNS_IS_TEXT             1
#define SNS_MESSAGE_TO_SEND     "Hello World from FT900!"
///////////////////////////////////////////////////////////////////////////////////



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
            "Welcome to Amazon SNS Example...\r\n\r\n"
            "Demonstrate sending text/email messages via Amazon SNS\r\n"
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

extern int iot_rtc_get_date_stamp(char* pcDate, int iSize);
extern int iot_rtc_get_amz_date(char* pcDate, int iSize);
extern void iot_sntp_start();
extern uint32_t iot_sntp_get_time();


static int generateHash(unsigned char* pcData, int lDataLen, unsigned char* pcHash)
{
    int lRet = 0;
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    lRet = mbedtls_sha256_starts_ret(&ctx, 0);
    if (lRet < 0) {
        DEBUG_PRINTF( "mbedtls_sha256_starts_ret failed!\r\n");
        mbedtls_sha256_free(&ctx);
        return 0;
    }
    lRet = mbedtls_sha256_update_ret(&ctx, (unsigned char *)pcData, lDataLen);
    if (lRet < 0) {
        DEBUG_PRINTF( "mbedtls_sha256_update_ret failed!\r\n");
        mbedtls_sha256_free(&ctx);
        return 0;
    }
    lRet = mbedtls_sha256_finish_ret(&ctx, pcHash);
    if (lRet < 0) {
        DEBUG_PRINTF( "mbedtls_sha256_finish_ret failed!\r\n");
        mbedtls_sha256_free(&ctx);
        return 0;
    }
    mbedtls_sha256_free(&ctx);
    return 1;
}

static int signData(unsigned char* pcData, int lDataLen, char* pcKey, int lKeyLen, unsigned char* pcHash)
{
    int lRet = 0;
    mbedtls_md_context_t md_ctx;
    mbedtls_md_init(&md_ctx);

    if ((lRet = mbedtls_md_setup(&md_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1)) != 0) {
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

static int calculateSignature(unsigned char* pcSignature, int lSignatureSize, unsigned char* pcDateStamp, unsigned char* pcStringToSign)
{
    // signing key
    // getSignatureKey(key, dateStamp, regionName, serviceName):
    unsigned char aucHash[32+1] = {0};
    unsigned char aucKey[48] = {0};
    int lRet = 0;

    lRet = tfp_snprintf(aucKey, sizeof(aucKey), "AWS4%s", CONFIG_AWS_SECRET_KEY);
    if (lRet < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed! aucTemp\r\n");
        return 0;
    }

    lRet = signData(pcDateStamp, strlen(pcDateStamp), aucKey, strlen(aucKey), aucHash);
    if (lRet < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed! aucTemp\r\n");
        return 0;
    }

    lRet = signData(CONFIG_AWS_REGION, strlen(CONFIG_AWS_REGION), aucHash, 32, aucHash);
    if (lRet < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed! aucTemp\r\n");
        return 0;
    }

    lRet = signData(CONFIG_AWS_SERVICE, strlen(CONFIG_AWS_SERVICE), aucHash, 32, aucHash);
    if (lRet < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed! aucTemp\r\n");
        return 0;
    }

    lRet = signData(CONFIG_AWS_SIG4_REQUEST, strlen(CONFIG_AWS_SIG4_REQUEST), aucHash, 32, aucHash);
    if (lRet < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed! aucTemp\r\n");
        return 0;
    }

    lRet = signData(pcStringToSign, strlen(pcStringToSign), aucHash, 32, aucHash);
    if (lRet < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed! aucTemp\r\n");
        return 0;
    }

    for (int i=0; i<32; i++) {
        tfp_snprintf(pcSignature, lSignatureSize, "%s%02x", pcSignature, aucHash[i]);
    }

    return 1;
}

static int generateSignature(
        unsigned char* pcSignature, int lSignatureSize,
        unsigned char* pcRequest, int lRequestLen,
        unsigned char* pcAWSHost, char* pcCredentialScope,
        unsigned char* pcAmzDate, char* pcDateStamp)
{
    int lRet = 0;
    unsigned char aucHash[32+1] = {0};
    unsigned char aucTemp[142] = {0};
    unsigned char aucCanonicalRequest[256] = {0};


    // Request Hash
    lRet = generateHash(pcRequest, lRequestLen, aucHash);
    if (lRet < 0) {
        DEBUG_PRINTF( "generateHash failed!\r\n");
        return 0;
    }

    // Canonical Header
    lRet = tfp_snprintf(aucTemp, sizeof(aucTemp),
        "content-type:%s\nhost:%s\nx-amz-date:%s\n",
        CONFIG_HTTP_CONTENT_TYPE, pcAWSHost, pcAmzDate);
    if (lRet < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed! aucCanonicalHeader\r\n");
        return 0;
    }
    //DEBUG_PRINTF( "CanonicalHeader:%s [%d]\r\n", aucTemp, strlen(aucTemp) );

    // Canonical Request
    lRet = tfp_snprintf(aucCanonicalRequest, sizeof(aucCanonicalRequest),
        "%s\n%s\n\n%s\n%s\n",
        CONFIG_HTTP_METHOD, CONFIG_HTTP_API, aucTemp, CONFIG_AWS_HEADERS);
    if (lRet < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed! aucCanonicalRequest\r\n");
        return 0;
    }
    for (int i=0; i<32; i++) {
        tfp_snprintf(aucCanonicalRequest, sizeof(aucCanonicalRequest), "%s%02x", aucCanonicalRequest, aucHash[i]);
    }
    //DEBUG_PRINTF( "CanonicalRequest:%s [%d]\r\n", aucCanonicalRequest, strlen(aucCanonicalRequest) );

    // Canonical Request Hash
    memset(aucHash, 0, sizeof(aucHash));
    lRet = generateHash(aucCanonicalRequest, strlen(aucCanonicalRequest), aucHash);
    if (lRet < 0) {
        DEBUG_PRINTF( "generateHash failed!\r\n");
        return 0;
    }

    // String to sign
    lRet = tfp_snprintf(aucTemp, sizeof(aucTemp),
        "%s\n%s\n%s\n",
        CONFIG_AWS_ALGORITHM, pcAmzDate, pcCredentialScope);
    if (lRet < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed! aucStringToSign\r\n");
        return 0;
    }
    for (int i=0; i<32; i++) {
        tfp_snprintf(aucTemp, sizeof(aucTemp), "%s%02x", aucTemp, aucHash[i]);
    }
    //DEBUG_PRINTF( "StringToSign:%s [%d]\r\n", aucTemp, strlen(aucTemp) );

    // Calculate signature
    lRet = calculateSignature(pcSignature, lSignatureSize, pcDateStamp, aucTemp);
    if (lRet < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed! aucSignature\r\n");
        return 0;
    }
    //DEBUG_PRINTF( "Signature:%s [%d]\r\n", pcSignature, strlen(pcSignature) );

    return 1;
}

static char* generate_request(char* pcMessageToSend, int lIsText, char* pcPhoneNumber, char* pcTopicArn) {

    int lSize = 0, lSize2 = 0;
    int lRet = 0, lErr = 0;
    unsigned char* pucRequest = NULL;


    if (lIsText) {
        lSize = 57 +strlen(pcPhoneNumber) +strlen(pcMessageToSend) +1;
        pucRequest = (unsigned char*)pvPortMalloc(lSize);
        if (!pucRequest) {
            DEBUG_PRINTF( "pvPortMalloc failed! pucPollyRequest\r\n");
            lErr = 1;
            goto err;
        }
        memset(pucRequest, 0, lSize);
        lRet = tfp_snprintf(pucRequest, lSize,
            "Action=Publish&Version=2010-03-31&PhoneNumber=%s&Message=%s",
            pcPhoneNumber, pcMessageToSend);
        if (lRet < 0) {
            DEBUG_PRINTF( "tfp_snprintf failed! pucRequest\r\n");
            lErr = 1;
            goto err;
        }
    }
    else {
        lSize = 54 +strlen(pcTopicArn) +strlen(pcMessageToSend) +1;
        pucRequest = (unsigned char*)pvPortMalloc(lSize);
        if (!pucRequest) {
            DEBUG_PRINTF( "pvPortMalloc failed! pucPollyRequest\r\n");
            lErr = 1;
            goto err;
        }
        memset(pucRequest, 0, lSize);
        lRet = tfp_snprintf(pucRequest, lSize,
            "Action=Publish&Version=2010-03-31&TopicArn=%s&Message=%s",
            pcTopicArn, pcMessageToSend);
        if (lRet < 0) {
            DEBUG_PRINTF( "tfp_snprintf failed! pucRequest\r\n");
            lErr = 1;
            goto err;
        }
    }
    int lCount = 0;
    for (int i=0; i<lSize; i++) {
        if ( (pucRequest[i]>=0x21 && pucRequest[i]<=0x25) ||
             (pucRequest[i]>=0x27 && pucRequest[i]<=0x2C) || pucRequest[i]==0x2F ||
             (pucRequest[i]>=0x3A && pucRequest[i]<=0x3C) ||
             (pucRequest[i]>=0x3E && pucRequest[i]<=0x40) ||
             (pucRequest[i]>=0x5B && pucRequest[i]<=0x5E) || pucRequest[i]==0x60 ||
             (pucRequest[i]>=0x7B && pucRequest[i]<=0x7E) ) {
            lCount++;
        }
    }
    lSize2 = lSize + lCount*2;
    unsigned char* pucRequest2 = (unsigned char*)pvPortMalloc(lSize2);
    if (!pucRequest2) {
        DEBUG_PRINTF( "pvPortMalloc failed! pucRequest2\r\n");
        lErr = 1;
        goto err;
    }
    memset(pucRequest2, 0, lSize2);

    for (int i=0, j=0; i<lSize; i++, j++) {
        if ( (pucRequest[i]>=0x21 && pucRequest[i]<=0x25) ||
             (pucRequest[i]>=0x27 && pucRequest[i]<=0x2C) || pucRequest[i]==0x2F ||
             (pucRequest[i]>=0x3A && pucRequest[i]<=0x3C) ||
             (pucRequest[i]>=0x3E && pucRequest[i]<=0x40) ||
             (pucRequest[i]>=0x5B && pucRequest[i]<=0x5E) || pucRequest[i]==0x60 ||
             (pucRequest[i]>=0x7B && pucRequest[i]<=0x7E) ) {
            tfp_snprintf(pucRequest2, lSize2, "%s%%%02X", pucRequest2, pucRequest[i]);
            j+=2;
        }
        else {
            pucRequest2[j] = pucRequest[i];
        }
    }
    for (int i=0; i<lSize2; i++) {
        if (pucRequest2[i]==' ') {
            pucRequest2[i] = '+';
        }
    }


err:
    if (pucRequest) {
        vPortFree(pucRequest);
    }
    if (lErr) {
        if (pucRequest2) {
            vPortFree(pucRequest2);
        }
        return NULL;
    }
    return pucRequest2;
}
static int generate_http_request(char* pcPacketBuffer, int lPacketBufferLen, char* pcMessageToSend, int lIsText, char* pcPhoneNumber, char* pcTopicArn)
{
    unsigned char aucDateStamp[8+1] = {0};
    unsigned char aucAmzDate[16+1] ={0};
    int lRet = 0;
    int lErr = 0;
    int lSize = 0;


    //
    // Request
    //
    char* pucRequest = generate_request(pcMessageToSend, lIsText, pcPhoneNumber, pcTopicArn);
    if (!pucRequest) {
        DEBUG_PRINTF( "generate_request failed! pucRequest\r\n");
        lErr = 1;
        goto err;
    }
    //DEBUG_PRINTF( "Request:%s [%d]\r\n", pucRequest, strlen(pucRequest) );


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


    //
    // Generate time/date stamps
    //
#if 1
    iot_rtc_get_date_stamp(aucDateStamp, sizeof(aucDateStamp));
    iot_rtc_get_amz_date(aucAmzDate, sizeof(aucAmzDate));
#else
    tfp_snprintf(aucDateStamp, sizeof(aucDateStamp), "20190607");
    tfp_snprintf(aucAmzDate, sizeof(aucAmzDate), "20190607T033646Z");
#endif


    //
    // Credential scope
    //
    lRet = sizeof(aucDateStamp) + 1 + strlen(CONFIG_AWS_REGION) + 1 + strlen(CONFIG_AWS_SERVICE) + 1 + strlen(CONFIG_AWS_SIG4_REQUEST) + 1;
    unsigned char* pucCredentialScope = (unsigned char*)pvPortMalloc(lRet);
    if (!pucCredentialScope) {
        DEBUG_PRINTF( "pvPortMalloc failed! pucCredentialScope\r\n");
        lErr = 1;
        goto err;
    }
    memset(pucCredentialScope, 0, lRet);
    lRet = tfp_snprintf(pucCredentialScope, lRet, "%s/%s/%s/%s",
        aucDateStamp, CONFIG_AWS_REGION, CONFIG_AWS_SERVICE, CONFIG_AWS_SIG4_REQUEST);
    if (lRet < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed! aucCredentialScope\r\n");
        lErr = 1;
        goto err;
    }
    //DEBUG_PRINTF( "CredentialScope:%s [%d]\r\n", pucCredentialScope, strlen(pucCredentialScope) );


    //
    // Generate signature
    //
    unsigned char aucSignature[64+1] = {0};
    lRet = generateSignature(
        aucSignature, sizeof(aucSignature),
        pucRequest, strlen(pucRequest),
        CONFIG_AWS_HOST, pucCredentialScope,
        aucAmzDate, aucDateStamp
        );
    if (lRet < 0) {
        DEBUG_PRINTF( "generateSignature failed! aucSignature\r\n");
        lErr = 1;
        goto err;
    }

    lRet = tfp_snprintf(pcPacketBuffer, lPacketBufferLen,
        "%s %s HTTP/1.1\r\nHost:%s\r\nContent-Type:%s\r\nX-Amz-Date:%s\r\nAuthorization:%s Credential=%s/%s,SignedHeaders=%s,Signature=%s\r\nContent-Length:%d\r\n\r\n%s\r\n",
        CONFIG_HTTP_METHOD, CONFIG_HTTP_API,
		CONFIG_AWS_HOST,
        CONFIG_HTTP_CONTENT_TYPE,
        aucAmzDate,
        CONFIG_AWS_ALGORITHM, CONFIG_AWS_ACCESS_KEY, pucCredentialScope, CONFIG_AWS_HEADERS, aucSignature,
        strlen(pucRequest),
        pucRequest
        );
    if (lRet < 0) {
        DEBUG_PRINTF( "tfp_snprintf failed! aucStringToSign\r\n");
        lErr = 1;
        goto err;
    }
    DEBUG_PRINTF( "%s [%d]\r\n", pcPacketBuffer, strlen(pcPacketBuffer) );


err:
    if (!pucCredentialScope) {
        vPortFree(pucCredentialScope);
    }
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
    char buffer[512+128] = {0};


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


    /* Initialize socket connection with Amazon SNS */
    Socket_t xSocket = SOCKETS_Socket();
    SocketsSockaddr_t xAddress = {0};
    xAddress.pcServer = CONFIG_AWS_HOST;
    xAddress.usPort = CONFIG_HTTP_TLS_PORT;
    lRet = SOCKETS_Connect( xSocket, &xAddress );
    if (lRet != SOCKETS_ERROR_NONE) {
        DEBUG_PRINTF( "SOCKETS_Connect failed! %d\r\n", lRet );
        return;
    }

    /* Generate request for Amazon SNS */
    iot_sntp_start();
    lRet = generate_http_request(buffer, sizeof(buffer), SNS_MESSAGE_TO_SEND,
        SNS_IS_TEXT, CONFIG_AWS_SNS_PHONE_NUMBER, CONFIG_AWS_SNS_TOPIC_ARN);
    if (lRet < 0) {
        DEBUG_PRINTF( "generate_http_request failed! %d\r\n", lRet );
        return;
    }
    iot_sntp_stop();

    /* Send request to Amazon SNS */
    lRet = SOCKETS_Send( xSocket, buffer, lRet, 0 );
    if (lRet < SOCKETS_ERROR_NONE) {
        DEBUG_PRINTF( "SOCKETS_Send failed! %d\r\n", lRet );
        return;
    }
    DEBUG_PRINTF( "SOCKETS_Send [%d] successful!\r\n\r\n", lRet);

    /* Recv response from Amazon SNS */
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

    /* Close socket connection with Amazon SNS */
    SOCKETS_Close( xSocket );

    for (;;);
}
