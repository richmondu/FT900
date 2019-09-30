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

#include <net.h>
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
#include "aws_bufferpool.h"
#include "aws_mqtt_agent.h"



/* Macro Definitions ---------------------------------------------------------*/

#define TASK_SYSTEM_STACK_SIZE      (512+256)
#define TASK_SYSTEM_PRIORITY        2

/* Private variables ---------------------------------------------------------*/

TaskHandle_t g_hSystemTask = NULL;




/*-----------------------------------------------------------*/

static void vTaskSystem(void* params)
{
    BaseType_t xResult = MQTT_AGENT_Init();
    if ( xResult == pdPASS ) {
        xResult = BUFFERPOOL_Init();
    }
    if ( xResult == pdPASS ) {
        DEBUG_MINIMAL( "Initialize app...\r\n" );
        ((iot_task)params)(NULL);
    }

    DEBUG_MINIMAL( "Initialize app...x\r\n" );
    for (;;);
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
    uart_open(UART0, 1, UART_DIVIDER_19200_BAUD, 8, 0, 0);
    /* Enable tfp_printf() functionality... */
    init_printf(UART0, myputc);
}

static inline void setupEthernet()
{
	net_setup();
}

/*-----------------------------------------------------------*/

inline iot_status iot_setup( iot_task task )
{
    sys_reset_all();
    interrupt_enable_globally();
    setupUART();

    uart_puts(UART0,
        "\x1B[2J" /* ANSI/VT100 - Clear the Screen */
        "\x1B[H" /* ANSI/VT100 - Move Cursor to Home */
        "Copyright (C) Bridgetek Pte Ltd \r\n"
        "--------------------------------------------------------------------- \r\n"
        "Welcome to FT900 AWS IoT Demo\r\n"
        "--------------------------------------------------------------------- \r\n");

    setupEthernet();

    xTaskCreate(vTaskSystem, "SYSTEM", TASK_SYSTEM_STACK_SIZE,
        task, TASK_SYSTEM_PRIORITY, &g_hSystemTask);

    vTaskStartScheduler();
    return pdPASS;
}


