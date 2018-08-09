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

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ft900.h"
#include "tinyprintf.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* LWIP includes. */
#include "lwipopts.h"
#include "lwip/ip_addr.h"
#include "iot.h"
#include "iot_config.h"
#include "net.h"

/* MQTT includes. */
#include "aws_bufferpool.h"
#include "aws_mqtt_agent.h"



/* Macro Definitions ---------------------------------------------------------*/

#define TASK_NOTIFY_NETIF_UP        (0x01)
#define TASK_NOTIFY_LINK_UP         (0x02)
#define TASK_NOTIFY_LINK_DOWN       (0x04)
#define TASK_NOTIFY_PACKET          (0x08)

// Do not modify unless you know what you are doing
// These are currently the most optimal stack size for this application
#define TASK_SYSTEM_STACK_SIZE      (208)
#define TASK_SYSTEM_PRIORITY        (configMAX_PRIORITIES-1)
#define TASK_MQTTAPP_STACK_SIZE     (208)
#define TASK_MQTTAPP_PRIORITY       (tskIDLE_PRIORITY + 1)

/* Private variables ---------------------------------------------------------*/

TaskHandle_t g_hSystemTask = NULL;
TaskHandle_t g_hMQTTAppTask = NULL;

/*-----------------------------------------------------------*/



/*-----------------------------------------------------------*/

static void connectionCB(int netif_up, int link_up, int packet_available)
{
    if (packet_available) {
        if (g_hSystemTask) {
            xTaskNotify(g_hSystemTask, TASK_NOTIFY_PACKET, eSetBits);
            return;
        }
    }

    if (netif_up) {
        if (g_hSystemTask) {
            xTaskNotify(g_hSystemTask,TASK_NOTIFY_NETIF_UP,eSetBits);
        }
    }

    if (link_up) {
        if (g_hSystemTask) {
            xTaskNotify(g_hSystemTask,TASK_NOTIFY_LINK_UP,eSetBits);
        }
    }
    else {
        if (g_hSystemTask) {
            xTaskNotify(g_hSystemTask,TASK_NOTIFY_LINK_DOWN,eSetBits);
        }
    }
}

/*-----------------------------------------------------------*/

static inline void initializeApplication(iot_task task)
{
    BaseType_t xResult = pdPASS;


    xResult = MQTT_AGENT_Init();
    if ( xResult == pdPASS ) {
        xResult = BUFFERPOOL_Init();
    }
    if ( xResult == pdPASS ) {
        xTaskCreate(task, "MQTTAPP", TASK_MQTTAPP_STACK_SIZE,
            NULL, TASK_MQTTAPP_PRIORITY, &g_hMQTTAppTask );
    }
}

static void vTaskSystem(void* params)
{

    // Initialize Ethernet
    {
        ip_addr_t ipaddr, gateway, subnet;
        ipaddr.addr = IOT_CONFIG_IP_ADDRESS;
        gateway.addr = IOT_CONFIG_IP_GATEWAY;
        subnet.addr = IOT_CONFIG_IP_SUBNET;


        net_init(ipaddr, gateway, subnet, LWIP_DHCP, NULL, connectionCB);

        while (xTaskNotifyWait(pdFALSE, TASK_NOTIFY_NETIF_UP, NULL, portMAX_DELAY ) == false);

        while (1) {
            if (!netif_is_link_up(net_get_netif())) {
                if (ethernet_is_link_up()) {
                    netif_set_link_up(net_get_netif());
                    DEBUG_MINIMAL("Ethernet connected.\r\n\r\n");
                    break;
                }
            }
        }

        while (xTaskNotifyWait(pdTRUE, TASK_NOTIFY_LINK_UP, NULL, portMAX_DELAY ) == false);
    }

    // Initialize application
    initializeApplication((iot_task)params);

    // Monitor connection and disconnection
    uint32_t ulTimeout = 500 * portTICK_PERIOD_MS;
    while (1) {
        if (netif_is_link_up(net_get_netif())) {
            if (!ethernet_is_link_up()) {
                netif_set_link_down(net_get_netif());
                DEBUG_MINIMAL("Ethernet disconnected.\r\n\r\n");
            }
        }
        else {
            if (ethernet_is_link_up()) {
                netif_set_link_up(net_get_netif());
                DEBUG_MINIMAL("Ethernet connected.\r\n\r\n");
            }
        }

        // While data is received, keep on reading
        while (xTaskNotifyWait(0, TASK_NOTIFY_PACKET, NULL, ulTimeout) == pdTRUE) {
            arch_ft900_tick(net_get_netif());
        }
    }
}

/*-----------------------------------------------------------*/

static void myputc(void* p, char c)
{
    uart_write((ft900_uart_regs_t*) p, (uint8_t) c);
}

static inline void setupUART()
{
    /* Set up UART */
    sys_enable(sys_device_uart0);
    gpio_function(48, pad_func_3);
    gpio_function(49, pad_func_3);
    uart_open(UART0, 1, UART_DIVIDER_115200_BAUD, 8, 0, 0);
    /* Enable tfp_printf() functionality... */
    init_printf(UART0, myputc);
}

static inline void setupEthernet()
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

/*-----------------------------------------------------------*/

inline iot_status iot_setup( iot_task task )
{
    sys_reset_all();

    setupUART();

#if 1
    uart_puts(UART0, "\r\n"
        "\x1B[2J" /* ANSI/VT100 - Clear the Screen */
        "\x1B[H" /* ANSI/VT100 - Move Cursor to Home */ );
#else
    uart_puts(UART0,
        "\x1B[2J" /* ANSI/VT100 - Clear the Screen */
        "\x1B[H" /* ANSI/VT100 - Move Cursor to Home */
        "Copyright (C) Bridgetek Pte Ltd \r\n"
        "--------------------------------------------------------------------- \r\n"
        "Welcome to FT900 AWS IoT Demo\r\n"
        "--------------------------------------------------------------------- \r\n");
#endif

    setupEthernet();

    interrupt_enable_globally();

    srand((unsigned int)time(NULL));

    xTaskCreate(vTaskSystem, "SYSTEM", TASK_SYSTEM_STACK_SIZE,
        task, TASK_SYSTEM_PRIORITY, &g_hSystemTask);

    vTaskStartScheduler();
    for(;;);

    return pdPASS;
}


