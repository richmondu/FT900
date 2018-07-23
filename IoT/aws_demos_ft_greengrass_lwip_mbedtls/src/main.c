/*
 * Amazon FreeRTOS V1.0.0
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

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ft900.h"
#include "tinyprintf.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Credentials includes */
#include "aws_clientcredential.h"

/* LWIP includes. */
#include "lwip/ip_addr.h"
#include "net.h"

/* MQTT includes. */
#include "aws_bufferpool.h"
#include "aws_mqtt_agent.h"
#include "aws_demo_config.h"



/* Macro Definitions ---------------------------------------------------------*/

#define TASK_NOTIFY_NETIF_UP    	(0x01)
#define TASK_NOTIFY_LINK_UP     	(0x02)
#define TASK_NOTIFY_LINK_DOWN   	(0x04)
#define TASK_NOTIFY_PACKET   		(0x08)

// Do not modify unless you know what you are doing
// These are currently the most optimal stack size for this application
#define TASK_SYSTEM_STACK_SIZE      176//(144)
#define TASK_SYSTEM_PRIORITY        (configMAX_PRIORITIES-1)
#define TASK_MQTTAPP_STACK_SIZE     (192)
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

static inline void initializeApplication()
{
	extern void vMQTTAppTask( void * pvParameters );
    BaseType_t xResult = pdPASS;


    xResult = MQTT_AGENT_Init();
    if ( xResult == pdPASS ) {
        xResult = BUFFERPOOL_Init();
    }
    if ( xResult == pdPASS ) {
		xTaskCreate(vMQTTAppTask, "MQTTAPP", TASK_MQTTAPP_STACK_SIZE,
			NULL, TASK_MQTTAPP_PRIORITY, &g_hMQTTAppTask );
    }
}

static void vTaskSystem(void* params)
{

	// Initialize Ethernet
	{
		ip_addr_t ipaddr, gateway, mask;
		ipaddr.addr = FT9XX_IP_ADDRESS;
		gateway.addr = FT9XX_IP_GATEWAY;
		mask.addr = FT9XX_IP_SUBNET;


		net_init(ipaddr, gateway, mask, 1, NULL, connectionCB);

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
	initializeApplication();

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
#if 0
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

/**
 * @brief Application runtime entry point.
 */
int main( void )
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
		"Welcome to AWS IoT Demo\r\n"
		"--------------------------------------------------------------------- \r\n");
#endif

    setupEthernet();

	interrupt_enable_globally();

    srand((unsigned int)time(NULL));

    xTaskCreate(vTaskSystem, "SYSTEM", TASK_SYSTEM_STACK_SIZE,
    	NULL, TASK_SYSTEM_PRIORITY, &g_hSystemTask);

    vTaskStartScheduler();
    for(;;);

    return 0;
}

