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
#include "ft900.h"
#include "tinyprintf.h"
#include "net.h"
#include "task.h"
#include "avs/avs.h"
#include "button.h"



#define TEST_MODE 1



#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



#define BUTTON_GPIO  (31)

/* Task Configurations. */
#define TASK_STATS_STACK_SIZE           (500)           //Task Stack Size
#define TASK_STATS_PRIORITY             (1)             //Task Priority
#define TASK_STATS_PERIOD_MS            (5000)

#define TASK_CLIENT_STACK_SIZE          (5000)          //Task Stack Size
#define TASK_CLIENT_PRIORITY            (1)             //Task Priority
#define TASK_CLIENT_PERIOD_MS           (100)

#define TASK_NOTIFY_NETIF_UP            0x01
#define TASK_NOTIFY_LINK_UP             0x02
#define TASK_NOTIFY_LINK_DOWN           0x03

static TaskHandle_t gx_Task_Handle;
void vTaskConnect(void *pvParameters);
void vTaskAlexa(void *pvParameters);



void myputc(void* p, char c)
{
    uart_write((ft900_uart_regs_t*) p, (uint8_t) c);
}

int main(void)
{
    sys_reset_all();
    interrupt_disable_globally();

    /* enable uart */
    sys_enable(sys_device_uart0);
    gpio_function(48, pad_func_3);
    gpio_function(49, pad_func_3);

    uart_open(UART0, 1,
        UART_DIVIDER_9600_BAUD,
        uart_data_bits_8,
        uart_parity_none,
        uart_stop_bits_1);

    /* Enable tfp_printf() functionality... */
    init_printf(UART0, myputc);

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
        TASK_CLIENT_STACK_SIZE,
        NULL,
        TASK_CLIENT_PRIORITY,
        &gx_Task_Handle) != pdTRUE) {
        DEBUG_PRINTF("Client failed\r\n");
    }

    DEBUG_PRINTF("Starting Scheduler.. \r\n");
    vTaskStartScheduler();
    DEBUG_PRINTF("Should never reach here!\r\n");
    for (;;)
        ;
}

////////////////////////////////////////////////////////////////////////////////////////
// Network task
////////////////////////////////////////////////////////////////////////////////////////
void vTaskConnect(void *pvParameters)
{
    while (1) {
        // Check for ethernet disconnection.
        if (net_is_link_up()) {
            if (!ethernet_is_link_up()) {
                net_set_link_down();
                DEBUG_PRINTF("Ethernet disconnected.\r\n");
            }
        }
        else {
            if (ethernet_is_link_up()) {
                DEBUG_PRINTF("Ethernet connected.\r\n");
                net_set_link_up();
            }
        }
        for (int i = 0; i < 10; i++) {
            net_tick();/* 10 ms delay */
            vTaskDelay(1 * portTICK_PERIOD_MS);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// Network init callback
////////////////////////////////////////////////////////////////////////////////////////
static void net_init_callback(int netif_up, int link_up, int packet_available)
{
    if (netif_up) {
        xTaskNotify(gx_Task_Handle,TASK_NOTIFY_NETIF_UP,eSetBits);
    }
    if (link_up) {
        xTaskNotify(gx_Task_Handle,TASK_NOTIFY_LINK_UP,eSetBits);
    }
    else {
        xTaskNotify(gx_Task_Handle,TASK_NOTIFY_LINK_DOWN,eSetBits);
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// Network initialization
////////////////////////////////////////////////////////////////////////////////////////
static inline void initialize_network()
{
    uint32_t ulNotifiedValue;
    ip_addr_t ip_addr = {0};
    ip_addr_t gw_addr = {0};
    ip_addr_t net_mask = {0};


    net_init(ip_addr, gw_addr, net_mask, 1, "FT90x_Alexa", net_init_callback);

    while (xTaskNotifyWait( pdFALSE,    /* Don't clear bits on entry. */
            TASK_NOTIFY_NETIF_UP, /* Clear all bits on exit. */
            &ulNotifiedValue, /* Stores the notified value. */
            portMAX_DELAY ) == false);

    if (xTaskCreate(vTaskConnect,
            "Connect",
            TASK_STATS_STACK_SIZE,
            NULL,
            TASK_STATS_PRIORITY,
            NULL) != pdTRUE) {
        DEBUG_PRINTF("Connect Monitor failed\n");
    }

    while (xTaskNotifyWait( pdTRUE,    /* Don't clear bits on entry. */
            TASK_NOTIFY_LINK_UP, /* Clear all bits on exit. */
            &ulNotifiedValue, /* Stores the notified value. */
            portMAX_DELAY ) == false);
}

////////////////////////////////////////////////////////////////////////////////////////
// Set filenames for request and response audio files
////////////////////////////////////////////////////////////////////////////////////////
#define STR_REQUEST  "REQUEST"
#define STR_RESPONSE "RESPONSE"
static void set_filenames(char *pcRequest, int lRequestSize, char *pcResponse, int lResponseSize, int lCounter)
{
    tfp_snprintf( pcRequest,  lRequestSize,  "%s.raw", STR_REQUEST);
    tfp_snprintf( pcResponse, lResponseSize, "%s.raw", STR_RESPONSE);
}



#if TEST_MODE
////////////////////////////////////////////////////////////////////////////////////////
// Alexa task
// FYI: Modify avs_config.h for IP address, audio sampling rate, etc.
////////////////////////////////////////////////////////////////////////////////////////
void vTaskAlexa(void *pvParameters)
{
    (void) pvParameters;
    int lCounter = 0;
    char acFileNameRequest[32] = {0};
    char acFileNameResponse[32] = {0};


    DEBUG_PRINTF("\r\nInitializing network...\r\n");
    initialize_network();
    DEBUG_PRINTF("Network connected.\r\n");

    DEBUG_PRINTF("\r\nInitializing AVS...\r\n");
    avs_init();
    set_filenames(acFileNameRequest, sizeof(acFileNameRequest), acFileNameResponse, sizeof(acFileNameResponse), ++lCounter);
    vTaskDelay(pdMS_TO_TICKS(1000));

    for (;;)
    {
        //DEBUG_PRINTF("\r\nPlaying Alexa request...\r\n");
        //avs_play_response(acFileNameRequest);

        int lTrials = 0;
        do {
            DEBUG_PRINTF("\r\nConnecting to Alexa provider... %s:%d\r\n",
                ipaddr_ntoa(avs_get_server_addr()), avs_get_server_port());
            if (!avs_connect()) {
                vTaskDelay(pdMS_TO_TICKS(2000));
                if (++lTrials == 30) {
                    chip_reboot();
                }
                continue;
            }
            vTaskDelay(pdMS_TO_TICKS(3000));
            break;
        } while (1);

        DEBUG_PRINTF("\r\nSending Alexa query...\r\n");
        if (avs_send_request(acFileNameRequest)) {

            DEBUG_PRINTF("\r\nReceiving Alexa response...\r\n");
            if (avs_recv_response(acFileNameResponse)) {

                DEBUG_PRINTF("\r\nPlaying Alexa response...\r\n");
                avs_play_response(acFileNameResponse);
            }
        }

        DEBUG_PRINTF("\r\nClosing TCP connection...\r\n");
        avs_disconnect();

        int lRestart = 10;
        DEBUG_PRINTF("\r\nRestarting in %d seconds...", lRestart);
        for (int i=0; i<lRestart; i++) {
            DEBUG_PRINTF(".");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        DEBUG_PRINTF("\r\n\r\n");
    }

    avs_free();
}
#else // TEST_MODE

static int g_lRecordAudio = 0;
static int g_lProcessAudio = 0;
int record_audio(void);
void vIsrAlexaButton(void);

////////////////////////////////////////////////////////////////////////////////////////
// Alexa task
// FYI: Modify avs_config.h for IP address, audio sampling rate, etc.
////////////////////////////////////////////////////////////////////////////////////////
void vTaskAlexa(void *pvParameters)
{
    (void) pvParameters;
    int lCounter = 0;
    char acFileNameRequest[32] = {0};
    char acFileNameResponse[32] = {0};


    DEBUG_PRINTF("\r\nInitializing network...\r\n");
    initialize_network();
    DEBUG_PRINTF("Network connected.\r\n");

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

            set_filenames(acFileNameRequest, sizeof(acFileNameRequest), acFileNameResponse, sizeof(acFileNameResponse), ++lCounter);

            // Record audio from microphone and save to SD card
            if (avs_record_request(acFileNameRequest, record_audio)) {
                g_lProcessAudio = 1;
                DEBUG_PRINTF("\r\nRecording audio completed!\r\n");
            }

            g_lRecordAudio = 0;
        }

        // Process the recorded audio in SD card
        if (g_lProcessAudio) {
            DEBUG_PRINTF("\r\nConnecting to Alexa provider... %s:%d\r\n",
                ipaddr_ntoa(avs_get_server_addr()), avs_get_server_port());
            if (!avs_connect()) {
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }
            vTaskDelay(pdMS_TO_TICKS(3000));

            DEBUG_PRINTF("\r\nSending Alexa query...\r\n");
            if (avs_send_request(acFileNameRequest)) {

                DEBUG_PRINTF("\r\nReceiving Alexa response...\r\n");
                if (avs_recv_response(acFileNameResponse)) {

                    DEBUG_PRINTF("\r\nPlaying Alexa response...\r\n");
                    avs_play_response(acFileNameResponse);
                }
            }

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
    return g_lRecordAudio;
}
#endif // TEST_MODE


