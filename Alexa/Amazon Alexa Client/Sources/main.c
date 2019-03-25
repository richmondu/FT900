/**
  @file main.c
  @brief
  Amazon Alexa Client.

 */
/*
 * ============================================================================
 * History
 * =======
 * 2019-02-14 : Created v1
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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "ft900.h"
#include "tinyprintf.h"
#include "net.h"
#include "task.h"
#include "avs/avs.h"
#include "button.h"
#include "time_duration.h"



///////////////////////////////////////////////////////////////////////////////////
/* Default network configuration. */
#define USE_DHCP 1       // 1: Dynamic IP, 0: Static IP
static ip_addr_t ip      = IPADDR4_INIT_BYTES( 0, 0, 0, 0 );
static ip_addr_t gateway = IPADDR4_INIT_BYTES( 0, 0, 0, 0 );
static ip_addr_t mask    = IPADDR4_INIT_BYTES( 0, 0, 0, 0 );
static ip_addr_t dns     = IPADDR4_INIT_BYTES( 0, 0, 0, 0 );
///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
/* Configurables */
#define USE_TEST_MODE                   1
#define USE_RECVPLAY_RESPONSE           1
#define USE_MEASURE_PERFORMANCE         1
#define USE_PLAY_RECORDED_AUDIO         0
#define BUTTON_GPIO                     (31)
///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
/* Task Configurations. */
#define TASK_ALEXA_STACK_SIZE           (6144)
#define TASK_ALEXA_PRIORITY             (1)
///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
/* Debug logs */
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else // DEBUG
#define DEBUG_PRINTF(...)
#endif // DEBUG
///////////////////////////////////////////////////////////////////////////////////



void myputc(void* p, char c)
{
    uart_write((ft900_uart_regs_t*) p, (uint8_t) c);
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

void vTaskAlexa(void *pvParameters);

int main(void)
{
    sys_reset_all();
    interrupt_disable_globally();
    uart_setup();
    ethernet_setup();

#if USE_MEASURE_PERFORMANCE
    time_duration_setup();
#endif // USE_MEASURE_PERFORMANCE


    /* Print out a welcome message... */
    uart_puts(UART0,
        "\x1B[2J" /* ANSI/VT100 - Clear the Screen */
        "\x1B[H" /* ANSI/VT100 - Move Cursor to Home */
        "Copyright (C) Bridgetek Pte Ltd \r\n"
        "--------------------------------------------------------------------- \r\n"
        "Welcome to Amazon Alexa Application... \r\n"
        "\r\n"
        "Demonstrate Amazon Alexa.\r\n"
        "--------------------------------------------------------------------- \r\n");

    if (xTaskCreate(vTaskAlexa,
        "Alexa",
        TASK_ALEXA_STACK_SIZE,
        NULL,
        TASK_ALEXA_PRIORITY,
        NULL) != pdTRUE) {
        DEBUG_PRINTF("Client failed\r\n");
    }

    DEBUG_PRINTF("Starting Scheduler.. \r\n");
    vTaskStartScheduler();
    DEBUG_PRINTF("Should never reach here!\r\n");
    for (;;)
        ;
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

static inline void initialize_network()
{
    int lRet = 0;

    net_init( ip, gateway, mask, USE_DHCP, dns, NULL, NULL );
    while ( !net_is_ready() ) {
        vTaskDelay( pdMS_TO_TICKS(1000) );
        DEBUG_PRINTF( "." );
        if (lRet++ > 30) {
            DEBUG_PRINTF( "Could not recover. Do reboot.\r\n" );
            chip_reboot();
        }
    }
    vTaskDelay( pdMS_TO_TICKS(1000) );
    DEBUG_PRINTF( "\r\n" );
    display_network_info();
}

#if !NET_USE_EEPROM
void net_supply_mac(uint8_t *mac)
{
    mac[0] = 0x44;
    mac[1] = 0x6D;
    mac[2] = 0x57;
    mac[3] = 0xAA;
    mac[4] = 0xBB;
    mac[5] = 0xCC;
}
#endif // NET_USE_EEPROM



////////////////////////////////////////////////////////////////////////////////////////
// Set filenames for request and response audio files
////////////////////////////////////////////////////////////////////////////////////////
#define STR_REQUEST  "REQUEST.raw"
#define STR_RESPONSE "RESPONSE.raw"

#if USE_TEST_MODE
////////////////////////////////////////////////////////////////////////////////////////
// Alexa task
// FYI: Modify avs_config.h for IP address, audio sampling rate, etc.
////////////////////////////////////////////////////////////////////////////////////////
void vTaskAlexa(void *pvParameters)
{
    (void) pvParameters;
    char* acFileNameRequest = STR_REQUEST;
#if !USE_RECVPLAY_RESPONSE
    char* acFileNameResponse = STR_RESPONSE;
#endif
    int lRet = 0;


    DEBUG_PRINTF("\r\nInitializing network...");
    initialize_network();

    DEBUG_PRINTF("\r\nInitializing AVS...\r\n");
    avs_init();
    vTaskDelay(pdMS_TO_TICKS(1000));


    for (;;)
    {
loop:
        lRet = 0;
        if ( !net_is_ready() ) {
            DEBUG_PRINTF( "Waiting for network configuration..." );
            do {
                vTaskDelay( pdMS_TO_TICKS(1000) );
                DEBUG_PRINTF( "." );
                if (lRet++ > 30) {
                    DEBUG_PRINTF( "Could not recover. Do reboot.\r\n" );
                    chip_reboot();
                }
            }
            while (!net_is_ready());
            DEBUG_PRINTF( "\r\n" );
            display_network_info();
        }


#if USE_PLAY_RECORDED_AUDIO
        //DEBUG_PRINTF("\r\nPlaying Alexa request...\r\n");
        //avs_play_response(acFileNameRequest);
#endif // USE_PLAY_RECORDED_AUDIO

        lRet = 0;
        do {
            DEBUG_PRINTF("\r\nConnecting to Alexa provider... %s:%d\r\n",
                ipaddr_ntoa(avs_get_server_addr()), avs_get_server_port());
            if (!avs_connect()) {
                if (avs_err()) {
                    goto loop;
                }
                vTaskDelay(pdMS_TO_TICKS(2000));
                if (++lRet == 30) {
                    chip_reboot();
                }
                continue;
            }
            break;
        } while (1);


#if USE_MEASURE_PERFORMANCE
        struct tm time0 = {0};
        struct tm time1 = {0};
#if !USE_RECVPLAY_RESPONSE
        struct tm time2 = {0};
#endif // USE_RECVPLAY_RESPONSE
        struct tm timeX = {0};
        time_duration_get_time(&time0);

        DEBUG_PRINTF("\r\nSending Alexa query...");
        if (avs_send_request(acFileNameRequest)) {
            time_duration_get_time(&time1);

#if USE_RECVPLAY_RESPONSE
            DEBUG_PRINTF("[%d seconds]\r\nStreaming Alexa response...", (int)time_duration_get(&time1, &time0));
            if (avs_recv_and_play_response()) {
                time_duration_get_time(&timeX);
                DEBUG_PRINTF("[%d seconds]\r\n", (int)time_duration_get(&timeX, &time1));
            }
            else {
                time_duration_get_time(&timeX);
                DEBUG_PRINTF("[%d seconds] TIMEDOUT\r\n", (int)time_duration_get(&timeX, &time1));
            }
#else // USE_RECVPLAY_RESPONSE
            DEBUG_PRINTF("[%d seconds]\r\nReceiving Alexa response...", (int)time_duration_get(&time1, &time0));
            if (avs_recv_response(acFileNameResponse)) {
                time_duration_get_time(&time2);

                DEBUG_PRINTF("[%d seconds]\r\nPlaying Alexa response...", (int)time_duration_get(&time2, &time1));
                avs_play_response(acFileNameResponse);
                time_duration_get_time(&timeX);
                DEBUG_PRINTF("[%d seconds]\r\n", (int)time_duration_get(&timeX, &time2));
            }
            else {
                time_duration_get_time(&timeX);
                DEBUG_PRINTF("[%d seconds] TIMEDOUT\r\n", (int)time_duration_get(&timeX, &time1));
            }
#endif // USE_RECVPLAY_RESPONSE
        }

        DEBUG_PRINTF("Performance: %d seconds\r\n", (int)time_duration_get(&timeX, &time0));
#else // USE_MEASURE_PERFORMANCE
        DEBUG_PRINTF("\r\nSending Alexa query...\r\n");
        if (avs_send_request(acFileNameRequest)) {

            DEBUG_PRINTF("\r\nReceiving Alexa response...\r\n");
            if (avs_recv_response(acFileNameResponse)) {

                DEBUG_PRINTF("\r\nPlaying Alexa response...\r\n");
                avs_play_response(acFileNameResponse);
            }
        }
#endif // USE_MEASURE_PERFORMANCE


        DEBUG_PRINTF("\r\nClosing TCP connection...\r\n");
        avs_disconnect();

        lRet = 10;
        DEBUG_PRINTF("\r\nRestarting in %d seconds...", lRet);
        for (int i=0; i<lRet; i++) {
            DEBUG_PRINTF(".");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        DEBUG_PRINTF("\r\n\r\n");
    }

    avs_free();
}
#else // USE_TEST_MODE

static uint8_t g_lRecordAudio = 0;
static uint8_t g_lProcessAudio = 0;
int record_audio(void);
void vIsrAlexaButton(void);

////////////////////////////////////////////////////////////////////////////////////////
// Alexa task
// FYI: Modify avs_config.h for IP address, audio sampling rate, etc.
////////////////////////////////////////////////////////////////////////////////////////
void vTaskAlexa(void *pvParameters)
{
    (void) pvParameters;
    const char* acFileNameRequest = STR_REQUEST;
#if !USE_RECVPLAY_RESPONSE
    const char* acFileNameResponse = STR_RESPONSE;
#endif // USE_RECVPLAY_RESPONSE
    int lRet = 0;


    DEBUG_PRINTF("\r\nInitializing network...");
    initialize_network();

    DEBUG_PRINTF("\r\nInitializing button...\r\n");
    if (!button_setup(vIsrAlexaButton, BUTTON_GPIO)) {
        DEBUG_PRINTF("Error! Connect GPIO%d to GND!\r\n", BUTTON_GPIO);
    }

    DEBUG_PRINTF("\r\nInitializing AVS...\r\n");
    avs_init();

    DEBUG_PRINTF("\r\nWaiting for button event...\r\n");
    for (;;)
    {
        // Record audio from microphone and save to SD card
        if (g_lRecordAudio) {
            DEBUG_PRINTF("\r\nRecording audio from microphone to SD card...\r\n");

            // Record audio from microphone and save to SD card
            if (avs_record_request(acFileNameRequest, record_audio)) {
                g_lProcessAudio = 1;
                DEBUG_PRINTF("\r\nRecording audio completed!\r\n");

#if USE_PLAY_RECORDED_AUDIO
                DEBUG_PRINTF("\r\nPlaying Alexa request...\r\n");
                avs_play_response(acFileNameRequest);
#endif // USE_PLAY_RECORDED_AUDIO
            }

            g_lRecordAudio = 0;
        }

        // Process the recorded audio in SD card
        if (g_lProcessAudio) {
            lRet = 0;
            if ( !net_is_ready() ) {
                DEBUG_PRINTF("\r\nWaiting for network configuration...");
                do {
                    vTaskDelay( pdMS_TO_TICKS(1000) );
                    DEBUG_PRINTF( "." );
                    if (lRet++ > 30) {
                        DEBUG_PRINTF( "Could not recover. Do reboot.\r\n" );
                        chip_reboot();
                    }
                }
                while (!net_is_ready());
                DEBUG_PRINTF("\r\n");
                display_network_info();
            }

            DEBUG_PRINTF("\r\nConnecting to Alexa provider... %s:%d\r\n",
                ipaddr_ntoa(avs_get_server_addr()), avs_get_server_port());
            if (!avs_connect()) {
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }

#if USE_RECVPLAY_RESPONSE
            DEBUG_PRINTF("\r\nSending Alexa query...\r\n");
            if (avs_send_request(acFileNameRequest)) {

                DEBUG_PRINTF("Streaming Alexa response...\r\n");
                avs_recv_and_play_response();
            }
#else // USE_RECVPLAY_RESPONSE
            DEBUG_PRINTF("\r\nSending Alexa query...\r\n");
            if (avs_send_request(acFileNameRequest)) {

                DEBUG_PRINTF("Receiving Alexa response...\r\n");
                if (avs_recv_response(acFileNameResponse)) {

                    DEBUG_PRINTF("Playing Alexa response...\r\n");
                    avs_play_response(acFileNameResponse);
                }
            }
#endif // USE_RECVPLAY_RESPONSE

            DEBUG_PRINTF("\r\nClosing TCP connection...\r\n");
            avs_disconnect();

            vTaskDelay(pdMS_TO_TICKS(3000));
            g_lProcessAudio = 0;
            DEBUG_PRINTF("\r\nWaiting for button event...\r\n");
        }
    }

    avs_free();
}

////////////////////////////////////////////////////////////////////////////////////////
// ISR routine for button
////////////////////////////////////////////////////////////////////////////////////////
void vIsrAlexaButton(void)
{
    if (!g_lProcessAudio) {
        if (button_is_interrupted()) {
            if (button_is_pressed()) {
                g_lRecordAudio = 1;
                DEBUG_PRINTF("\r\nButton is pressed!\r\n");
            }
            else if (button_is_released()) {
                g_lRecordAudio = 0;
                DEBUG_PRINTF("\r\nButton is released!\r\n");
            }
        }
    }
}

int record_audio(void)
{
    return (int)g_lRecordAudio;
}
#endif // USE_TEST_MODE


