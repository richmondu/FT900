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



#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



/* Task Configurations. */
#define TASK_STATS_STACK_SIZE           (500)           //Task Stack Size
#define TASK_STATS_PRIORITY             (1)             //Task Priority
#define TASK_STATS_PERIOD_MS            (5000)

#define TASK_CLIENT_STACK_SIZE          (4000)          //Task Stack Size
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
// Alexa task
////////////////////////////////////////////////////////////////////////////////////////
void vTaskAlexa(void *pvParameters)
{
    (void) pvParameters;
    int lSocket = 0;
    const char* pcFileNameRequest = "request.raw";
    const char* pcFileNameResponse = "response.raw";


    DEBUG_PRINTF("\r\nInitializing network...\r\n");
    initialize_network();
    DEBUG_PRINTF("Network connected.\r\n\r\n");

    avsInit();
    vTaskDelay(pdMS_TO_TICKS(3000));

    for (;;)
    {
        DEBUG_PRINTF("\r\nConnecting to Alexa provider... %s:%d\r\n",
            ipaddr_ntoa(avsGetServerAddress()), avsGetServerPort());
        if ((lSocket = avsConnect()) < 0) {
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }
        vTaskDelay(pdMS_TO_TICKS(3000));

        DEBUG_PRINTF("\r\nSending Alexa query...\r\n");
        if (avsSendAlexaRequest(lSocket, pcFileNameRequest)) {

            DEBUG_PRINTF("\r\nReceiving Alexa response...\r\n");
            if (avsRecvAlexaResponse(lSocket, pcFileNameResponse)) {

                DEBUG_PRINTF("\r\nPlaying Alexa response...\r\n");
                avsPlayAlexaResponse(pcFileNameResponse);
            }
        }

        DEBUG_PRINTF("\r\nClosing TCP connection...\r\n");
        avsDisconnect(lSocket);

        DEBUG_PRINTF("\r\nRestarting in 15 seconds...");
        for (int i=0; i<15; i++) {
            DEBUG_PRINTF(".");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        DEBUG_PRINTF("\r\n\r\n");
    }

    avsFree();
}


