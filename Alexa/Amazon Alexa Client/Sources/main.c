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
#include "avs_config.h"
#include "avs/avs.h"
#include "button.h"
#include "time_duration.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#if (COMMUNICATION_IO==1)   // Ethernet
#include "net.h"
#elif (COMMUNICATION_IO==2) // WiFi
#include "net.h"
#include "wifi.h"
#endif // COMMUNICATION_IO



///////////////////////////////////////////////////////////////////////////////////
/* Debug logs */
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else // DEBUG
#define DEBUG_PRINTF(...)
#endif // DEBUG
///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
/* Configurables */
#define USE_RECVPLAY_RESPONSE           1
#define BUTTON_GPIO                     (31)
///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
/* Task Configurations. */
#define TASK_ALEXA_STREAMER_STACK_SIZE  (512+256)
#define TASK_ALEXA_COMMANDER_STACK_SIZE (5120+256)
#define TASK_ALEXA_PRIORITY             (1)
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

#if (COMMUNICATION_IO==1) // Ethernet
static inline void ethernet_setup(void)
{
    net_setup();
}
#elif (COMMUNICATION_IO==2) // WiFi
static inline void wifi_setup(void)
{
    sys_enable(sys_device_uart1);
    gpio_function(52, pad_uart1_txd); /* UART1 TXD MM900EVxA CN3 pin 9 */
    gpio_function(53, pad_uart1_rxd); /* UART1 RXD MM900EVxA CN3 pin 7 */
    gpio_function(54, pad_uart1_rts); /* UART1 RTS MM900EVxA CN3 pin 3 */
    gpio_function(55, pad_uart1_cts); /* UART1 CTS MM900EVxA CN3 pin 11 */
    interrupt_enable_globally();
}
#endif // COMMUNICATION_IO



void vTaskAlexaStreamer(void *pvParameters);
void vTaskAlexaCommander(void *pvParameters);

int main(void)
{
    sys_reset_all();
    interrupt_disable_globally();
    uart_setup();
#if (COMMUNICATION_IO==1)   // Ethernet
    ethernet_setup();
#elif (COMMUNICATION_IO==2) // WiFi
    wifi_setup();
#endif // COMMUNICATION_IO

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

    if (xTaskCreate(vTaskAlexaCommander, "Commander", TASK_ALEXA_COMMANDER_STACK_SIZE,
        NULL, TASK_ALEXA_PRIORITY, NULL) != pdTRUE) {
        DEBUG_PRINTF("vTaskAlexaCommander failed\r\n");
    }
    if (xTaskCreate(vTaskAlexaStreamer, "Streamer", TASK_ALEXA_STREAMER_STACK_SIZE,
        NULL, TASK_ALEXA_PRIORITY, NULL) != pdTRUE) {
        DEBUG_PRINTF("vTaskAlexaStreamer failed\r\n");
    }

    DEBUG_PRINTF("Starting Scheduler.. \r\n");
    vTaskStartScheduler();
    DEBUG_PRINTF("Should never reach here!\r\n");
    for (;;) ;
}

static inline void display_network_info()
{
#if (COMMUNICATION_IO==1)   // Ethernet
    uint8_t* mac = net_get_mac();
    DEBUG_PRINTF( "MAC=%02X:%02X:%02X:%02X:%02X:%02X\r\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );

    ip_addr_t addr = net_get_ip();
    DEBUG_PRINTF( "IP=%s\r\n", inet_ntoa(addr) );
    addr = net_get_gateway();
    DEBUG_PRINTF( "GW=%s\r\n", inet_ntoa(addr) );
    addr = net_get_netmask();
    DEBUG_PRINTF( "MA=%s\r\n", inet_ntoa(addr) );
#elif (COMMUNICATION_IO==2)   // WiFi
    uint8_t ucIPAddr[16] = {0};
    uint8_t ucGateway[16] = {0};
    uint8_t ucMask[16] = {0};
    WIFI_GetIP( ucIPAddr, ucGateway, ucMask );
    DEBUG_PRINTF( "IP=%s\r\n", ucIPAddr );
    DEBUG_PRINTF( "GW=%s\r\n", ucGateway );
    DEBUG_PRINTF( "MA=%s\r\n", ucMask );
#endif
}


static inline void initialize_network()
{
#if (COMMUNICATION_IO==1)   // Ethernet
    int lRet = 0;
    ip_addr_t ip = {AVS_CONFIG_ETHERNET_IP_ADDRESS};
    ip_addr_t gw = {AVS_CONFIG_ETHERNET_GATEWAY};
    ip_addr_t mask = {AVS_CONFIG_ETHERNET_MASK};
    ip_addr_t dns = {AVS_CONFIG_ETHERNET_DNS};

    net_init( ip, gw, mask, AVS_CONFIG_ETHERNET_USE_DHCP, dns, NULL, NULL );

    while ( !net_is_ready() ) {
        vTaskDelay( pdMS_TO_TICKS(1000) );
        DEBUG_PRINTF( "." );
        if (lRet++ > 30) {
            DEBUG_PRINTF( "Could not recover. Do reboot.\r\n" );
            chip_reboot();
        }
    }
    DEBUG_PRINTF( "\r\n" );
#elif (COMMUNICATION_IO==2) // WiFi
    WIFINetworkParams_t xNetworkParams;
    WIFIReturnCode_t xWifiStatus;

    /* Setup WiFi parameters to connect to access point. */
    xNetworkParams.pcSSID = AVS_CONFIG_WIFI_SSID;
    xNetworkParams.ucSSIDLength = sizeof( AVS_CONFIG_WIFI_SSID );
    xNetworkParams.pcPassword = AVS_CONFIG_WIFI_PASSWORD;
    xNetworkParams.ucPasswordLength = sizeof( AVS_CONFIG_WIFI_PASSWORD );
    xNetworkParams.xSecurity = AVS_CONFIG_WIFI_SECURITY;

    xWifiStatus = WIFI_On();
    if( xWifiStatus == eWiFiSuccess ) {
        /* Try connecting using provided wifi credentials. */
        xWifiStatus = eWiFiFailure;
        while(xWifiStatus != eWiFiSuccess) {
            xWifiStatus = WIFI_ConnectAP( &( xNetworkParams ) );
            if(xWifiStatus != eWiFiSuccess) {
                DEBUG_PRINTF("Failed to connect to AP\r\n");
                DEBUG_PRINTF("Re-connecting in 1 seconds %s\r\n", xNetworkParams.pcSSID);
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            else {
                DEBUG_PRINTF("\r\n\r\nWiFi connected to AP %s\r\n", xNetworkParams.pcSSID);
            }
        }
    }
    else {
        tfp_printf("WIFI_On failed!\r\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
#endif
    display_network_info();
}

#if (COMMUNICATION_IO==1)   // Ethernet
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
#endif



////////////////////////////////////////////////////////////////////////////////////////
// Set filenames for request and response audio files
////////////////////////////////////////////////////////////////////////////////////////
//static char* g_pcFileNameRequest = NULL;
#define SEND_COMMAND_RESET  (-1)
#define SEND_COMMAND_RECORD (0)

static char g_cSendCommand = SEND_COMMAND_RESET;
static char g_cRecordVoice = 0;
static char g_cQuit = 0;
static char g_cVolumeIncrease = 0;

static SemaphoreHandle_t g_xMutexRecordPlay;
static char cbRecordVoice(void);
static char cbSendTriggered(void);

enum {
    REQUEST_RECORD,
    REQUEST_TIME,
    REQUEST_MUSIC,
    REQUEST_LIVENEWS,
    REQUEST_ALARM,
    REQUEST_STOP,
    REQUEST_YES,
    REQUEST_PERSON,
    REQUEST_AUDIOBOOK,

    REQUEST_VOLUMEUP,
    REQUEST_VOLUMEDOWN,
    REQUEST_QUIT
};

static const char* g_pcREQUEST[] = {
    "record voice",
    "ask time",
    "play music",
    "play live news",
    "set alarm",
    "stop",
    "yes",
    "ask person",
    "play audio book",

    "increase volume",
    "decrease volume",
    "quit",
};

static const char* g_pcREQUESTfile[] = {
    "REQUEST0.RAW",
    "REQUEST1.RAW",
    "REQUEST2.RAW",
    "REQUEST3.RAW",
    "REQUEST4.RAW",
    "REQUEST5.RAW",
    "REQUEST6.RAW",
    "REQUEST7.RAW",
    "REQUEST8.RAW",
};

static void usage(void)
{
    DEBUG_PRINTF("\r\nUsage:\r\n");
    DEBUG_PRINTF("  Press 'r' to start/stop voice recording.\r\n");
    DEBUG_PRINTF("  Press 't' to ask current time.\r\n");
    DEBUG_PRINTF("  Press 'p' to ask who is Lebron James.\r\n");
    DEBUG_PRINTF("  Press 'm' to play music from TuneIn radio.\r\n");
    DEBUG_PRINTF("  Press 'n' to play live news from Fox News.\r\n");
    DEBUG_PRINTF("  Press 'b' to play audio book from Audible.\r\n");
    DEBUG_PRINTF("  Press 'a' to set alarm in 10 seconds.\r\n");
    DEBUG_PRINTF("  Press 's' to tell stop.\r\n");
    DEBUG_PRINTF("  Press 'y' to tell yes.\r\n");
    DEBUG_PRINTF("  Press '+' to increase volume.\r\n");
    DEBUG_PRINTF("  Press '-' to decrease volume.\r\n");
    DEBUG_PRINTF("  Press 'q' to quit and restart.\r\n");
    DEBUG_PRINTF("\r\n");
}

////////////////////////////////////////////////////////////////////////////////////////
// ISR routine for button
////////////////////////////////////////////////////////////////////////////////////////
void vIsrAlexaButton(void)
{
    uint8_t c = 0;

    if (g_cSendCommand == SEND_COMMAND_RESET) {
        //if (uart_is_interrupted(UART0, uart_interrupt_rx)) {
            uart_read(UART0, &c);
            switch (c) {
                case 'R':
                case 'r': { // mic recording
                    g_cSendCommand = (char)REQUEST_RECORD;
                    if (g_cRecordVoice == 0) {
                        DEBUG_PRINTF("\r\n[start %s]\r\n", g_pcREQUEST[(char)REQUEST_RECORD]);
                        g_cRecordVoice = 1;
                    }
                    break;
                }
                case 'T':
                case 't': { // time
                    g_cSendCommand = (char)REQUEST_TIME;
                    break;
                }
                case 'M':
                case 'm': { // music
                    g_cSendCommand = (char)REQUEST_MUSIC;
                    break;
                }
                case 'N':
                case 'n': { // live news
                    g_cSendCommand = (char)REQUEST_LIVENEWS;
                    break;
                }
                case 'A':
                case 'a': { // alarm
                    g_cSendCommand = (char)REQUEST_ALARM;
                    break;
                }
                case 'S':
                case 's': { // stop
                    g_cSendCommand = (char)REQUEST_STOP;
                    break;
                }
                case 'Y':
                case 'y': { // yes
                    g_cSendCommand = (char)REQUEST_YES;
                    break;
                }
                case 'P':
                case 'p': { // person
                    g_cSendCommand = (char)REQUEST_PERSON;
                    break;
                }
                case 'B':
                case 'b': { // book
                    g_cSendCommand = (char)REQUEST_AUDIOBOOK;
                    break;
                }
                case '+': { // volume up
                    DEBUG_PRINTF("\r\n[%s]\r\n", g_pcREQUEST[(char)REQUEST_VOLUMEUP]);
                    if (g_cVolumeIncrease == 0) {
                        g_cVolumeIncrease = 20;
                    }
                    break;
                }
                case '-': { // volume down
                    DEBUG_PRINTF("\r\n[%s]\r\n", g_pcREQUEST[(char)REQUEST_VOLUMEDOWN]);
                    if (g_cVolumeIncrease == 0) {
                        g_cVolumeIncrease = -20;
                    }
                    break;
                }
                case 'Q':
                case 'q': { // quit
                    DEBUG_PRINTF("\r\n[%s]\r\n", g_pcREQUEST[(char)REQUEST_QUIT]);
                    g_cQuit = 1;
                    break;
                }
                default: {
                    break;
                }
            }
        //}
    }
    else if (g_cSendCommand == SEND_COMMAND_RECORD) {
        //if (uart_is_interrupted(UART0, uart_interrupt_rx)) {
            uart_read(UART0, &c);
            switch (c) {
                case 'R':
                case 'r': { // mic recording
                    g_cSendCommand = SEND_COMMAND_RECORD;
                    if (g_cRecordVoice) {
                        DEBUG_PRINTF("\r\n[stop %s]\r\n", g_pcREQUEST[0]);
                        g_cRecordVoice = 0;
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        //}
    }
    else {
        vTaskDelay( pdMS_TO_TICKS(1) );
    }
}

void vTaskAlexaCommander(void *pvParameters)
{
    int lRet = 0;

    DEBUG_PRINTF("\r\nInitializing network...");
    initialize_network();

    DEBUG_PRINTF("\r\nInitializing AVS...\r\n");
    avs_init();
    vTaskDelay(pdMS_TO_TICKS(1000));

    DEBUG_PRINTF("\r\nInitializing button...\r\n");
    if (!button_setup(vIsrAlexaButton, BUTTON_GPIO)) {
        DEBUG_PRINTF("Error! Connect GPIO%d to GND!\r\n", BUTTON_GPIO);
    }

    g_xMutexRecordPlay = xSemaphoreCreateMutex();

    DEBUG_PRINTF("\r\nInitialization completed!\r\n");
    for (;;)
    {
loop:

#if (COMMUNICATION_IO==1)   // Ethernet
        // Ensure network is ready
        if ( !net_is_ready() ) {
            lRet = 0;
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
#endif

        // Handle quit command and disconnection
        if (g_cQuit) {
            vTaskDelay(pdMS_TO_TICKS(1));

            lRet = 15;
            DEBUG_PRINTF("\r\nRestarting in %d seconds...", lRet);
            for (int i=0; i<lRet; i++) {
                DEBUG_PRINTF(".");
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            DEBUG_PRINTF("\r\n\r\n");
            vTaskDelay(pdMS_TO_TICKS(1));

            g_cQuit = 0;
            g_cSendCommand = SEND_COMMAND_RESET;
        }

        // Handle connection to Alexa provider
        if (!avs_isconnected()) {
            lRet = 0;
            do {
                DEBUG_PRINTF("\r\nConnecting to Alexa provider... %s:%d [id:0x%08x]\r\n",
                    ipaddr_ntoa(avs_get_server_addr()), avs_get_server_port(), avs_get_device_id());
                if (!avs_connect()) {
                    if (avs_err()) {
                        goto loop;
                    }
                    if (++lRet == 30) {
                        chip_reboot();
                    }
                    vTaskDelay(pdMS_TO_TICKS(2000));
                    continue;
                }
                break;
            } while (1);

            DEBUG_PRINTF("\r\nConnected to Alexa provider!\r\n");
            usage();
        }

        // Set volume
        if (g_cVolumeIncrease) {
            avs_set_volume(g_cVolumeIncrease);
            g_cVolumeIncrease = 0;
            DEBUG_PRINTF("\r\nSetting volume [%d%%]...OK\r\n", avs_get_volume());
        }

        // No command is set
        if (g_cSendCommand == SEND_COMMAND_RESET) {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }
        // record audio is set
        else if (g_cSendCommand == SEND_COMMAND_RECORD) {
            if (g_cRecordVoice) {
                xSemaphoreTake(g_xMutexRecordPlay, pdMS_TO_TICKS(portMAX_DELAY));

                DEBUG_PRINTF("\r\nRecording Alexa query [%s]...\r\n", g_pcREQUEST[(int)g_cSendCommand]);
                avs_record_request(g_pcREQUESTfile[(int)g_cSendCommand], cbRecordVoice);
                DEBUG_PRINTF("\r\nRecorded Alexa query [%s]...OK\r\n", g_pcREQUEST[(int)g_cSendCommand]);

                DEBUG_PRINTF("\r\nSending Alexa query [%s]...", g_pcREQUEST[(int)g_cSendCommand]);
                lRet = avs_send_request(g_pcREQUESTfile[(int)g_cSendCommand]);
                if (!lRet) {
                    DEBUG_PRINTF("FAILED!\r\n");
                }
                else {
                    DEBUG_PRINTF("OK\r\n");
                }

                xSemaphoreGive(g_xMutexRecordPlay);
                goto exit;
            }
        }

        // Send command
        xSemaphoreTake(g_xMutexRecordPlay, pdMS_TO_TICKS(portMAX_DELAY));
        DEBUG_PRINTF("\r\nSending Alexa query [%s]...", g_pcREQUEST[(int)g_cSendCommand]);
        lRet = avs_send_request(g_pcREQUESTfile[(int)g_cSendCommand]);
        xSemaphoreGive(g_xMutexRecordPlay);
        if (!lRet) {
            DEBUG_PRINTF("FAILED!\r\n");
        }
        else {
            DEBUG_PRINTF("OK\r\n");
        }

exit:
        // Reset command
        g_cSendCommand = SEND_COMMAND_RESET;
        g_cRecordVoice = 0;
        vTaskDelay(pdMS_TO_TICKS(3000));
    }

    avs_free();
}

void vTaskAlexaStreamer(void *pvParameters)
{
    (void) pvParameters;
    int lRet = 0;


    for (;;)
    {
#if (COMMUNICATION_IO==1)   // Ethernet
        if (!net_is_ready()) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
#endif

        if (!avs_isconnected()) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        while (!g_cQuit) {
            if (g_cSendCommand == SEND_COMMAND_RESET) {
#if USE_RECVPLAY_RESPONSE
                xSemaphoreTake(g_xMutexRecordPlay, pdMS_TO_TICKS(portMAX_DELAY));
                lRet = avs_recv_and_play_response(cbSendTriggered);
                if (!lRet) {
                    xSemaphoreGive(g_xMutexRecordPlay);
                    break;
                }
                xSemaphoreGive(g_xMutexRecordPlay);
#else // USE_RECVPLAY_RESPONSE
                // This method is not ideal for music playback
                lRet = avs_recv_response(g_pcREQUEST[0]);
                if (!lRet) {
                    break;
                }
                else if (lRet > 0) {
                    avs_play_response(g_pcREQUEST[0]);
                }
#endif // USE_RECVPLAY_RESPONSE
                vTaskDelay(pdMS_TO_TICKS(1));
            }
            else {
                vTaskDelay(pdMS_TO_TICKS(500));
            }
        }

        DEBUG_PRINTF("\r\nClosing TCP connection...\r\n");
        if (avs_isconnected()) {
            avs_disconnect();
        }
    }
}

char cbRecordVoice(void)
{
    return g_cRecordVoice;
}

char cbSendTriggered(void)
{
    return (g_cSendCommand != SEND_COMMAND_RESET);
}


