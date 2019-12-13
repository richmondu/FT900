#include <ft900.h>
#include "tinyprintf.h"

/* FreeRTOS Headers. */
#include "FreeRTOS.h"
#include "task.h"

/* netif Abstraction Header. */
#include "net.h"

/* IOT Headers. */
#include <iot_config.h>
#include "iot/iot.h"
#include "iot/iot_utils.h"

/* IoT Modem */
#include "iot_modem.h"
#include "json.h"


#include <string.h>
#include <stdlib.h>





///////////////////////////////////////////////////////////////////////////////////
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {CRITICAL_SECTION_BEGIN;tfp_printf(__VA_ARGS__);CRITICAL_SECTION_END;} while (0)
#else
#define DEBUG_PRINTF(...)
#endif
///////////////////////////////////////////////////////////////////////////////////



#if ENABLE_UART_ATCOMMANDS

extern TaskHandle_t g_iot_app_handle; // used by iot_modem_uart_isr
extern iot_handle g_handle;

typedef struct _UART_ATCOMMANDS {
	char* m_pcCmd;
	void (*m_pcFxn)(uint8_t ucCmdIdx, char *pcCmd, int lCmdLen);
	char* m_pcHelp;
//	char* m_pcEx;
} UART_ATCOMMANDS;

static void uart_cmdhdl_mobile(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);
static void uart_cmdhdl_email(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);
static void uart_cmdhdl_notification(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);
static void uart_cmdhdl_mOdem(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);
static void uart_cmdhdl_storage(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);
static void uart_cmdhdl_default(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);
static void uart_cmdhdl_continue(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);
static void uart_cmdhdl_echo(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);
static void uart_cmdhdl_help(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);
static void uart_cmdhdl_info(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);
static void uart_cmdhdl_more(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);
static void uart_cmdhdl_pause(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);
static void uart_cmdhdl_reset(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);
static void uart_cmdhdl_update(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);
static void uart_cmdhdl_status(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen);

UART_ATCOMMANDS g_acUartCommands[UART_ATCOMMANDS_NUM] = {
	{ UART_ATCOMMAND_MOBILE,   uart_cmdhdl_mobile,       UART_ATCOMMAND_DESC_MOBILE  },
	{ UART_ATCOMMAND_EMAIL,    uart_cmdhdl_email,        UART_ATCOMMAND_DESC_EMAIL   },
	{ UART_ATCOMMAND_NOTIFY,   uart_cmdhdl_notification, UART_ATCOMMAND_DESC_NOTIFY  },
	{ UART_ATCOMMAND_MODEM,    uart_cmdhdl_mOdem,        UART_ATCOMMAND_DESC_MODEM   },
	{ UART_ATCOMMAND_STORAGE,  uart_cmdhdl_storage,      UART_ATCOMMAND_DESC_STORAGE },
	{ UART_ATCOMMAND_DEFAULT,  uart_cmdhdl_default,      UART_ATCOMMAND_DESC_DEFAULT },

	{ UART_ATCOMMAND_CONTINUE, uart_cmdhdl_continue,     UART_ATCOMMAND_DESC_CONTINUE },
	{ UART_ATCOMMAND_ECHO,     uart_cmdhdl_echo,         UART_ATCOMMAND_DESC_ECHO     },
	{ UART_ATCOMMAND_HELP,     uart_cmdhdl_help,         UART_ATCOMMAND_DESC_HELP     },
	{ UART_ATCOMMAND_INFO,     uart_cmdhdl_info,         UART_ATCOMMAND_DESC_INFO     }, // = {software version, present configuration, present status, list of I2C devices and addresses, IP address, connection status to service, etc}" },
	{ UART_ATCOMMAND_MORE,     uart_cmdhdl_more,         UART_ATCOMMAND_DESC_MORE     },
	{ UART_ATCOMMAND_PAUSE,    uart_cmdhdl_pause,        UART_ATCOMMAND_DESC_PAUSE    },
	{ UART_ATCOMMAND_RESET,    uart_cmdhdl_reset,        UART_ATCOMMAND_DESC_RESET    },
	{ UART_ATCOMMAND_UPDATE,   uart_cmdhdl_update,       UART_ATCOMMAND_DESC_UPDATE   },

	{ UART_ATCOMMAND_STATUS,   uart_cmdhdl_status,       UART_ATCOMMAND_DESC_STATUS   },// OK if all is good, ERROR if device is in error state" },
};

static char g_acUartCommandBuffer[64] = {0};
static uint8_t g_ucUartCommandBufferOffset = 0;
static uint8_t g_ucUartCommandBufferAvailable = 1;

#endif // ENABLE_UART_ATCOMMANDS


static uint16_t g_uwBaudrates[UART_PROPERTIES_BAUDRATE_COUNT] = {
	UART_DIVIDER_110_BAUD,
	UART_DIVIDER_150_BAUD,
	UART_DIVIDER_300_BAUD,
	UART_DIVIDER_1200_BAUD,
	UART_DIVIDER_2400_BAUD,
	UART_DIVIDER_4800_BAUD,
	UART_DIVIDER_9600_BAUD,
	UART_DIVIDER_19200_BAUD,
	UART_DIVIDER_31250_BAUD,
	UART_DIVIDER_38400_BAUD,
	UART_DIVIDER_57600_BAUD,
	UART_DIVIDER_115200_BAUD,
	UART_DIVIDER_230400_BAUD,
	UART_DIVIDER_460800_BAUD,
	UART_DIVIDER_921600_BAUD,
	UART_DIVIDER_1000000_BAUD
};



#if ENABLE_UART_ATCOMMANDS

static inline void uart_publish(char* pcMenos, char* pcRecipient, char* pcMessage)
{
    char topic[64] = {0};
    char payload[160] = {0};
    tfp_snprintf( topic, sizeof(topic), "%s%s/trigger_notification/uart/%s", PREPEND_REPLY_TOPIC, iot_utils_getdeviceid(), pcMenos);
    if (pcRecipient && pcMessage) {
    	tfp_snprintf( payload, sizeof(payload), "{\"recipient\":\"%s\",\"message\":\"%s\"}", pcRecipient, pcMessage);
    }
    else if (!pcRecipient && !pcMessage) {
    	tfp_snprintf( payload, sizeof(payload), "{}");
    }
    iot_publish( g_handle, topic, payload, strlen(payload), 1 );
    DEBUG_PRINTF("PUB %s %s\r\n\r\n", topic, payload);
}

static inline int uart_parse(char* pcCmd, int lCmdLen, char* recipient, int lRecipientSize, char* message, int lMessageSize)
{
    char* pcRecipient = pcCmd;
    if (*pcRecipient != '+') {
        DEBUG_PRINTF("wrong syntax 1\r\n");
        return 0;
    }
    pcRecipient++;
    if (*pcRecipient == '\0') {
        DEBUG_PRINTF("wrong syntax 2\r\n");
        return 0;
    }

    char* pcMessage = strchr(pcRecipient, '+');
    if (!pcMessage) {
        DEBUG_PRINTF("wrong syntax 3\r\n");
        return 0;
    }
    pcMessage++;
    if (*pcMessage == '\0') {
        DEBUG_PRINTF("wrong syntax 4\r\n");
        return 0;
    }

    if (pcMessage-pcRecipient-1 >= lRecipientSize) {
        DEBUG_PRINTF("recipient length is too big\r\n");
        return 0;
    }
    if (lCmdLen-(pcMessage-pcCmd) >= lMessageSize) {
        DEBUG_PRINTF("message length is too big\r\n");
        return 0;
    }

    strncpy(recipient, pcRecipient, pcMessage-pcRecipient-1);
    //DEBUG_PRINTF("%s\r\n", recipient);
    strncpy(message, pcMessage, lCmdLen-(pcMessage-pcCmd));
    //DEBUG_PRINTF("%s\r\n", message);

    return 1;
}

static void uart_cmdhdl_mobile(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
	int lCmdLenLocal = strlen(g_acUartCommands[ucCmdIdx].m_pcCmd);
    if (lCmdLen == lCmdLenLocal) {
    	uart_publish("mobile", NULL, NULL);
    	return;
    }

    char recipient[32] = {0};
    char message[64] = {0};
    if (uart_parse(pcCmd+lCmdLenLocal, lCmdLen, recipient, sizeof(recipient), message, sizeof(message))) {
    	uart_publish("mobile", recipient, message);
    }
}

static void uart_cmdhdl_email(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
	int lCmdLenLocal = strlen(g_acUartCommands[ucCmdIdx].m_pcCmd);
    if (lCmdLen == lCmdLenLocal) {
    	uart_publish("email", NULL, NULL);
    	return;
    }

    char recipient[32] = {0};
    char message[64] = {0};
    if (uart_parse(pcCmd+lCmdLenLocal, lCmdLen, recipient, sizeof(recipient), message, sizeof(message))) {
    	uart_publish("email", recipient, message);
    }
}

static void uart_cmdhdl_notification(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
    if (lCmdLen == strlen(g_acUartCommands[ucCmdIdx].m_pcCmd)) {
    	uart_publish("notification", NULL, NULL);
    	return;
    }
}

static void uart_cmdhdl_mOdem(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
	int lCmdLenLocal = strlen(g_acUartCommands[ucCmdIdx].m_pcCmd);
    if (lCmdLen == lCmdLenLocal) {
    	uart_publish("mOdem", NULL, NULL);
    	return;
    }

    char recipient[32] = {0};
    char message[64] = {0};
    if (uart_parse(pcCmd+lCmdLenLocal, lCmdLen, recipient, sizeof(recipient), message, sizeof(message))) {
    	uart_publish("mOdem", recipient, message);
    }
}

static void uart_cmdhdl_storage(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
    if (lCmdLen == strlen(g_acUartCommands[ucCmdIdx].m_pcCmd)) {
    	uart_publish("storage", NULL, NULL);
    	return;
    }
}

static void uart_cmdhdl_default(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
    if (lCmdLen == strlen(g_acUartCommands[ucCmdIdx].m_pcCmd)) {
    	uart_publish("default", NULL, NULL);
    	return;
    }
}

static void uart_cmdhdl_continue(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
    DEBUG_PRINTF("%d\r\n", ucCmdIdx);
}

static void uart_cmdhdl_echo(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
    DEBUG_PRINTF("%d\r\n", ucCmdIdx);
}

static void uart_cmdhdl_help(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
	DEBUG_PRINTF("\r\nUART Commands:\r\n");
    for (int i=0; i<UART_ATCOMMANDS_NUM; i++) {
        DEBUG_PRINTF("%s\t%s\r\n", g_acUartCommands[i].m_pcCmd, g_acUartCommands[i].m_pcHelp);
    }
	DEBUG_PRINTF("\r\n");
}

static void uart_cmdhdl_info(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
    DEBUG_PRINTF("%d\r\n", ucCmdIdx);
}

static void uart_cmdhdl_more(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
    DEBUG_PRINTF("%d\r\n", ucCmdIdx);
}

static void uart_cmdhdl_pause(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
    DEBUG_PRINTF("%d\r\n", ucCmdIdx);
}

static void uart_cmdhdl_reset(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
    DEBUG_PRINTF("%d\r\n", ucCmdIdx);
}

static void uart_cmdhdl_update(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
    DEBUG_PRINTF("%d\r\n", ucCmdIdx);
}

static void uart_cmdhdl_status(uint8_t ucCmdIdx, char* pcCmd, int lCmdLen)
{
    DEBUG_PRINTF("%d\r\n", ucCmdIdx);
}


void iot_modem_uart_command_help()
{
	uart_cmdhdl_help(0, NULL, 0);
}

void iot_modem_uart_command_process()
{
    g_ucUartCommandBufferAvailable = 0;
    DEBUG_PRINTF("command: %s [%d]\r\n", g_acUartCommandBuffer, g_ucUartCommandBufferOffset);

    for ( int i=0; i<UART_ATCOMMANDS_NUM; i++ ) {
    	if (strncmp(g_acUartCommandBuffer, g_acUartCommands[i].m_pcCmd, strlen(g_acUartCommands[i].m_pcCmd))==0) {
            g_acUartCommands[i].m_pcFxn(i, g_acUartCommandBuffer, (int)g_ucUartCommandBufferOffset);
			break;
    	}
	}

    memset(g_acUartCommandBuffer, 0, sizeof(g_acUartCommandBuffer));
    g_ucUartCommandBufferOffset = 0;
    g_ucUartCommandBufferAvailable = 1;
}

void iot_modem_uart_isr()
{
    static uint8_t c;

    if (uart_is_interrupted(UART0, uart_interrupt_rx))
    {
        if (g_ucUartCommandBufferAvailable == 0) {
            return;
        }

        // read input from UART and store to array
        uart_read(UART0, &c);
        g_acUartCommandBuffer[g_ucUartCommandBufferOffset++] = c;
        uart_write(UART0, c);

        // check if command exceeds buffer
        if (g_ucUartCommandBufferOffset == sizeof(g_acUartCommandBuffer)) {
            DEBUG_PRINTF("\r\nCommand should be less than %d bytes\r\n", sizeof(g_acUartCommandBuffer));
            memset(g_acUartCommandBuffer, 0, sizeof(g_acUartCommandBuffer));
            g_ucUartCommandBufferOffset = 0;
            return;
        }

        // process the command when enter is pressed
        if (c == 0x0D) {
        	// process enter/carriage return
            g_acUartCommandBuffer[g_ucUartCommandBufferOffset-1] = '\0'; // Remove the enter key
            g_ucUartCommandBufferOffset--;
            xTaskNotifyFromISR(g_iot_app_handle, 0, eNoAction, NULL);
        }
        else if (c == 0x08) {
        	// process backspace
            g_acUartCommandBuffer[g_ucUartCommandBufferOffset-1] = '\0'; // Remove the backspace
            g_acUartCommandBuffer[g_ucUartCommandBufferOffset-2] = '\0'; // Remove the other character
            g_ucUartCommandBufferOffset -= 2;
        }
    }
}

#endif // ENABLE_UART_ATCOMMANDS


void iot_modem_uart_enable(UART_PROPERTIES* properties, int enable, int disable)
{
	if (disable) {
        uart_close(UART0);
        uart_soft_reset(UART0);
	}

	if (enable) {
		uart_open(
			UART0, 1,
			g_uwBaudrates[properties->m_ucBaudrate],
			properties->m_ucDatabits,
			properties->m_ucParity,
			properties->m_ucStopbits
			);
	}
}


