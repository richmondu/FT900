/**
  @file at.c
  @brief
  ESP32 Wifi module.

 */
/*
 * ============================================================================
 * History
 * =======
 * 2019-04-25 : Created v1
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
#if (COMMUNICATION_IO==2) // WiFi
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "ft900.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "ft900_uart_simple.h"
#include "uartrb.h"
#include "at.h"



//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



#define CRLF "\r\n"
#define AT "AT"
#define OK "OK"
#define ERROR "ERROR"
#define QUERY "?"
#define SET "="
#define ERROR_CODE "ERR CODE"
#define STATUS_CODE "STATUS"
#define READY_CODE "ready"

#define MARKER_MAX_LENGTH 20
#define MARKER_WIFI_CONNECTED "WIFI CONNECTED\r\n"
#define MARKER_WIFI_GOT_IP "WIFI GOT IP\r\n"
#define MARKER_WIFI_DISCONNECTED "WIFI DISCONNECTED\r\n"
#define MARKER_SERVER_CONNECT ",CONNECT\r\n"
#define MARKER_SERVER_CLOSE ",CLOSED\r\n"
#define MARKER_IPD "\r\n+IPD,"

//#define RINGBUFFER_SIZE 64
#define AT_MAX_COMMAND_LEN 256

/* Define to echo commands sent to the AT firmware on the debug port.
 * The AT firmware will echo received commands anyway.
 */
#undef MONITOR_ECHO_TX

/* Define to echo the response from the AT firmware on the debug port.
 */
#undef MONITOR_ECHO_RX
/* Define to enable UART monitor
 */
#undef UART_MONITOR
struct ipd_store {
    enum at_ipd_status valid;
    uint16_t length;
    uint8_t *buffer;
    int8_t link_id;
    char remote_ip[AT_MAX_IP];
    uint16_t remote_port;
    struct ipd_store *next;
};

static struct ipd_store *ipd_head;
static struct ipd_store *ipd_write;

static ft900_uart_regs_t *uart_at;
static ft900_uart_regs_t *uart_monitor;

/* Timeout counter */
static TimerHandle_t at_timer;
//static int at_tx_timeout_cmd = pdMS_TO_TICKS(20000);
//static int at_rx_timeout_cmd = pdMS_TO_TICKS(20000);
static int at_tx_timeout_cmd = pdMS_TO_TICKS(30000);
static int at_rx_timeout_cmd = pdMS_TO_TICKS(30000);

static TimerHandle_t cmd_timer;
//static int cmd_timeout = pdMS_TO_TICKS(10000);
//static int cmd_timeout_inet = pdMS_TO_TICKS(20000);
//static int cmd_timeout_ipd = pdMS_TO_TICKS(500);
//static int cmd_timeout_ap = pdMS_TO_TICKS(20000);
#if 1 // TODO: RMU fix MQTT Connect timeout due to slow ESP32 TLS Connect
static int cmd_timeout = pdMS_TO_TICKS(30000); // 100 is too slow for AWS
#endif
static int cmd_timeout_inet = pdMS_TO_TICKS(30000);
static int cmd_timeout_ipd = pdMS_TO_TICKS(30000);
static int cmd_timeout_ap = pdMS_TO_TICKS(30000);

/* AT device will normally echo commands */
static enum at_echo at_echo = at_echo_off;
static enum at_cwlap_sort at_cwlapopt_sort = at_cwlap_sort_ordered;
static enum at_cwlap_mask at_cwlapopt_mask = at_cwlap_mask_all;
static enum at_enable at_cipmux = at_disable;
static enum at_txmode at_cipmode = at_txmode_normal;
static enum at_enable at_cipdinfo = at_disable;

static int8_t at_state_ipd_pending = 0;
static int8_t at_state_wifi_connected = 0;
static int8_t at_state_wifi_station_has_ip = 0;
static enum at_connection at_state_server_connect[AT_LINK_ID_COUNT] = {0};

static int8_t at_txcommand(const char *command);
static int8_t at_rxresponse(char *response, uint16_t *length, int cmdtimeout);
static int8_t at_txresponse(char *response, uint16_t length);

static void peek_async_message(void);
static uint16_t check_async_message(char *message, uint16_t length);
static int8_t async_ipd_receive(void);

static char *rsp_next_line(const char *line);
static uint16_t rsp_get_line_length(const char *line);
static uint16_t rsp_get_line_length_max(const char *line, const uint16_t max);
static char *rsp_skip_cmd_echo(const char *line);
static char *rsp_check_response(const char *line, const char *expected);
static int8_t cmd_execute_with_timeout(char *cmd, int cmdtimeout);
static int8_t cmd_execute(char *cmd);
static int8_t cmd_query_with_timeout(char *cmd, char *params, uint16_t param_max_len, int cmdtimeout);
static int8_t cmd_query(char *cmd, char *params, uint16_t param_max_len);
static int8_t cmd_set_with_timeout(char *cmd, char *params, int cmdtimeout);
static int8_t cmd_set(char *cmd, char *params);
static char *helper_strcpy_param(char *dest, const char *src);
static char *helper_strcpy_param_escapify(char *dest, const char *src, uint16_t max);
static char *helper_strcpy_param_unescapify(char *dest, const char *src, uint16_t max);

static int8_t helper_query_uart(char *cmd, struct at_cwuart_s *uart);

static int8_t at_txcommand(const char *command)
{
    uint16_t espCount;
    char *espPtr;
    uint16_t dbgCount;
    char *dbgPtr;
    uint16_t count;
    int8_t rsp = AT_ERROR_RESPONSE;

    // Transmit AT command and optionally echo it to the debug line
    espPtr = (char *)command;
    espCount = strnlen(command, AT_MAX_COMMAND_LEN);

#ifdef MONITOR_ECHO_TX
    dbgPtr = espPtr;
    dbgCount = espCount;
#else // DEBUG_ECHO
    (void)dbgPtr;
    dbgCount = 0;
#endif // DEBUG_ECHO

    xTimerChangePeriod(at_timer, at_tx_timeout_cmd, 0);
    DEBUG_PRINTF("\t[AT]: %s", command);
    do
    {
        // Send command to AT
        if (espCount > 0)
        {
            count = uartrb_write(uart_at, (uint8_t *)espPtr, espCount);
            if (count > 0)
            {
                espCount -= count;
                espPtr += count;
            }
        }

#ifdef MONITOR_ECHO_TX
        // Copy command to debug port
        if (dbgCount > 0)
        {
            count = uartrb_write(uart_monitor, (uint8_t *)dbgPtr, dbgCount);
            if (count > 0)
            {
                dbgCount -= count;
                dbgPtr += count;
            }
        }
#endif // DEBUG_ECHO

        if (xTimerIsTimerActive(at_timer) == pdFALSE)
        {
            configPRINTF(("AT timer not active \r\n"));
            rsp = AT_ERROR_TIMEOUT;
            break;
        }

        xTimerReset(at_timer, 0);

    } while (espCount || dbgCount);

    if (espCount == 0)
    {
        rsp = AT_OK;
    }

    xTimerStop(at_timer, 0);

    return rsp;
}

static int8_t at_rxresponse(char *response, uint16_t *length, int cmdtimeout)
{
    uint16_t espCount = 0;
    char *espPtr;
    uint16_t rspLength = *length;
    uint16_t count;
    char *rspparams;
    int8_t complete = 0;
    int8_t rsp = AT_ERROR_RESPONSE;

    // Receive response from AT
    espPtr = response;
    espCount = 0;

    // Read an parse any async messages which may be pending.
    peek_async_message();

    // Read in echoed command and ignore.
    if (at_echo == at_echo_on)
    {
        xTimerChangePeriod(at_timer, at_rx_timeout_cmd, 0);

        count = uartrb_readln(uart_at, (uint8_t *)espPtr, rspLength);

        if (xTimerIsTimerActive(at_timer) == pdFALSE)
        {
            rsp = AT_ERROR_TIMEOUT;
        }

        xTimerStop(at_timer, 0);

        at_txresponse(espPtr, count);
    }

    if (rsp == AT_ERROR_TIMEOUT)
    {
        return rsp;
    }

    xTimerChangePeriod(cmd_timer, cmdtimeout, 0);
    //xTimerChangePeriod(at_timer, at_rx_timeout_cmd, 0);

    do
    {
        count = uartrb_readln(uart_at, (uint8_t *)espPtr, rspLength - espCount);
        if (count > 0)
        {
            rspparams = rsp_check_response(espPtr, "AT!");
            if (rspparams)
            {
                rsp = strtol(rspparams, &rspparams, 16);
            }
            if (strncmp(espPtr, OK, count) == 0)
            {
                complete = 1;
                rsp = AT_OK;
            }
            else if (strncmp(espPtr, ERROR, count) == 0)
            {
                complete = -1;
            }

            if (complete == 0)
            {
                espPtr += count;
                espCount += (count + AT_STRING_LENGTH(CRLF));
                *espPtr++ = '\r';
                *espPtr++ = '\n';
            }

            xTimerReset(at_timer, 0);
        }

        //if (xTimerIsTimerActive(at_timer) == pdFALSE)
        //{
            //rsp = AT_ERROR_TIMEOUT;
            //break;
        //}

        if (xTimerIsTimerActive(cmd_timer) == pdFALSE)
        {
            rsp = AT_ERROR_TIMEOUT;
            break;
        }
    } while (!complete);

    //xTimerStop(at_timer, 0);
    xTimerStop(cmd_timer, 0);

    *length = espCount;
    return rsp;
}

static int8_t at_txresponse(char *response, uint16_t length)
{
    int8_t rsp = AT_OK;

#if MONITOR_ECHO_RX
    uint16_t dbgCount;
    char *dbgPtr;
    uint16_t count;

    // Copy response to debug port
    dbgPtr = response;
    dbgCount = length;

    xTimerChangePeriod(at_timer, at_tx_timeout_cmd, 0);

    do
    {
        if (dbgCount > 0)
        {
            count = uartrb_write(uart_monitor, (uint8_t *)dbgPtr, dbgCount);
            if (count > 0)
            {
                dbgCount -= count;
                dbgPtr += count;
            }
        }

        if (xTimerIsTimerActive(at_timer) == pdFALSE)
        {
            rsp = AT_ERROR_TIMEOUT;
        }
    } while (dbgCount);

    uartrb_putc(uart_monitor, '\r');
    uartrb_putc(uart_monitor, '\n');

    xTimerStop(at_timer, 0);

#endif // MONITOR_ECHO_RX
    return rsp;
}

static void at_timer_callback(TimerHandle_t xTimerHandle)
{
//    tfp_printf("%s \r\n", __FUNCTION__);
    // Signal a timeout to the ring buffer so any blocking call
    // to the AT is returned.
    uartrb_timeout(uart_at);
}

static void cmd_timer_callback(TimerHandle_t xTimerHandle)
{
//    tfp_printf("%s \r\n", __FUNCTION__);
    // No action
}

int8_t at_init(ft900_uart_regs_t *at, ft900_uart_regs_t *monitor)
{
    enum at_cipstatus status = at_cipstatus_not_connected;
    int8_t count;
    struct at_cipstatus_s cipstatus[AT_LINK_ID_COUNT];
    int8_t err;

    uart_at = at;
    uart_monitor = monitor;
#ifdef UART_MONITOR
    // Enable 16 byte FIFO buffers
    uart_mode(uart_monitor, uart_mode_16550);
    // Open UART 1 using the coding required.
    uart_open(uart_monitor,                    /* Device */
                1,                        /* Prescaler = 1 */
                UART_DIVIDER_115200_BAUD,  /* Divider = 1302 */
                uart_data_bits_8,         /* No. buffer Bits */
                uart_parity_none,         /* Parity */
                uart_stop_bits_1);        /* No. Stop Bits */
    uartrb_setup(uart_monitor, uartrb_flow_rts_cts);
#endif


    //uart_mode(uart_at, uart_mode_16550);
    uart_mode(uart_at, 1); //uart_mode_16550 = 1

    // Open UART 1 using the coding required.
    uart_open(uart_at,                    /* Device */
            1,                        /* Prescaler = 1 */
            UART_DIVIDER_115200_BAUD,  /* Divider = 1302 */
            8,         /* No. buffer Bits */
            0,         /* Parity */
            0);        /* No. Stop Bits */


    // UART 0 is already set-up. This enabled interrupts and the ring buffers.
    uartrb_setup(uart_at, uartrb_flow_none);

    at_timer = xTimerCreate("AT_COMMS", at_tx_timeout_cmd, pdFALSE, 0, at_timer_callback);
    if (at_timer == NULL)
    {
        return AT_ERROR_TIMER;
    }

    cmd_timer = xTimerCreate("AT_CMD", cmd_timeout_inet, pdFALSE, 0, cmd_timer_callback);
    if (cmd_timer == NULL)
    {
        return AT_ERROR_TIMER;
    }

    uartrb_flush_read(uart_at);

    if (at_query_cwjap(NULL) == AT_OK)
    {
        at_state_wifi_connected = 1;
    }

    if (at_query_cipsta(NULL, NULL, NULL) == AT_OK)
    {
        at_state_wifi_station_has_ip = 1;
    }

    at_query_ate(&at_echo);
    at_query_cipmux(&at_cipmux);
    at_query_cipmode(&at_cipmode);
    at_query_cipdinfo(&at_cipdinfo);

    count = AT_LINK_ID_COUNT;
    err = at_query_cipstatus(&status, &count, cipstatus);
    if ((err == AT_OK) && (count > 0))
    {
        uint8_t link_id;

        // Set connection active for each link_id received.
        while (--count > 0)
        {
            link_id = cipstatus[count].link_id;
            at_state_server_connect[link_id] = at_connected;
        }
    }

    ipd_head = NULL;
    ipd_write = NULL;

    return AT_OK;
}

int8_t at_timeout_tx_comms(int timeout)
{
    at_tx_timeout_cmd = timeout;
    return AT_OK;
}

int8_t at_timeout_rx_comms(int timeout)
{
    at_rx_timeout_cmd = timeout;
    return AT_OK;
}

int8_t at_timeout_comms(int timeout)
{
    at_rx_timeout_cmd = timeout;
    at_tx_timeout_cmd = timeout;
    return AT_OK;
}

int8_t at_timeout_cmd(int timeout)
{
    cmd_timeout = timeout;
    return AT_OK;
}

int8_t at_timeout_inet(int timeout)
{
    cmd_timeout_inet = timeout;
    return AT_OK;
}

int8_t at_timeout_ipd(int timeout)
{
    cmd_timeout_ipd = timeout;
    return AT_OK;
}

int8_t at_timeout_ap(int timeout)
{
    cmd_timeout_ap = timeout;
    return AT_OK;
}

int8_t at_command(const char *command, uint16_t *length, char *response, int cmdtimeout)
{
    int8_t complete;

    peek_async_message();

    // Transmit command to AT.
    complete = at_txcommand(command);

    // If transmission was successful.
    if (complete == 0)
    {
        // Receive the response from the AT.
        complete = at_rxresponse(response, length, cmdtimeout);
    }
    if (complete == 0)
    {
        complete = at_txresponse(response, *length);
    }

    return complete;
}

int8_t at_query_ate(enum at_echo *echo)
{
    int8_t complete;
    char rsp[16];
    uint16_t count;

    // Transmit command to AT.
    complete = at_txcommand(AT CRLF);

    // If transmission was successful.
    if (complete == 0)
    {
        if (xTimerChangePeriod(at_timer, at_rx_timeout_cmd, 0) != pdPASS)
        {
            return AT_ERROR_TIMER;
        }

        count = uartrb_readln(uart_at, (uint8_t *)rsp, 16);
        if (strncmp(rsp, AT CRLF, count) == 0)
        {
            at_echo = at_echo_on;
            *echo = at_echo_on;
        }
        else
        {
            at_echo = at_echo_off;
            *echo = at_echo_off;
        }

        while (1)
        {
            if (xTimerIsTimerActive(at_timer) == pdFALSE)
            {
                return AT_ERROR_TIMEOUT;
            }
            count = uartrb_readln(uart_at, (uint8_t *)rsp, 16);
            if (strncmp(rsp, OK CRLF, count) == 0)
            {
                break;
            }
        }

        xTimerStop(at_timer, 0);
    }

    return complete;
}

int8_t at_passthrough(void)
{
#ifdef UART_MONITOR
    int count;
    uint8_t txData[RINGBUFFER_SIZE];
    uint16_t txCount = 0;
    uint16_t txPtr = 0;
    uint8_t rxData[RINGBUFFER_SIZE];
    uint16_t rxCount = 0;
    uint16_t rxPtr = 0;

    while (1)
    {
        if (txCount == 0)
        {
            txCount = uartrb_read(uart_monitor, txData, RINGBUFFER_SIZE);
            txPtr = 0;
        }

        if (txCount > 0)
        {
            if (txData[0] == 0x04) // EOT
            {
                txCount = 0;
                txPtr = 0;

                // Leave pass through mode.
                return 1;
            }

            count = uartrb_write(uart_at, txData + txPtr, txCount);
            if (count > 0)
            {
                txCount -= count;
                txPtr += count;
            }
        }

        if (rxCount == 0)
        {
            rxCount = uartrb_read(uart_at, rxData, RINGBUFFER_SIZE);
            rxPtr = 0;
        }

        if (rxCount > 0)
        {
            count = uartrb_write(uart_monitor, rxData + rxPtr, rxCount);
            if (count > 0)
            {
                rxCount -= count;
                rxPtr += count;
            }
        }
    }
#endif
    return AT_OK;
}

__attribute__((unused)) static char *helper_strcpy_param(char *dest, const char *src)
{
    // Simple copy ensuring a NULL terminator AND the
    // return value points to the NULL.
    *dest = 0;
    while(*src)
    {
        *dest++ = *src++;
    }
    *dest = 0;
    return dest;
}

static char *helper_strcpy_param_escapify(char *dest, const char *src, uint16_t max)
{
    // Copy and escape ensuring a NULL terminator AND the
    // return value points to the NULL.
    *dest++ = '\"';
    *dest = 0;
    while ((*src) && (max))
    {
        switch(*src)
        {
        case '\\' : *dest++ = '\\'; *dest = '\\'; break;
        case '\'' : *dest++ = '\\'; *dest = '\''; break;
        case '\"' : *dest++ = '\\'; *dest = '\"'; break;
        case '\a' : *dest++ = '\\'; *dest = 'a'; break;
        case '\b' : *dest++ = '\\'; *dest = 'b'; break;
        case '\f' : *dest++ = '\\'; *dest = 'f'; break;
        case '\n' : *dest++ = '\\'; *dest = 'n'; break;
        case '\r' : *dest++ = '\\'; *dest = 'r'; break;
        case '\t' : *dest++ = '\\'; *dest = 't'; break;
        case '\v' : *dest++ = '\\'; *dest = 'v'; break;
        case '\?' : *dest++ = '\\'; *dest = '\?'; break;
        default:  *dest = *src;
        }
        src++;
        dest++;
        max--;
    }
    *dest++ = '\"';
    *dest = 0;
    return dest;
}

static char *helper_strcpy_param_unescapify(char *dest, const char *src, uint16_t max)
{
    int8_t escape = 0;
    int8_t quotes = 0;

    // Copy and unescape ensuring a NULL terminator AND the
    // return value points to the NULL.
    while ((*src) && (max))
    {
        if ((escape == 0) && (*src == '\\'))
        {
            escape = 1;
        }
        else if ((escape == 0) && ((*src == ',') || (*src == '\r')))
        {
            // Do not copy parameter separators
            break;
        }
        else if ((escape == 0) && (quotes == 0) && (*src == '\"'))
        {
            quotes = 1;
        }
        else if ((escape == 0) && (quotes == 1) && (*src == '\"'))
        {
            quotes = 0;
        }
        else if ((escape == 1) && (quotes == 1))
        {
            switch (*src)
            {
            case '\\' : *dest = '\\'; break;
            case '\'' : *dest = '\''; break;
            case '\"' : *dest = '\"'; break;
            case 'a' : *dest = '\a'; break;
            case 'b' : *dest = '\b'; break;
            case 'f' : *dest = '\f'; break;
            case 'n' : *dest = '\n'; break;
            case 'r' : *dest = '\r'; break;
            case 't' : *dest = '\t'; break;
            case 'v' : *dest = '\v'; break;
            case '?' : *dest = '?'; break;
            default:  *dest = *src;
            }
            escape = 0;
            dest++;
        }
        else
        {
            *dest = *src;
            dest++;
        }

        src++;
        max--;
    }

    *dest = 0;
    return dest;
}

static char *rsp_next_line(const char *line)
{
    char *end;

    // CRLF will be escaped if it is within a string.
    end = strstr(line, CRLF);
    if (end)
    {
        end = end + AT_STRING_LENGTH(CRLF);
    }

    // Return NULL if not found
    return end;
}

__attribute__((unused)) static uint16_t rsp_get_line_length(const char *line)
{
    char *end;

    // CRLF will be escaped if it is within a string.
    end = strstr(line, CRLF);
    if (end)
    {
        return end - line + 1;
    }
    return 0;
}

__attribute__((unused)) static uint16_t rsp_get_line_length_max(const char *line, const uint16_t max)
{
    char *end;
    uint16_t len;

    // CRLF will be escaped if it is within a string.
    end = strstr(line, CRLF);
    if (end)
    {
        len = end - line + 1;
        if (len < max)
        {
            return len;
        }
        return max;
    }
    return 0;
}

static char *rsp_next_param(const char *params)
{
    const char *cur;
    int8_t escape = 0;
    int8_t quotes = 0;

    cur = params;
    while (*cur)
    {
        if ((quotes == 0) && (*cur == '\"'))
        {
            quotes = 1;
        }
        else if ((quotes == 1) && (*cur == '\"'))
        {
            quotes = 0;
        }
        else if ((quotes == 1) && (escape == 0) && (*cur == '\\'))
        {
            escape = 1;
        }
        else if (escape == 0)
        {
            if (*cur == ',')
            {
                cur++;
                break;
            }
        }
        else
        {
            escape = 0;
        }

        cur++;
    }

    // Return NULL if not found.
    if (*cur)
        return (char *)cur;

    return NULL;
}

__attribute__((unused)) static uint16_t rsp_get_param_length(const char *params)
{
    char *end;


    // CRLF will be escaped if it is within a string.
    end = rsp_next_param(params);
    if (end)
    {
        return end - params - 1;
    }
    return AT_OK;
}

__attribute__((unused)) static uint16_t rsp_get_param_length_max(const char *params, const uint16_t max)
{
    char *end;
    uint16_t len;

    // CRLF will be escaped if it is within a string.
    end = rsp_next_param(params);
    if (end)
    {
        len = end - params - 1;
        if (len < max)
        {
            return len;
        }
        return max;
    }
    return AT_OK;
}


static  __attribute__((unused)) char *rsp_skip_cmd_echo(const char *line)
{
    char *end = (char *)line;

    if (at_echo == at_echo_on)
    {
        end = rsp_next_line(end);
    }
    return end;
}

static void peek_async_message(void)
{
    char message[MARKER_MAX_LENGTH] = {0};
    uint16_t count;
    int8_t found;

    do
    {
        count = uartrb_peek(uart_at, (uint8_t *)message, MARKER_MAX_LENGTH - 1);
        message[count] = '\0';

        if (count == 0)
        {
            break;
        }

        found = check_async_message(message, count);

        // Remove the async message from ring buffer.
        if (found)
        {
            count = uartrb_read(uart_at, (uint8_t *)message, found);
            at_txresponse(message, count);
        }

    } while (found);
}

static uint16_t check_async_message(char *message, uint16_t length)
{
    int8_t link_id;
    uint16_t found = 0;

    if (length >= AT_STRING_LENGTH(MARKER_IPD))
    {
        if (strncmp(message, MARKER_IPD, AT_STRING_LENGTH(MARKER_IPD)) == 0)
        {
            //tfp_printf("IPD received \r\n");
            async_ipd_receive();
        }
    }
    if (length >= AT_STRING_LENGTH(MARKER_WIFI_CONNECTED))
    {
        if (strncmp(message, MARKER_WIFI_CONNECTED, AT_STRING_LENGTH(MARKER_WIFI_CONNECTED)) == 0)
        {
            at_state_wifi_connected = 1;
            found = AT_STRING_LENGTH(MARKER_WIFI_CONNECTED);
        }
    }
    if (length >= AT_STRING_LENGTH(MARKER_WIFI_GOT_IP))
    {
        if (strncmp(message, MARKER_WIFI_GOT_IP, AT_STRING_LENGTH(MARKER_WIFI_GOT_IP)) == 0)
        {
            at_state_wifi_station_has_ip = 1;
            found = AT_STRING_LENGTH(MARKER_WIFI_GOT_IP);
        }
    }
    if (length >= AT_STRING_LENGTH(MARKER_WIFI_DISCONNECTED))
    {
        if (strncmp(message, MARKER_WIFI_DISCONNECTED, AT_STRING_LENGTH(MARKER_WIFI_DISCONNECTED)) == 0)
        {
            at_state_wifi_connected = 0;
            at_state_wifi_station_has_ip = 0;
            found = AT_STRING_LENGTH(MARKER_WIFI_DISCONNECTED);
        }
    }
    if (length >= AT_STRING_LENGTH(MARKER_SERVER_CONNECT))
    {
        if (strncmp(message + 1, MARKER_SERVER_CONNECT, AT_STRING_LENGTH(MARKER_SERVER_CONNECT)) == 0)
        {
            link_id = message[0] - '0';
            if (link_id >= AT_LINK_ID_MIN && link_id <= AT_LINK_ID_MAX)
            {
                at_state_server_connect[link_id] = at_connected;
            }
            found = AT_STRING_LENGTH(MARKER_SERVER_CONNECT) + 1;
        }
    }
    if (length >= AT_STRING_LENGTH(MARKER_SERVER_CLOSE))
    {
        if (strncmp(message + 1, MARKER_SERVER_CLOSE, AT_STRING_LENGTH(MARKER_SERVER_CLOSE)) == 0)
        {
            link_id = message[0] - '0';
            if (link_id >= AT_LINK_ID_MIN && link_id <= AT_LINK_ID_MAX)
            {
                at_state_server_connect[link_id] = at_not_connected;
            }
            found = AT_STRING_LENGTH(MARKER_SERVER_CLOSE) + 1;
        }
    }

    return found;
}

static char *rsp_check_response(const char *line, const char *expected)
{
    char *rspcolon;
    // Miss out the AT at the start of the query response.
    // e.g. command of AT+SLEEP? -> response of +SLEEP:1000\r\n
    const char *after_at = expected + AT_STRING_LENGTH(AT);

    // Check for a colon - this must be present
    rspcolon = strchr(line, ':');
    if (rspcolon)
    {
        // Compare the response from the '+' to before the '?'
        // sent in the command.
        if (strncmp(line, after_at, rspcolon - line) == 0)
        {
            // Return a pointer to the start of the parameters
            return rspcolon + 1;
        }
        if (strncmp(line, STATUS_CODE, AT_STRING_LENGTH(STATUS_CODE)) == 0)
        {
            // Return a pointer to the start of the parameters
            return rspcolon + 1;
        }
        if (strncmp(line, ERROR_CODE, AT_STRING_LENGTH(ERROR_CODE)) == 0)
        {
            // Return a pointer to the start of the parameters
            return rspcolon + 1;
        }
    }
    return NULL;
}

static int8_t cmd_execute_with_timeout(char *cmd, int cmdtimeout)
{
    char *rsp_buffer;
    int8_t rsp;

    uint16_t rsp_length = AT_MIN_RESPONSE + strlen(cmd);

    rsp_buffer = pvPortMalloc(rsp_length);
    if (!rsp_buffer) return AT_ERROR_RESOURCE;

    rsp = at_command(cmd, &rsp_length, rsp_buffer, cmdtimeout);
    // No response
    if (rsp)
    {
    }

    vPortFree(rsp_buffer);
    return rsp;
}

static int8_t cmd_execute(char *cmd)
{
    return cmd_execute_with_timeout(cmd, cmd_timeout);
}

static int8_t cmd_query_with_timeout(char *cmd, char *params, uint16_t param_max_len, int cmdtimeout)
{
    uint16_t rsp_length = AT_MIN_RESPONSE + strlen(cmd) + param_max_len;
    char *rsp_buffer;
    int8_t rsp;
    char *rspline;
    char *rspparams;
    uint16_t len = 0;

    *params = '\0';
    rsp_buffer = pvPortMalloc(rsp_length);
    if (!rsp_buffer) return AT_ERROR_RESOURCE;
    rsp = at_command(cmd, &rsp_length, rsp_buffer, cmdtimeout);
    if (rsp == AT_OK)
    {
        rspline = rsp_buffer;

        while (rspline)
        {
            // +COMMAND:
            rspparams = rsp_check_response(rspline, cmd);
            if (rspparams)
            {
                strncat(params, rspparams, rsp_get_line_length_max(rspparams, param_max_len - len));
                len = strlen(params);
                // Multiple entries to be separated by \r\n
                if ((param_max_len - len) > 2)
                {
                    strcat(params, "\r\n");
                    len += 2;
                }
            }
            rspline = rsp_next_line(rspline);
        }
    }
    else
    {
    }

    vPortFree(rsp_buffer);
    return rsp;
}

static int8_t cmd_query(char *cmd, char *params, uint16_t param_max_len)
{
    return cmd_query_with_timeout(cmd, params, param_max_len, cmd_timeout);
}

static int8_t cmd_set_with_timeout(char *cmd, char *params, int cmdtimeout)
{
    char *rsp_buffer, *cmd_buffer;
    int8_t rsp;

    uint16_t cmd_length = strlen(cmd) + AT_STRING_LENGTH(SET) + strlen(params) + AT_STRING_LENGTH(CRLF) + 1;
    uint16_t rsp_length = AT_MIN_RESPONSE + cmd_length;

    cmd_buffer = pvPortMalloc(cmd_length);
    if (!cmd_buffer) return AT_ERROR_RESOURCE;

    tfp_sprintf(cmd_buffer, "%s" SET "%s" CRLF, cmd, params);

    rsp_buffer = pvPortMalloc(rsp_length);
    if (!rsp_buffer) return AT_ERROR_RESOURCE;
    rsp = at_command(cmd_buffer, &rsp_length, rsp_buffer, cmdtimeout);
    if (rsp)
    {
    	DEBUG_PRINTF("Resp: %s \r\n", rsp_buffer);
    }

    vPortFree(cmd_buffer);
    vPortFree(rsp_buffer);
    return rsp;
}

static int8_t cmd_set(char *cmd, char *params)
{
    return cmd_set_with_timeout(cmd, params, cmd_timeout);
}

int8_t at_at(void)
{
    return cmd_execute("AT" CRLF);
}

int8_t at_rst(void)
{
    int8_t rsp;
    char rsp_buffer[16];
    uint16_t count;
    int8_t complete = 0;

    rsp = cmd_execute("AT+RST" CRLF);
    if (rsp == AT_OK)
    {
        at_echo = at_echo_on;
        at_cwlapopt_sort = at_cwlap_sort_ordered;
        at_cwlapopt_mask = at_cwlap_mask_all;
        at_cipmux = at_disable;
        at_cipmode = at_txmode_normal;
        at_cipdinfo = at_disable;

        // Wait for "ready"
        do
        {
            count = uartrb_readln(uart_at, (uint8_t *)rsp_buffer, sizeof(rsp_buffer));
            if (count > 0)
            {
                if (strncmp(rsp_buffer, READY_CODE, count) == 0)
                {
                    complete = 1;
                    rsp = AT_OK;
                }
            }
        } while (!complete);
    }

    return rsp;
}

int8_t at_gmr(struct at_cwgmr_s *gmr)
{
    uint16_t rsp_length = AT_MIN_RESPONSE + AT_MIN_COMMAND + AT_STRING_LENGTH(struct at_cwgmr_s);
    char *rsp_buffer;
    int8_t rsp;
    char *rspline;
    char *rspcolon;

    rsp_buffer = pvPortMalloc(rsp_length);
    if (!rsp_buffer) return AT_ERROR_RESOURCE;

    rsp = at_command("AT+GMR" CRLF, &rsp_length, rsp_buffer, cmd_timeout);
    if (rsp == AT_OK)
    {
        rsp = AT_ERROR_QUERY;
        rspline = rsp_buffer;
        {
            // Line 1 AT firmware version.
            rspcolon = strchr(rspline, ':');
            if (rspcolon) rspline = rspcolon + sizeof(char);
            if (gmr) strlcpy(gmr->at_version, rspline, rsp_get_line_length_max(rspline, AT_STRING_LENGTH(gmr->at_version)));

            rspline = rsp_next_line(rspline);
            if (rspline)
            {
                rspcolon = strchr(rspline, ':');
                if (rspcolon) rspline = rspcolon + sizeof(char);
                if (gmr) strlcpy(gmr->sdk_version, rspline, rsp_get_line_length_max(rspline, AT_STRING_LENGTH(gmr->sdk_version)));

                rspline = rsp_next_line(rspline);
                if (rspline)
                {
                    rspcolon = strchr(rspline, ':');
                    if (rspcolon) rspline = rspcolon + sizeof(char);
                    if (gmr) strlcpy(gmr->compile_time, rspline, rsp_get_line_length_max(rspline, AT_STRING_LENGTH(gmr->compile_time)));
                    rsp = AT_OK;
                }
            }
        }
    }

    vPortFree(rsp_buffer);
    return rsp;
}

int8_t at_set_gslp(int timeout)
{
    char params[AT_MAX_NUMBER];

    return cmd_set("AT+GSLP", params);
}

int8_t at_ate(int8_t on)
{
    char *cmd;
    int8_t rsp;

    if (on) cmd = "ATE1" CRLF;
    else cmd = "ATE0" CRLF;

    rsp = cmd_execute(cmd);
    if (rsp == AT_OK)
    {
        // No response
        at_echo = on;
    }

    return rsp;
}

int8_t at_restore(void)
{
    int8_t rsp;

    rsp = cmd_execute("AT+RESTORE" CRLF);
    if (rsp == AT_OK)
    {
        // No response
        at_echo = at_echo_on;
        at_cwlapopt_sort = at_cwlap_sort_ordered;
        at_cwlapopt_mask = at_cwlap_mask_all;
        at_cipmux = at_disable;
        at_cipmode = at_txmode_normal;
        at_cipdinfo = at_disable;
    }
    return rsp;
}

static int8_t helper_query_uart(char *cmd, struct at_cwuart_s *uart)
{
    int8_t rsp;
    char rspparams[AT_MAX_NUMBER * 5];
    char *rspnext = rspparams;

    if (uart == 0)
        return AT_ERROR_PARAMETERS;

    rsp = cmd_query(cmd, rspparams, sizeof(rspparams));
    if (rsp == AT_OK)
    {
        rsp = AT_ERROR_QUERY;
        uart->baud = strtol(rspnext, NULL, 10);
        rspnext = rsp_next_param(rspnext);
        if (rspnext)
        {
            uart->databits = strtol(rspnext, NULL, 10);
            rspnext = rsp_next_param(rspnext);
        }
        if (rspnext)
        {
            uart->stopbits = strtol(rspnext, NULL, 10);
            rspnext = rsp_next_param(rspnext);
        }
        if (rspnext)
        {
            uart->parity = strtol(rspnext, NULL, 10);
            rspnext = rsp_next_param(rspnext);
        }
        if (rspnext)
        {
            // Check there is an end using strol
            uart->flow = strtol(rspnext, &rspnext, 10);
        }
        if (rspnext)
        {
            rsp = AT_OK;
        }
    }

    return rsp;
}

int8_t at_set_uart_cur(struct at_cwuart_s *uart)
{
    char params[AT_MAX_NUMBER * 5];

    tfp_sprintf(params, "%ld,%d,%d,%d,%d", uart->baud,
            uart->databits, uart->stopbits,
            uart->parity, uart->flow);
    return cmd_set("AT+UART_CUR", params);
}

int8_t at_query_uart_cur(struct at_cwuart_s *uart)
{
    return helper_query_uart("AT+UART_CUR" QUERY CRLF, uart);
}

int8_t at_set_uart_def(struct at_cwuart_s *uart)
{
    char params[AT_MAX_NUMBER * 5];

    sprintf(params, "%ld,%d,%d,%d,%d", uart->baud,
            uart->databits, uart->stopbits,
            uart->parity, uart->flow);
    return cmd_set("AT+UART_DEF", params);
}

int8_t at_query_uart_def(struct at_cwuart_s *uart)
{
    return helper_query_uart("AT+UART_DEF" QUERY CRLF, uart);
}

int8_t at_set_sleep(enum at_enable on)
{
    char *cmd;

    if (on) cmd = "AT+SLEEP" SET "1" CRLF;
    else cmd = "AT+SLEEP" SET "0" CRLF;

    return cmd_execute(cmd);
}

int8_t at_query_sleep(enum at_enable *on)
{
    int8_t rsp;
    char rspparams[AT_MAX_NUMBER];
    char *rspnext = rspparams;

    if (on == 0)
        return AT_ERROR_PARAMETERS;

    rsp = cmd_query("AT+SLEEP" QUERY CRLF, rspparams, AT_MAX_NUMBER);
    if (rsp == AT_OK)
    {
        rsp = AT_ERROR_QUERY;
        *on = strtol(rspnext, &rspnext, 10);
        if (rspnext)
        {
            rsp = AT_OK;
        }
    }

    return rsp;
}

int8_t at_set_cwmode(enum at_mode mode)
{
    char params[AT_MAX_NUMBER];

    tfp_sprintf(params, "%d", mode);
    return cmd_set("AT+CWMODE", params);
}

int8_t at_query_cwmode(enum at_mode *mode)
{
    int8_t rsp;
    char rspparams[AT_MAX_NUMBER];
    char *rspnext = rspparams;

    if (mode == 0)
        return AT_ERROR_PARAMETERS;

    rsp = cmd_query("AT+CWMODE" QUERY CRLF, rspparams, AT_MAX_NUMBER);
    if (rsp == AT_OK)
    {
        rsp = AT_ERROR_QUERY;
        *mode = strtol(rspnext, &rspnext, 10);
        if (rspnext)
        {
            rsp = AT_OK;
        }
    }

    return rsp;
}

int8_t at_set_cwjap(struct at_set_cwjap_s *cwjap)
{
    char params[(AT_MAX_SSID_ESCAPED) + (AT_MAX_PWD_ESCAPED) + (AT_MAX_BSSID_ESCAPED)];
    char *paramend = params;

    paramend = helper_strcpy_param_escapify(paramend, cwjap->ssid, AT_MAX_SSID_ESCAPED);
    *paramend++ = ',';
    paramend = helper_strcpy_param_escapify(paramend, cwjap->pwd, AT_MAX_PWD_ESCAPED);
    if (cwjap->bssid[0])
    {
        *paramend++ = ',';
        paramend = helper_strcpy_param_escapify(paramend, cwjap->bssid, AT_MAX_BSSID_ESCAPED);
    }

    return cmd_set_with_timeout("AT+CWJAP", params, cmd_timeout_ap);
}

// Response when not connected to an AP
// AT+CWJAP?
// No AP
//
// OK

int8_t at_query_cwjap(struct at_query_cwjap_s *cwjap)
{
    int8_t rsp;
    char rspparams[(AT_MAX_SSID_ESCAPED) + (AT_MAX_PWD_ESCAPED) + (AT_MAX_BSSID_ESCAPED)];
    char *rspnext = rspparams;

    rsp = cmd_query("AT+CWJAP" QUERY CRLF, rspparams, (AT_MAX_SSID_ESCAPED) + (AT_MAX_PWD_ESCAPED) + (AT_MAX_BSSID_ESCAPED));

    if (rsp == AT_OK)
    {
        rsp = AT_ERROR_QUERY;
        if (cwjap != 0)
        {
            helper_strcpy_param_unescapify(cwjap->ssid, rspnext, rsp_get_param_length_max(rspnext, AT_STRING_LENGTH(cwjap->ssid)));
        }
        rspnext = rsp_next_param(rspnext);
        if (rspnext)
        {
            if (cwjap != 0)
            {
                helper_strcpy_param_unescapify(cwjap->bssid, rspnext, rsp_get_param_length_max(rspnext, AT_STRING_LENGTH(cwjap->bssid)));
            }
            rspnext = rsp_next_param(rspnext);
        }
        if (rspnext)
        {
            if (cwjap != 0)
            {
                cwjap->channel = strtol(rspnext, NULL, 10);
            }
            rspnext = rsp_next_param(rspnext);
        }
        if (rspnext)
        {
            if (cwjap != 0)
            {
                cwjap->strength = strtol(rspnext, &rspnext, 10);
            }
        }
        if (rspnext)
        {
            rsp = AT_OK;
        }
    }

    return rsp;
}

int8_t at_set_cwlapopt(int8_t sort, int8_t mask)
{
    int8_t rsp;
    char params[(AT_MAX_NUMBER * 2)];

    tfp_sprintf(params, "%d,%d", sort, mask);
    rsp = cmd_set("AT+CWLAPOPT", params);
    if (rsp == AT_OK)
    {
        at_cwlapopt_sort = sort;
        at_cwlapopt_mask = mask;
    }

    return rsp;
}

int8_t at_cwlap(struct at_cwlap_s *rsp_cwlap, int8_t *entries)
{
    char rspline[((AT_MAX_SSID_ESCAPED) + (AT_MAX_NUMBER * 3) + (AT_MAX_BSSID_ESCAPED))];
    char *rspparams;
    char *rspnext;
    uint16_t count;
    int8_t complete = 0;
    int8_t slot = 0;
    int8_t rsp = AT_ERROR_RESPONSE;

    if ((rsp_cwlap == 0) || (entries == 0))
        return AT_ERROR_PARAMETERS;

    at_txcommand("AT+CWLAP" CRLF);

    // Read in echoed command and ignore.
    if (at_echo == at_echo_on)
    {
        xTimerChangePeriod(at_timer, at_rx_timeout_cmd, 0);

        count = uartrb_readln(uart_at, (uint8_t *)rspline, AT_STRING_LENGTH(rspline));
        rspnext = &rspline[count];

        if (xTimerIsTimerActive(at_timer) == pdFALSE)
        {
            rsp = AT_ERROR_TIMEOUT;
        }
        xTimerStop(at_timer, 0);

        at_txresponse(rspline, count);
    }

    if (rsp == AT_ERROR_TIMEOUT)
    {
        return rsp;
    }

    xTimerChangePeriod(cmd_timer, cmd_timeout_ap, 0);

    do
    {
        count = uartrb_readln(uart_at, (uint8_t *)rspline, AT_STRING_LENGTH(rspline));
        if (count > 0)
        {
            if (strncmp(rspline, OK, count) == 0)
            {
                complete = 1;
                rsp = AT_OK;
            }
            else if (strncmp(rspline, ERROR, count) == 0)
            {
                complete = -1;
            }
            else if (slot < *entries)
            {
                rspparams = rsp_check_response(rspline, "AT+CWLAP" CRLF);
                if (rspparams)
                {
                    rspnext = rspparams;

                    if (*rspnext == '(')
                    {
                        rspnext++;
                    }

                    if ((rspnext) && (at_cwlapopt_mask & at_cwlap_mask_ecn))
                    {
                        rsp_cwlap[slot].ecn = strtol(rspnext, NULL, 10);
                        rspnext = rsp_next_param(rspnext);
                    }
                    else
                    {
                        rsp_cwlap[slot].ecn = 0;
                    }

                    rsp_cwlap[slot].ssid[0] = 0;
                    if ((rspnext) && (at_cwlapopt_mask & at_cwlap_mask_ssid))
                    {
                        helper_strcpy_param_unescapify(rsp_cwlap[slot].ssid, rspnext, rsp_get_param_length_max(rspnext, AT_STRING_LENGTH(rsp_cwlap[0].ssid)));
                        rspnext = rsp_next_param(rspnext);
                    }

                    if ((rspnext) && (at_cwlapopt_mask & at_cwlap_mask_strength))
                    {
                        rsp_cwlap[slot].strength = strtol(rspnext, &rspnext, 10);
                        rspnext = rsp_next_param(rspnext);
                    }
                    else
                    {
                        rsp_cwlap[slot].strength = 0;
                    }

                    rsp_cwlap[slot].bssid[0] = 0;
                    if ((rspnext) && (at_cwlapopt_mask & at_cwlap_mask_bssid))
                    {
                        helper_strcpy_param_unescapify(rsp_cwlap[slot].bssid, rspnext, rsp_get_param_length_max(rspnext, AT_STRING_LENGTH(rsp_cwlap[0].bssid)));
                        rspnext = rsp_next_param(rspnext);
                    }

                    if ((rspnext) && (at_cwlapopt_mask & at_cwlap_mask_channel))
                    {
                        rsp_cwlap[slot].channel = strtol(rspnext, &rspnext, 10);
                    }
                    else
                    {
                        rsp_cwlap[slot].channel = 0;
                    }

                    slot++;
                }
            }

            if (xTimerIsTimerActive(cmd_timer) == pdFALSE)
            {
                rsp = AT_ERROR_TIMEOUT;
                break;
            }

            xTimerReset(cmd_timer, 0);
        }

    } while (!complete);

    xTimerStop(cmd_timer, 0);

    *entries = slot;

    return rsp;
}

int8_t at_cwqap(void)
{
    return cmd_execute("AT+CWQAP" CRLF);
}

int8_t at_query_cwsap(void)
{
    return AT_ERROR_NOT_SUPPORTED;
}

int8_t at_set_cwsap(void)
{
    return AT_ERROR_NOT_SUPPORTED;
}

int8_t at_query_cwlif(void)
{
    return AT_ERROR_NOT_SUPPORTED;
}

int8_t at_query_cwdhcp(struct at_cwdhcp_s *cwdhcp)
{
    int8_t rsp;
    char rspparams[AT_MAX_NUMBER];
    char *rspnext = rspparams;

    if (cwdhcp == 0)
        return AT_ERROR_PARAMETERS;

    rsp = cmd_query("AT+CWDHCP" QUERY CRLF, rspparams, sizeof(rspparams));
    if (rsp == AT_OK)
    {
        int8_t map;

        rsp = AT_ERROR_QUERY;
        map = strtol(rspnext, &rspnext, 10) & 0x7f;
        if (rspnext)
        {
            cwdhcp->station = (map & at_cwdhcp_station)?at_enable:at_disable;
            cwdhcp->soft_ap = (map & at_cwdhcp_soft_ap)?at_enable:at_disable;
            rsp = AT_OK;
        }
    }

    return rsp;
}

int8_t at_set_cwdhcp(enum at_enable operation, struct at_cwdhcp_s *cwdhcp)
{
    int8_t rsp;
    char params[AT_MAX_NUMBER * 2];
    int8_t mode;

    mode = (cwdhcp->station == at_enable?at_cwdhcp_station:0)
                                                                                                                                                                                                                                                            + (cwdhcp->soft_ap == at_enable?at_cwdhcp_soft_ap:0);
    tfp_sprintf(params, "%d,%d", operation, mode);
    rsp = cmd_set("AT+CWDHCP", params);
    if (rsp == AT_OK)
    {
        // No action.
    }

    return rsp;
}

int8_t at_query_cwdhcps(void)
{
    //"AT+CWDHCPS?"
    return AT_ERROR_NOT_SUPPORTED;
}

int8_t at_set_cwdhcps(void)
{
    //"AT+CWDHCPS="
    return AT_ERROR_NOT_SUPPORTED;
}

int8_t at_set_cwautoconn(enum at_enable enable)
{
    int8_t rsp;
    char params[(AT_MAX_NUMBER * 1)];

    tfp_sprintf(params, "%d", enable);
    rsp = cmd_set("AT+CWAUTOCONN", params);
    if (rsp == AT_OK)
    {
        // No action.
    }

    return rsp;
}

int8_t at_query_cwautoconn(enum at_enable *enable)
{
    int8_t rsp;
    char rspparams[AT_MAX_NUMBER];
    char *rspnext = rspparams;

    if (enable == 0)
        return AT_ERROR_PARAMETERS;

    rsp = cmd_query("AT+CWAUTOCONN" QUERY CRLF, rspparams, sizeof(rspparams));
    if (rsp == AT_OK)
    {
        rsp = AT_ERROR_QUERY;
        *enable = strtol(rspnext, &rspnext, 10);
        if (rspnext)
        {
            rsp = AT_OK;
        }
    }

    return rsp;
}

int8_t at_query_cipstamac(char *mac)
{
    int8_t rsp;
    char rspparams[AT_MAX_MAC * 2];
    char *rspnext = rspparams;

    if (mac == 0)
        return AT_ERROR_PARAMETERS;

    rsp = cmd_query("AT+CIPSTAMAC" QUERY CRLF, rspparams, sizeof(rspparams));
    if (rsp == AT_OK)
    {
        helper_strcpy_param_unescapify(mac, rspnext, rsp_get_param_length_max(rspnext, AT_MAX_MAC));
    }

    return rsp;
}

int8_t at_set_cipstamac(char *mac)
{
    char params[AT_MAX_MAC * 2];
    char *paramend = params;

    paramend = helper_strcpy_param_escapify(paramend, mac, AT_MAX_MAC * 2);

    return cmd_set("AT+CIPSTAMAC", params);
}

int8_t at_query_cipapmac(char *mac)
{
    int8_t rsp;
    char rspparams[AT_MAX_MAC * 2];
    char *rspnext = rspparams;

    rsp = cmd_query("AT+CIPAPMAC" QUERY CRLF, rspparams, sizeof(rspparams));
    if (rsp == AT_OK)
    {
        if (mac != 0)
        {
            helper_strcpy_param_unescapify(mac, rspnext, rsp_get_param_length_max(rspnext, AT_MAX_MAC));
        }
    }

    return rsp;
}

int8_t at_set_cipapmac(char *mac)
{
    char params[AT_MAX_MAC * 2];
    char *paramend = params;

    paramend = helper_strcpy_param_escapify(paramend, mac, AT_MAX_MAC * 2);

    return cmd_set("AT+CIPAPMAC", params);
}

static int8_t at_query_cip_helper(char *cmd, char *ip, char *gateway, char *mask)
{
    int8_t rsp;
    char rspparams[((AT_MAX_IP * 2) * 3) + 128];
    char *rspnext;
    char *rspline = rspparams;
    char *rspcolon;

    rsp = cmd_query(cmd, rspparams, sizeof(rspparams));
    if (rsp == AT_OK)
    {
        rsp = AT_ERROR_QUERY;
        rspcolon = strchr(rspline, ':');
        if (rspcolon)
        {
            rspnext = rspcolon + sizeof(char);
            if (ip != 0)
            {
                helper_strcpy_param_unescapify(ip, rspnext, rsp_get_line_length_max(rspnext, AT_MAX_IP));
            }
            rspline = rsp_next_line(rspline);
            if (rspline)
            {
                rspcolon = strchr(rspline, ':');
                if (rspcolon)
                {
                    rspnext = rspcolon + sizeof(char);
                    if (gateway)
                    {
                        helper_strcpy_param_unescapify(gateway, rspnext, rsp_get_line_length_max(rspnext, AT_MAX_IP));
                    }
                    rspline = rsp_next_line(rspline);
                    if (rspline)
                    {
                        rspcolon = strchr(rspline, ':');
                        if (rspcolon)
                        {
                            rspnext = rspcolon + sizeof(char);
                            if (mask)
                            {
                                helper_strcpy_param_unescapify(mask, rspnext, rsp_get_line_length_max(rspnext, AT_MAX_IP));
                            }
                            rsp = AT_OK;
                        }
                    }
                }
            }
        }
    }
    return rsp;
}

int8_t at_query_cipsta(char *ip, char *gateway, char *mask)
{
    return at_query_cip_helper("AT+CIPSTA" QUERY CRLF, ip, gateway, mask);
}

int8_t at_query_cipap(char *ip, char *gateway, char *mask)
{
    return at_query_cip_helper("AT+CIPAP" QUERY CRLF, ip, gateway, mask);
}

int8_t at_query_cipdomain(char* domain, char* ipChar)
{
    char* cmd_buffer = pvPortMalloc(20 + strlen(domain));
    char *resp = ipChar;
    tfp_sprintf(cmd_buffer, "AT+CIPDOMAIN=\"%s\"" CRLF, domain);
    int8_t at_ret = cmd_query(cmd_buffer , resp, 16);
    vPortFree(cmd_buffer);
    return at_ret;
}
static int8_t at_set_cip_sta_ap_helper(char *cmd, char *ip, char *gateway, char *mask)
{
    char params[((AT_MAX_IP * 2) * 3) + 128];
    char *paramend = params;

    paramend = helper_strcpy_param_escapify(paramend, ip, AT_MAX_IP * 2);
    if (gateway)
    {
        *paramend++ = ',';
        paramend = helper_strcpy_param_escapify(paramend, gateway, AT_MAX_IP * 2);
        if (mask)
        {
            *paramend++ = ',';
            paramend = helper_strcpy_param_escapify(paramend, mask, AT_MAX_IP * 2);
        }
    }
    return cmd_set(cmd, params);
}

int8_t at_set_cipsta(char *ip, char *gateway, char *mask)
{
    return at_set_cip_sta_ap_helper("AT+CIPSTA", ip, gateway, mask);
}

int8_t at_set_cipap(char *ip, char *gateway, char *mask)
{
    return at_set_cip_sta_ap_helper("AT+CIPAP", ip, gateway, mask);
}

int8_t at_cwstartsmart(void)
{
    return cmd_execute("AT+CWSTARTSMART" CRLF);
}

int8_t at_set_cwstartsmart(enum at_smartconfig_type type)
{
    char params[AT_MAX_NUMBER * 2];

    tfp_sprintf(params, "%d", type);

    return cmd_set("AT+CWSTARTSMART", params);
}

int8_t at_cwstopsmart(void)
{
    return cmd_execute("AT+CWSTOPSMART" CRLF);
}

int8_t at_wps(enum at_enable type)
{
    char params[AT_MAX_NUMBER * 2];

    tfp_sprintf(params, "%d", type);

    return cmd_set("AT+WPS", params);
}

int8_t at_query_cipstatus(enum at_cipstatus *status, int8_t *count, struct at_cipstatus_s *cipstatus)
{
    int8_t rsp;
    char rspparams[((AT_MAX_IP + (AT_MAX_NUMBER * 4)) * 5) + (16 * 5)];
    char *rspnext;
    char *rspline = rspparams;
    enum at_cipstatus rspstatus = -1;
    int8_t slot = 0;

    if (status == 0)
        return AT_ERROR_PARAMETERS;
    if ((count == 0) && (cipstatus != 0))
        return AT_ERROR_PARAMETERS;

    rsp = cmd_query("AT+CIPSTATUS" CRLF, rspparams, sizeof(rspparams));
    if (rsp == AT_OK)
    {
        rsp = AT_ERROR_QUERY;
        rspstatus = strtol(rspline, NULL, 10);
        *status = rspstatus;

        if (count)
        {
            rsp = AT_ERROR_QUERY;

            while (slot < *count)
            {
                rspline = rsp_next_line(rspline);
                if (rspline)
                {
                    rspnext = rspline;
                    if (cipstatus)
                    {
                        cipstatus[slot].link_id = strtol(rspnext, NULL, 10);

                        rspnext = rsp_next_param(rspnext);
                        if (rspnext)
                        {
                            if (strncmp(rspnext, "\"TCP\"", 3) == 0)
                            {
                                cipstatus[slot].type = at_link_type_tcp;
                            }
                            if (strncmp(rspnext, "\"UDP\"", 3) == 0)
                            {
                                cipstatus[slot].type = at_link_type_udp;
                            }
                            rspnext = rsp_next_param(rspnext);
                        }
                        if (rspnext)
                        {
                            helper_strcpy_param_unescapify(cipstatus[slot].remote_ip, rspnext, rsp_get_line_length_max(rspnext, AT_MAX_IP));
                            rspnext = rsp_next_param(rspnext);
                        }
                        if (rspnext)
                        {
                            cipstatus[slot].remote_port = strtol(rspnext, NULL, 10);
                            rspnext = rsp_next_param(rspnext);
                        }
                        if (rspnext)
                        {
                            cipstatus[slot].local_port = strtol(rspnext, NULL, 10);
                            rspnext = rsp_next_param(rspnext);
                        }
                        if (rspnext)
                        {
                            cipstatus[slot].tetype = strtol(rspnext, &rspnext, 10);
                        }
                    }
                    if (rspnext)
                    {
                        rsp = AT_OK;
                        slot++;
                    }
                    else
                    {
                        break;
                    }
                }

                if (rsp != AT_OK)
                {
                    break;
                }
            }

            *count = slot;
        }
        else
        {
            rsp = AT_OK;
        }
    }

    return rsp;
}

int8_t at_set_cipdomain(char *domain)
{
    char params[AT_MAX_DOMAIN * 2];
    char *paramend = params;

    paramend = helper_strcpy_param_escapify(paramend, domain, AT_MAX_DOMAIN * 2);

    return cmd_set("AT+CIPDOMAIN", params);
}

static int8_t at_set_cipstart_tcp_helper(int8_t link_id, char *type, char *remote_ip, uint16_t remote_port, uint16_t tcp_keep_alive)
{
    char params[AT_MAX_DOMAIN + (AT_MAX_NUMBER * 3) + 8];
    char *paramend = params;

    if (at_cipmux == at_enable)
    {
        paramend += tfp_sprintf(paramend, "%d,", link_id);
    }
    paramend = helper_strcpy_param_escapify(paramend, type, AT_MAX_TRANSPORT * 2);
    *paramend++ = ',';
    paramend = helper_strcpy_param_escapify(paramend, remote_ip, AT_MAX_DOMAIN);
    *paramend++ = ',';
    paramend += tfp_sprintf(paramend, "%d", remote_port);
    if (tcp_keep_alive > 0)
    {
        *paramend++ = ',';
        paramend += tfp_sprintf(paramend, "%d", tcp_keep_alive);
    }

    /*     Possible responses are:

    AT+CIPSTART="TCP","xxxx",1234
    ALREADY CONNECTED

    ERROR

    AT+CIPSTART="TCP","xxxx",1234
    CONNECT

    OK
     */
    return cmd_set_with_timeout("AT+CIPSTART", params, cmd_timeout_inet);
}

static int8_t at_set_cipstart_udp_helper(int8_t link_id, char *remote_ip, uint16_t remote_port, uint16_t local_port, int8_t udp_mode)
{
    char params[(AT_MAX_IP * 2) + (AT_MAX_NUMBER * 3) + 8];
    char *paramend = params;

    if (at_cipmux == at_enable)
    {
        paramend += tfp_sprintf(paramend, "%d,", link_id);
    }
    paramend = helper_strcpy_param_escapify(paramend, "UDP,", AT_MAX_TRANSPORT * 2);
    paramend = helper_strcpy_param_escapify(paramend, remote_ip, AT_MAX_IP * 2);
    *paramend++ = ',';
    paramend += tfp_sprintf(paramend, "%d", remote_port);
    if (local_port > 0)
    {
        *paramend++ = ',';
        paramend += tfp_sprintf(paramend, "%d", local_port);
        *paramend++ = ',';
        paramend += tfp_sprintf(paramend, "%d", udp_mode);
    }

    return cmd_set("AT+CIPSTART", params);
}

int8_t at_set_cipstart_tcp(int8_t link_id, char *remote_ip, uint16_t remote_port, uint16_t tcp_keep_alive)
{
    return at_set_cipstart_tcp_helper(link_id, "TCP", remote_ip, remote_port, tcp_keep_alive);
}

int8_t at_set_cipstart_udp(int8_t link_id, char *remote_ip, uint16_t remote_port, uint16_t local_port, int8_t udp_mode)
{
    return at_set_cipstart_udp_helper(link_id, remote_ip, remote_port, local_port, udp_mode);
}

int8_t at_set_cipstart_ssl(int8_t link_id, char *remote_ip, uint16_t remote_port, uint16_t tcp_keep_alive)
{
    return at_set_cipstart_tcp_helper(link_id, "SSL", remote_ip, remote_port, tcp_keep_alive);
}

int8_t at_set_cipstart_tls_helper(int8_t link_id, char *type, char *remote_ip, uint16_t remote_port, uint16_t tcp_keep_alive)
{
    char params[AT_MAX_DOMAIN + (AT_MAX_NUMBER * 3) + 8];
    char *paramend = params;

    if (at_cipmux == at_enable)
    {
        paramend += tfp_sprintf(paramend, "%d,", link_id);
    }
    paramend = helper_strcpy_param_escapify(paramend, type, AT_MAX_TRANSPORT * 2);
    *paramend++ = ',';
    paramend = helper_strcpy_param_escapify(paramend, remote_ip, AT_MAX_DOMAIN);
    *paramend++ = ',';
    paramend += tfp_sprintf(paramend, "%d", remote_port);
    if (tcp_keep_alive > 0)
    {
        *paramend++ = ',';
        paramend += tfp_sprintf(paramend, "%d", tcp_keep_alive);
    }

    /*   Possible responses are:

    AT+CIPSTART="TCP","xxxx",1234
    ALREADY CONNECTED

    ERROR

    AT+CIPSTART="TCP","xxxx",1234
    CONNECT

    OK
     */
    return cmd_set_with_timeout("AT+TLSCONN", params, 20000);
}
int8_t at_set_cipstart_tls(int8_t link_id, char *remote_ip, uint16_t remote_port, uint16_t tcp_keep_alive)
{
    return at_set_cipstart_tls_helper(link_id, "TCP", remote_ip, remote_port, tcp_keep_alive);
}
static int8_t at_set_cipsend_all_helper(char *cmd, int8_t link_id, uint16_t length, uint8_t *buffer, char *remote_ip, uint16_t remote_port)
{
    int8_t rsp;
    char params[(AT_MAX_IP * 2) + (AT_MAX_NUMBER * 3) + 8];
    char *paramend = params;
    char rsp_buffer[16 + AT_MAX_NUMBER];
    uint16_t count;
    int8_t complete = 0;

    if (at_cipmux == at_enable)
    {
        paramend += tfp_sprintf(paramend, "%d,", link_id);
    }
    paramend += tfp_sprintf(paramend, "%d", length);
    if (remote_ip)
    {
        *paramend++ = ',';
        paramend = helper_strcpy_param_escapify(paramend, remote_ip, AT_MAX_IP * 2);
        paramend += tfp_sprintf(paramend, ",%d", remote_port);
    }

    rsp = cmd_set_with_timeout(cmd, params, cmd_timeout_inet);

    if (rsp == AT_OK)
    {
        rsp = AT_ERROR_SET;

        DEBUG_PRINTF("Wait for > \r\n");
        // Wait for ">"
        do
        {
            uartrb_getc(uart_at, (uint8_t *)rsp_buffer);

        } while (*rsp_buffer != '>');
        DEBUG_PRINTF("Write data \r\n");
        uartrb_write_wait(uart_at, buffer, length);
        DEBUG_PRINTF("Wait for SEND OK \r\n");
        do
        {
            count = uartrb_readln(uart_at, (uint8_t *)rsp_buffer, sizeof(rsp_buffer));
//            DEBUG_PRINTF("Resp: %s \r\n", rsp_buffer);
            if (count > 0)
            {
                if (strncmp(rsp_buffer, "SEND OK", count) == 0)
                {
                    complete = 1;
                    rsp = AT_OK;
                }
            }
        } while (!complete);
        DEBUG_PRINTF("Finished \r\n");
    }

    return rsp;
}

static inline int8_t at_set_cipsend_helper(int8_t link_id, uint16_t length, uint8_t *buffer, char *remote_ip, uint16_t remote_port)
{
    return at_set_cipsend_all_helper("AT+CIPSEND", link_id, length, buffer, remote_ip, remote_port);
}

static int8_t at_set_cipsendex_helper(int8_t link_id, uint16_t length, uint8_t *buffer, char *remote_ip, uint16_t remote_port)
{
    return at_set_cipsend_all_helper("AT+CIPSENDEX", link_id, length, buffer, remote_ip, remote_port);
}

int8_t at_set_cipsend(int8_t link_id, uint16_t length, uint8_t *buffer)
{
    return at_set_cipsend_helper( link_id, length, buffer, NULL, 0);
}

int8_t at_set_cipsend_udp(int8_t link_id, uint16_t length, uint8_t *buffer, char *remote_ip, uint16_t remote_port)
{
    return at_set_cipsend_helper(link_id, length, buffer, remote_ip, remote_port);
}

int8_t at_cipsend_start(void)
{
    return cmd_execute("AT+CIPSEND" CRLF);
}

int8_t at_cipsend_finish(void)
{
    return cmd_execute("+++");
}
int8_t at_set_tlssend(int8_t link_id, uint16_t length, uint8_t *buffer)
{
    return at_set_cipsend_all_helper("AT+TLSSEND", link_id, length, buffer, NULL, 0);
}
int8_t at_set_cipsendex(int8_t link_id, uint16_t length, uint8_t *buffer)
{
    return at_set_cipsendex_helper( link_id, length, buffer, NULL, 0);
}

int8_t at_set_cipsendex_udp(int8_t link_id, uint16_t length, uint8_t *buffer, char *remote_ip, uint16_t remote_port)
{
    return at_set_cipsendex_helper(link_id, length, buffer, remote_ip, remote_port);
}

int8_t at_set_cipclose(int8_t link_id)
{
    char params[AT_MAX_NUMBER * 2];

    if (at_cipmux == at_enable)
    {
        tfp_sprintf(params, "%d", link_id);

        return cmd_set("AT+CIPCLOSE", params);
    }

    return cmd_execute("AT+CIPCLOSE" CRLF);
}

int8_t at_set_tlsclose(int8_t link_id)
{
    char params[AT_MAX_NUMBER * 2];

    if (at_cipmux == at_enable)
    {
        tfp_sprintf(params, "%d", link_id);

        return cmd_set("AT+TLSCLOSE", params);
    }
    else
    {
        tfp_printf("CIPMUX must be enabled \n");
        return AT_OK;
    }
}

int8_t at_query_cifsr(char *soft_ap, char *station)
{
    int8_t rsp;
    char rspparams[((AT_MAX_IP * 2) * 2) + 128];
    char *rspline = rspparams;

    if ((soft_ap == 0) || (station == 0))
        return AT_ERROR_PARAMETERS;

    rsp = cmd_query("AT+CIFSR" CRLF, rspparams, sizeof(rspparams));
    if (rsp == AT_OK)
    {
        rsp = AT_ERROR_QUERY;
        helper_strcpy_param_unescapify(soft_ap, rspline, rsp_get_line_length_max(rspline, AT_MAX_IP));
        rspline = rsp_next_line(rspline);
        if (rspline)
        {
            helper_strcpy_param_unescapify(station, rspline, rsp_get_line_length_max(rspline, AT_MAX_IP));
            rsp = AT_OK;
        }
    }
    return rsp;
}

int8_t at_query_cipmux(enum at_enable *enable)
{
    int8_t rsp;
    char rspparams[AT_MAX_NUMBER];
    char *rspnext = rspparams;

    if (enable == 0)
        return AT_ERROR_PARAMETERS;

    rsp = cmd_query("AT+CIPMUX" QUERY CRLF, rspparams, AT_MAX_NUMBER);
    if (rsp == AT_OK)
    {
        rsp = AT_ERROR_QUERY;
        *enable = strtol(rspnext, &rspnext, 10);
        if (rspnext)
        {
            at_cipmux = *enable;
            rsp = AT_OK;
        }
    }

    return rsp;
}

int8_t at_set_cipmux(enum at_enable enable)
{
    char params[AT_MAX_NUMBER];
    int8_t rsp;

    tfp_sprintf(params, "%d", enable);

    rsp = cmd_set("AT+CIPMUX", params);
    if (rsp == AT_OK)
    {
        at_cipmux = enable;
    }

    return rsp;
}

int8_t at_set_cipserver(enum at_enable mode, uint16_t port)
{
    int8_t rsp;
    char params[AT_MAX_NUMBER * 2];

    tfp_sprintf(params, "%d,%d", mode, port);
    rsp = cmd_set_with_timeout("AT+CIPSERVER", params, cmd_timeout_inet);
    if (rsp == AT_OK)
    {
        // No action.
    }

    return rsp;
}

int8_t at_set_cipmode(enum at_txmode mode)
{
    char params[AT_MAX_NUMBER];
    int8_t rsp;

    tfp_sprintf(params, "%d", mode);

    rsp = cmd_set("AT+CIPMODE", params);

    if (rsp == AT_OK)
    {
        at_cipmode = mode;
    }

    return rsp;
}

int8_t at_query_cipmode(enum at_txmode *mode)
{
    int8_t rsp;
    char rspparams[AT_MAX_NUMBER];
    char *rspnext = rspparams;

    if (mode == 0)
        return AT_ERROR_PARAMETERS;

    rsp = cmd_query("AT+CIPMODE" QUERY CRLF, rspparams, AT_MAX_NUMBER);
    if (rsp == AT_OK)
    {
        rsp = AT_ERROR_QUERY;
        *mode = strtol(rspnext, &rspnext, 10);
        if (rspnext)
        {
            rsp = AT_OK;
            at_cipmode = *mode;
        }
    }

    return rsp;
}

int8_t at_set_cipsto(uint16_t timeout)
{
    char params[AT_MAX_NUMBER];

    tfp_sprintf(params, "%d", timeout);

    return cmd_set("AT+CIPSTO", params);
}

int8_t at_query_cipsto(uint16_t *timeout)
{
    int8_t rsp;
    char rspparams[AT_MAX_NUMBER];
    char *rspnext = rspparams;

    if (timeout == 0)
        return AT_ERROR_PARAMETERS;

    rsp = cmd_query("AT+CIPSTO" QUERY CRLF, rspparams, AT_MAX_NUMBER);
    if (rsp == AT_OK)
    {
        rsp = AT_ERROR_QUERY;
        *timeout = strtol(rspnext, &rspnext, 10);
        if (rspnext)
        {
            rsp = AT_OK;
        }
    }

    return rsp;
}

int8_t at_set_cipdinfo(enum at_enable enable)
{
    char params[AT_MAX_NUMBER];
    int8_t rsp;

    tfp_sprintf(params, "%d", enable);

    rsp = cmd_set("AT+CIPDINFO", params);

    if (rsp == AT_OK)
    {
        at_cipdinfo = enable;
    }

    return rsp;
}

int8_t at_query_cipdinfo(enum at_enable *enable)
{
    int8_t rsp;
    char rspparams[AT_MAX_NUMBER];
    char *rspnext = rspparams;

    if (enable == 0)
        return AT_ERROR_PARAMETERS;

    rsp = cmd_query("AT+CIPDINFO" QUERY CRLF, rspparams, AT_MAX_NUMBER);
    if (rsp == AT_OK)
    {
        rsp = AT_ERROR_QUERY;
        if (strncmp(rspnext, "TRUE", 4) == 0)
        {
            *enable = at_enable;
            at_cipdinfo = at_enable;
            rsp = AT_OK;
        }
        else if (strncmp(rspnext, "FALSE", 5) == 0)
        {
            *enable = at_disable;
            at_cipdinfo = at_disable;
            rsp = AT_OK;
        }
    }

    return rsp;
}

int8_t at_register_ipd(uint16_t length, uint8_t *buffer)
{
    struct ipd_store *new_ipd;
    struct ipd_store *end_ipd;

    new_ipd = pvPortMalloc(sizeof(struct ipd_store));
    if (!new_ipd) return AT_ERROR_RESOURCE;

    memset(&new_ipd->remote_ip, 0, sizeof(((struct ipd_store *)0)->remote_ip));
    new_ipd->valid = at_ipd_status_waiting;
    new_ipd->length = length;
    new_ipd->buffer = buffer;
    new_ipd->link_id = 0;
    new_ipd->next = NULL;

    if (ipd_head == NULL)
    {
        ipd_head = new_ipd;
        ipd_write = ipd_head;
    }
    else
    {
        end_ipd = ipd_head;
        while (end_ipd->next)
        {
            end_ipd = end_ipd->next;
        }
        end_ipd->next = new_ipd;
    }

    return AT_OK;
}

int8_t at_delete_ipd(uint8_t *buffer)
{
    struct ipd_store *end_ipd;
    struct ipd_store *prev_ipd;

    if (ipd_head != NULL)
    {
        end_ipd = ipd_head;
        prev_ipd = NULL;

        while (end_ipd)
        {
            if (end_ipd->buffer == buffer)
            {
                if (prev_ipd == NULL)
                {
                    // Remove this entry if it is the first in the list
                    ipd_head = end_ipd->next;
                    vPortFree(end_ipd);
                    break;
                }
                else
                {
                    // Remove this entry if it is elsewhere
                    prev_ipd = end_ipd->next;
                    vPortFree(end_ipd);
                    break;
                }

            }
            // Check next entry in the list
            prev_ipd = end_ipd;
            end_ipd = end_ipd->next;
        }
    }

    return AT_OK;
}

static int8_t async_ipd_receive(void)
{
    char rspparams[16 + (AT_MAX_NUMBER * 3) + AT_MAX_IP];
    char *rspnext;
    char *rspcolon;
    uint16_t packetlen = 0;
    uint16_t infolen;
    int8_t rsp = AT_NO_DATA;
    char val;

    at_state_ipd_pending = 0;

    if (ipd_write == NULL)
    {
        ipd_write = ipd_head;

        while (ipd_write)
        {
            // Look for a free slot
            if (ipd_write->valid == at_ipd_status_waiting)
            {
                break;
            }
            ipd_write = ipd_write->next;
        }
    }

    if ((ipd_head == NULL) || (ipd_write == NULL))
    {
        uartrb_flush_read(uart_at);
        return AT_NO_DATA;
    }

    // +IPD found. Process it.
    rsp = AT_ERROR_RESPONSE;

    xTimerChangePeriod(at_timer, at_rx_timeout_cmd, 0);

    // Store info string up to the colon ":" which precedes the data.
    rspnext = rspparams;
    infolen = sizeof(rspparams);
    do
    {
        while (uartrb_getc(uart_at, (uint8_t *)&val) == 0);
        *rspnext++ = val;
        if (infolen-- == 0)
        {
            rspnext = 0;
            break;
        }

        if (xTimerIsTimerActive(at_timer) == pdFALSE)
        {
            tfp_printf("[%s:%d] Time out", __FUNCTION__, __LINE__);
            rsp = AT_ERROR_TIMEOUT;
            break;
        }
    } while (val != ':');

    xTimerStop(at_timer, 0);

    if (rsp == AT_ERROR_TIMEOUT)
    {
        return rsp;
    }

    // Null terminate the info string and point parsing point to start.
    if (rspnext)
    {
        *rspnext = '\0';
        rspnext = rspparams;
    }

    at_txresponse(rspparams, strlen(rspparams));

    vTaskDelay (pdMS_TO_TICKS(50));

    xTimerChangePeriod(at_timer, at_rx_timeout_cmd * 1000, 0);

    rspnext = rspparams + AT_STRING_LENGTH(MARKER_IPD);

    if (rspnext)
    {
        if (at_cipmux == at_enable)
        {
            ipd_write->link_id = strtol(rspnext, &rspnext, 10);
            rspnext = rsp_next_param(rspnext);
        }
    }
    if (rspnext)
    {
        packetlen = strtol(rspnext, &rspnext, 10);
        rspnext = rsp_next_param(rspnext);
        rsp = AT_OK;
    }
    if (rspnext)
    {
        if (at_cipdinfo == at_enable)
        {
            helper_strcpy_param_unescapify(ipd_write->remote_ip, rspnext, rsp_get_param_length_max(rspnext, AT_MAX_IP));
            rspnext = rsp_next_param(rspnext);
        }
    }
    if (rspnext)
    {
        if (at_cipdinfo == at_enable)
        {
            ipd_write->remote_port = strtol(rspnext, &rspnext, 10);
            rspnext = rsp_next_param(rspnext);
        }
    }

    if (rsp == AT_OK)
    {
        rspcolon = strchr(rspparams, ':');
        if (rspcolon)
        {
        	//tfp_printf("ipd_write->length=%d\r\n", ipd_write->length);
            if (packetlen > ipd_write->length)
            {
                packetlen = ipd_write->length;
            }

            vTaskDelay (pdMS_TO_TICKS(50));
            // Read in packet data from the device.
            ipd_write->length = uartrb_read_wait(uart_at, ipd_write->buffer, packetlen);
            vTaskDelay (pdMS_TO_TICKS(50));
            //tfp_printf("ipd_write->length=%d\r\n", *((uint32_t*)ipd_write->buffer));

            if (xTimerIsTimerActive(at_timer) == pdFALSE)
            {
                rsp = AT_ERROR_TIMEOUT;
            }
            else
            {
                ipd_write->valid = at_ipd_status_data;
                vTaskDelay (pdMS_TO_TICKS(50));
                ipd_write = ipd_write->next;
            }
        }
        else
        {
            rsp = AT_ERROR_QUERY;
        }
    }

    xTimerStop(at_timer, 0);

    if (rsp == AT_ERROR_TIMEOUT)
    {
        return rsp;
    }
    return rsp;
}

inline int8_t at_ipd_info(int8_t *link_id, char *remote_ip, uint16_t *remote_port, uint16_t *length, uint8_t **buffer)
{
    if (length == 0)
        return AT_ERROR_PARAMETERS;
    if (at_cipmux == at_enable)
        if (link_id == 0)
            return AT_ERROR_PARAMETERS;

    xTimerChangePeriod(cmd_timer, at_rx_timeout_cmd, 0);
    do
    {
        // Look for data or a disconnect.
        peek_async_message();

        if (xTimerIsTimerActive(cmd_timer) == pdFALSE)
        {
            return AT_ERROR_TIMEOUT;
        }

    } while (ipd_head->valid != at_ipd_status_data);

    xTimerStop(cmd_timer, 0);

    if (ipd_head->valid == at_ipd_status_data)
    {
        struct ipd_store *ipd_next;

        if (length) *length = ipd_head->length;
        if (buffer) *buffer = ipd_head->buffer;
        if (remote_port) *remote_port = ipd_head->remote_port;
        if (remote_ip) strncpy(remote_ip, ipd_head->remote_ip, sizeof(ipd_head->remote_ip));
        if (link_id) *link_id = ipd_head->link_id;

        ipd_next = ipd_head->next;
        vPortFree(ipd_head);
        ipd_head = ipd_next;

        return AT_DATA_WAITING;
    }

    return AT_NO_DATA;
}

int8_t at_ipd(int8_t *link_id, uint16_t *length, uint8_t **buffer)
{
    return at_ipd_info(link_id, NULL, NULL, length, buffer);
}

uint16_t at_recv(int8_t link_id, uint16_t length, uint8_t *buffer)
{
    uint16_t offset = 0;
    uint16_t len= 0;

    xTimerChangePeriod(at_timer, pdMS_TO_TICKS(50), 0);
    do {
        len = uartrb_read(uart_at, buffer+offset, length-offset);
        offset += len;
        if (xTimerIsTimerActive(at_timer) == pdFALSE) {
            break;
        }
    }
    while (offset < length);
    xTimerStop(at_timer, 0);

    if (offset != length) {
        // TODO: investigate packet loss
        offset = length;
    }

    return offset;
}

int8_t at_is_wifi_connected()
{
    peek_async_message();
    return at_state_wifi_connected;
}

int8_t at_wifi_station_ip()
{
    peek_async_message();
    return at_state_wifi_station_has_ip;
}

enum at_connection at_is_server_connected()
{
    int8_t check;
    enum at_connection connect = at_not_connected;

    peek_async_message();
    for (check = AT_LINK_ID_MIN; check <= AT_LINK_ID_MAX; check++)
    {
        connect |= at_state_server_connect[check];
    }

    return connect;
}

enum at_connection at_is_link_id_connected(int8_t link_id)
{
    peek_async_message();
    return at_state_server_connect[link_id];
}
#endif
