/**
  @file uartrb.c
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

#include <stdint.h>
#include <string.h>

#include "ft900_uart_simple.h"
#include <ft900.h>
#include "FreeRTOSConfig.h"
#include "uartrb.h"

/* Enable mode for the FT9xx internal FIFOs.
 * 16450 FIFO size is zero.
 * 16550 FIFO size is 16.
 * 16650, 16750 and 16950 FIFO size is 128.
 */
#define ENABLE_FIFO 16

/* Suggested size of ring buffer used for both receive and transmit.
 * Reading and writing to the UART FIFO is gernerally performed by
 * an interrupt service routine.
 */
#define RINGBUFFER_SIZE 1024

/* Threshold number of bytes in the receive buffer where flow control
 * is to be enacted and signals are de-asserted. When a FIFO is enabled
 * and automatic out-of-band flow control is disabled this must be
 * at-least as large as the FIFO buffer size enabled on the device.
 */
#define RINGBUFFER_THRESHOLD (ENABLE_FIFO + 4)

/* Hysteresis value to re-enable flow control by asserting signal. */
#define RINGBUFFER_THRESHOLD_HYST 4

/* End-of-line marker */
#define EOL_CR 1
#define EOL_LF 2
#define EOL_CRLF 3
#define END_OF_LINE CRLF

/* Structure used to store received data and ring buffer indexes. */
typedef struct
{
    uint8_t     buffer[RINGBUFFER_SIZE];
    volatile uint16_t wr_idx;
    volatile uint16_t rd_idx;
    volatile uint8_t wait;
} RingBuffer_t;

/* Receive buffer */
static RingBuffer_t uart0BufferRx = { {0}, 0, 0, 0 };
static RingBuffer_t uart1BufferRx = { {0}, 0, 0, 0 };
/* Transmit buffer */
static RingBuffer_t uart0BufferTx = { {0}, 0, 0, 0 };
static RingBuffer_t uart1BufferTx = { {0}, 0, 0, 0 };
/* Flow control settings */
static uartrb_flow_t uart0Flow = uartrb_flow_none;
static uartrb_flow_t uart1Flow = uartrb_flow_none;
/* Timeout flags */
static volatile int8_t uart0Timeout = 0;
static volatile int8_t uart1Timeout = 0;

/* Local functions. */
static void uartrb_starttx(ft900_uart_regs_t *dev, RingBuffer_t *uartBuffer);
static uint16_t uartrb_available_int(RingBuffer_t *uartBuffer);
static uint16_t uartrb_used_int(RingBuffer_t *uartBuffer);
static void uartrb_ISR(ft900_uart_regs_t *dev);
static void uartrb_0_ISR();
static void uartrb_1_ISR();

/**
 The Interrupt which handles asynchronous transmission and reception
 of data into the ring buffer
 */
static void uartrb_0_ISR()
{
    uartrb_ISR(UART0);
}

static void uartrb_1_ISR()
{
    uartrb_ISR(UART1);
}

/**
 The Interrupt which handles asynchronous transmission and reception
 of data into the ring buffer
 */
static void uartrb_ISR(ft900_uart_regs_t *dev)
{
    static uint8_t c;
    static int16_t avail;
    static uint8_t curint;

    static RingBuffer_t *uartBuffer;
    uartrb_flow_t flow = (dev == UART0)?uart0Flow:uart1Flow;

    curint = uart_get_interrupt(dev);

    /* Transmit interrupt... */
    if ((curint == uart_interrupt_tx)
            || (curint == uart_interrupt_dcd_ri_dsr_cts))
    {
        uartBuffer = (dev == UART0)?&uart0BufferTx:&uart1BufferTx;

        /* Check to see how much data we have to transmit... */
        avail = uartrb_used_int(uartBuffer);

        /* Out buffer read pointer is not the same as the write pointer...
         * If flow control is enabled then CTS or DSR must be asserted. */
        if (flow == uartrb_flow_rts_cts)
        {
            if (!uart_cts(dev))
            {
                avail = 0;
                uartBuffer->wait = 1;
            }
        }
        else if (flow == uartrb_flow_dtr_dsr)
        {
            if (!uart_dsr(dev))
            {
                avail = 0;
                uartBuffer->wait = 1;
            }
        }

        if (avail)
        {
            /* Copy out the byte to be transmitted so that the uart_write is
               the last thing we do... */
            c = uartBuffer->buffer[uartBuffer->rd_idx];

            /* Increment the pointer and wrap around */
            uartBuffer->rd_idx++;
            if (uartBuffer->rd_idx == RINGBUFFER_SIZE) uartBuffer->rd_idx = 0;

            /* Write out a new byte, the following Transmit interrupt should handle
               the remaning bytes... */
            uart_write(dev, c);
            uartBuffer->wait = 0;
        }
    }

    /* Receive interrupt... */
    if (curint == uart_interrupt_rx)
    {
        uartBuffer = (dev == UART0)?&uart0BufferRx:&uart1BufferRx;

        avail = uartrb_available_int(uartBuffer);

        /* Read a byte into the Ring Buffer... */
        uart_read(dev, &c);
        uartBuffer->buffer[uartBuffer->wr_idx] = c;

        /* Increment the pointer and wrap around */
        uartBuffer->wr_idx++;
        if (uartBuffer->wr_idx == RINGBUFFER_SIZE) uartBuffer->wr_idx = 0;

        /* Enact flow control for CTS/RTS or DSR/DTR */
        /* De-assert RTS or DTR - receive buffer full */
        if (avail <= RINGBUFFER_THRESHOLD)
        {
            if (flow == uartrb_flow_rts_cts)
            {
                uart_rts(dev, 0);
                uartBuffer->wait = 1;
            }
            else if (flow == uartrb_flow_dtr_dsr)
            {
                uart_dtr(dev, 0);
                uartBuffer->wait = 1;
            }
        }
    }
}

static void uartrb_starttx(ft900_uart_regs_t *dev, RingBuffer_t *uartBuffer)
{
    uint8_t c;
//    configPRINTF(("%s %d %d\n", __FUNCTION__, __LINE__, uartBuffer->wait));
    CRITICAL_SECTION_BEGIN
    if (!uartBuffer->wait)
    {
        if (uartBuffer->rd_idx != uartBuffer->wr_idx)
        {
            //uart_disable_interrupts_globally(dev);
            c= uartBuffer->buffer[uartBuffer->rd_idx];
            /* Increment the pointer and wrap around */
            uartBuffer->rd_idx++;
            if (uartBuffer->rd_idx == RINGBUFFER_SIZE) uartBuffer->rd_idx = 0;
            //uart_enable_interrupts_globally(dev);

            uart_write(dev, c);
        }
    }
    CRITICAL_SECTION_END
}

static uint16_t uartrb_available_int(RingBuffer_t *uartBuffer)
{
    int16_t diff;

    diff = uartBuffer->rd_idx - uartBuffer->wr_idx;

    if (diff <= 0)
    {
        diff += RINGBUFFER_SIZE;
    }

    return (uint16_t)diff;
}

static uint16_t uartrb_used_int(RingBuffer_t *uartBuffer)
{
    int16_t diff;

    diff = uartBuffer->wr_idx - uartBuffer->rd_idx;

    if (diff < 0)
    {
        diff += RINGBUFFER_SIZE;
    }

    return (uint16_t)diff;
}

static void uartrb_flow_int(ft900_uart_regs_t *dev, RingBuffer_t *uartBuffer)
{
    /* Assert RTS or DTR - receive buffer not full */
    if (uartBuffer->wait)
    {
        uartrb_flow_t flow = (dev == UART0)?uart0Flow:uart1Flow;
        uint16_t avail = uartrb_available_int(uartBuffer);

        if (avail > RINGBUFFER_THRESHOLD + RINGBUFFER_THRESHOLD_HYST)
        {
            if (flow == uartrb_flow_rts_cts)
            {
                uart_rts(dev, 1);
                uartBuffer->wait = 0;
            }
            else if (flow == uartrb_flow_dtr_dsr)
            {
                uart_dtr(dev, 1);
                uartBuffer->wait = 0;
            }
        }
    }
}

/* API functions */

void uartrb_setup(ft900_uart_regs_t *dev, uartrb_flow_t flow)
{
    /* Enable the UART to fire interrupts when receiving data... */
    if (uart_enable_interrupt(dev, uart_interrupt_rx) == -1)
    {
        // ERROR RX
    }
    /* Enable the UART to fire interrupts when transmitting data... */
    if (uart_enable_interrupt(dev, uart_interrupt_tx) == -1)
    {
        // ERROR TX
    }
    if (flow != uartrb_flow_none)
    {
        /* Enable the UART to fire interrupts when modem changes occur... */
        if (uart_enable_interrupt(dev, uart_interrupt_dcd_ri_dsr_cts) == -1)
        {
            // ERROR FLOW
        }
    }
    else if (flow == uartrb_flow_rts_cts_auto)
    {
        // ERROR NOT SUPPORTED YET
    }

    /* Attach the interrupt so it can be called... */
    if (dev == UART0)
    {
        interrupt_attach(interrupt_uart0, (uint8_t) interrupt_uart0, uartrb_0_ISR);
        uart0Flow = flow;
    }
    else
    {
        interrupt_attach(interrupt_uart1, (uint8_t) interrupt_uart1, uartrb_1_ISR);
        uart1Flow = flow;
    }

    /* Enable interrupts to be fired... */
    uart_enable_interrupts_globally(dev);

    /* Enable RTS to start receiving data. */
    if (flow == uartrb_flow_rts_cts)
    {
        uart_rts(dev, 1);
    }
    else if (flow == uartrb_flow_dtr_dsr)
    {
        uart_dtr(dev, 1);
    }

    while (uart_get_interrupt(dev) != uart_interrupt_none);
}

/**
 Transmit a character of data over UART1 asynchronously.

 @return The number of bytes written to the buffer
 */
uint16_t uartrb_putc(ft900_uart_regs_t *dev, uint8_t val)
{
    uint16_t free;
    uint16_t copied = 0;
    RingBuffer_t *uartBuffer = (dev == UART0)?&uart0BufferTx:&uart1BufferTx;

    /* Determine how much space we have ... */
    CRITICAL_SECTION_BEGIN
    free = uartrb_available_int(uartBuffer);

    /* Copy in as much data as we can... */
    if (free)
    {
        uartBuffer->buffer[uartBuffer->wr_idx] = val;
        /* Increment the pointer and wrap around */
        uartBuffer->wr_idx++;
        if (uartBuffer->wr_idx == RINGBUFFER_SIZE) uartBuffer->wr_idx = 0;

        copied = 1;
    }
    CRITICAL_SECTION_END

    /* Start a transmission if nothing is being transmitted... */
    uartrb_starttx(dev, uartBuffer);

    return copied;
}

/**
 Transmit a buffer of data over UART1 asynchronously.

 @return The number of bytes written to the buffer
 */
uint16_t uartrb_write(ft900_uart_regs_t *dev, uint8_t *buffer, uint16_t len)
{
//    configPRINTF(("%s %d %s\n", __FUNCTION__, __LINE__, buffer));
    uint16_t avail;
    uint16_t free;
    uint16_t copied = 0;
    RingBuffer_t *uartBuffer = (dev == UART0)?&uart0BufferTx:&uart1BufferTx;

    if (len == 0)
    {
        return 0;
    }
    /* Determine how much space we have ... */
    CRITICAL_SECTION_BEGIN
    free = avail = uartrb_available_int(uartBuffer);

    /* Copy in as much data as we can... */
    while (len-- && free--)
    {
        uartBuffer->buffer[uartBuffer->wr_idx] = *buffer++;
        /* Increment the pointer and wrap around */
        uartBuffer->wr_idx++;
        if (uartBuffer->wr_idx == RINGBUFFER_SIZE) uartBuffer->wr_idx = 0;

        copied++;
    }
    CRITICAL_SECTION_END

    /* Start a transmission if nothing is being transmitted... */
    uartrb_starttx(dev, uartBuffer);

    return copied;
}

uint16_t uartrb_write_wait(ft900_uart_regs_t *dev, uint8_t *buffer, uint16_t len)
{
    int written;
    int total = 0;
    volatile int8_t *pTimeout = (dev == UART0)?&uart0Timeout:&uart1Timeout;

    *pTimeout = 0;
    while (len)
    {
        written = uartrb_write(dev, buffer, len);
        len -= written;
        buffer += written;
        total += written;

        if (*pTimeout)
        {
            *pTimeout = 0;
            break;
        }
    }
    return total;
}

uint16_t uartrb_readln(ft900_uart_regs_t *dev, uint8_t *buffer, uint16_t len)
{
    uint16_t avail;
    uint16_t copied = 0;
    int cr = 0;
    RingBuffer_t *uartBuffer = (dev == UART0)?&uart0BufferRx:&uart1BufferRx;
    volatile int8_t *pTimeout = (dev == UART0)?&uart0Timeout:&uart1Timeout;

    *pTimeout = 0;
    /* Copy in as much data as we can ...
       This can be either the maximum size of the buffer being given
       or the maximum number of bytes available in the Serial Port
       buffer */
    while (len--)
    {
        /* Leave space for a NULL terminator for the line received. */
        if (len == 0)
        {
            break;
        }

        /* WAIT and BLOCK for new data to be available */
        do
        {
            CRITICAL_SECTION_BEGIN
            avail = uartrb_used_int(uartBuffer);
            CRITICAL_SECTION_END
        } while (avail == 0);

        CRITICAL_SECTION_BEGIN
        *buffer = uartBuffer->buffer[uartBuffer->rd_idx];
        /* Increment the pointer and wrap around */
        uartBuffer->rd_idx++;
        if (uartBuffer->rd_idx == RINGBUFFER_SIZE) uartBuffer->rd_idx = 0;
        uartrb_flow_int(dev, uartBuffer);
        CRITICAL_SECTION_END

        if (*buffer == '\r')
        {
            if (cr == 0)
            {
                cr = EOL_CR;
            }
        }
        else if (*buffer == '\n')
        {
            if (cr == EOL_CR)
            {
                cr = EOL_CRLF;
            }
            else
            {
                cr = 0;
            }
        }
        else
        {
            // Non EOL character received.
            buffer++;
            copied++;
            cr = 0;
        }

        if (cr == EOL_CRLF) break;

        if (*pTimeout)
        {
            *pTimeout = 0;
            break;
        }
    }

    /* Always NULL terminate the line received. */
    *buffer = '\0';
    /* Report back how many bytes have been copied into the buffer...*/
    return copied;
}

/**
 Receive a number of bytes from the UART ring buffer

 @return The number of bytes read
 */
uint16_t uartrb_read(ft900_uart_regs_t *dev, uint8_t *buffer, uint16_t len)
{
    uint16_t avail;
    uint16_t copied = 0;
    RingBuffer_t *uartBuffer = (dev == UART0)?&uart0BufferRx:&uart1BufferRx;

    CRITICAL_SECTION_BEGIN
    avail = uartrb_used_int(uartBuffer);
    /* Copy in as much data as we can ...
       This can be either the maximum size of the buffer being given
       or the maximum number of bytes available in the Serial Port
       buffer */
    while(len && avail)
    {
        len--;
        avail--;

        *buffer = uartBuffer->buffer[uartBuffer->rd_idx];
        /* Increment the pointer and wrap around */
        uartBuffer->rd_idx++;
        if (uartBuffer->rd_idx == RINGBUFFER_SIZE) uartBuffer->rd_idx = 0;

        buffer++;
        copied++;
    }

    uartrb_flow_int(dev, uartBuffer);
    CRITICAL_SECTION_END

    /* Report back how many bytes have been copied into the buffer...*/
    return copied;
}

uint16_t uartrb_read_wait(ft900_uart_regs_t *dev, uint8_t *buffer, uint16_t len)
{
    int read;
    int total = 0;
    volatile int8_t *pTimeout = (dev == UART0)?&uart0Timeout:&uart1Timeout;

    *pTimeout = 0;
    while (len)
    {
        read = uartrb_read(dev, buffer, len);
        len -= read;
        buffer += read;
        total += read;

        if (*pTimeout)
        {
            *pTimeout = 0;
            break;
        }
    }
    return total;
}

/**
 Receive one byte from the UART ring buffer

 @return The number of bytes read
 */
uint16_t uartrb_getc(ft900_uart_regs_t *dev, uint8_t *val)
{
    uint16_t avail;
    uint16_t copied = 0;
    RingBuffer_t *uartBuffer = (dev == UART0)?&uart0BufferRx:&uart1BufferRx;

    CRITICAL_SECTION_BEGIN
    avail = uartrb_used_int(uartBuffer);
    /* Copy in as much data as we can ...
       This can be either the maximum size of the buffer being given
       or the maximum number of bytes available in the Serial Port
       buffer */
    if (avail)
    {
        *val = uartBuffer->buffer[uartBuffer->rd_idx];
        /* Increment the pointer and wrap around */
        uartBuffer->rd_idx++;
        if (uartBuffer->rd_idx == RINGBUFFER_SIZE) uartBuffer->rd_idx = 0;

        copied = 1;
    }
    uartrb_flow_int(dev, uartBuffer);
    CRITICAL_SECTION_END

    /* Report back how many bytes have been copied into the buffer...*/
    return copied;
}

/**
 See how much data is available in the UART1 ring buffer

 @return The number of bytes available
 */
uint16_t uartrb_available(ft900_uart_regs_t *dev)
{
    uint16_t diff;
    RingBuffer_t *uartBuffer = (dev == UART0)?&uart0BufferRx:&uart1BufferRx;

    CRITICAL_SECTION_BEGIN
    diff = uartrb_available_int(uartBuffer);
    CRITICAL_SECTION_END

    return diff;
}

uint16_t uartrb_used(ft900_uart_regs_t *dev)
{
    uint16_t diff;
    RingBuffer_t *uartBuffer = (dev == UART0)?&uart0BufferRx:&uart1BufferRx;

    CRITICAL_SECTION_BEGIN
    diff = uartrb_used_int(uartBuffer);
    CRITICAL_SECTION_END

    return diff;
}

/**
 See how much data is available in the UART1 ring buffer

 @return The number of bytes available
 */
uint16_t uartrb_waiting(ft900_uart_regs_t *dev)
{
    uint16_t diff;
    RingBuffer_t *uartBuffer = (dev == UART0)?&uart0BufferTx:&uart1BufferTx;

    CRITICAL_SECTION_BEGIN
    diff = uartrb_used_int(uartBuffer);
    CRITICAL_SECTION_END

    return diff;
}

uint16_t uartrb_peek(ft900_uart_regs_t *dev, uint8_t *buffer, uint16_t len)
{
    uint16_t avail;
    uint16_t copied = 0;
    RingBuffer_t *uartBuffer = (dev == UART0)?&uart0BufferRx:&uart1BufferRx;
    uint16_t peek_idx = uartBuffer->rd_idx;

    CRITICAL_SECTION_BEGIN
    avail = uartrb_used_int(uartBuffer);
    /* Copy in as much data as we can ...
       This can be either the maximum size of the buffer being given
       or the maximum number of bytes available in the Serial Port
       buffer */
    while(len-- && avail--)
    {
        *buffer = uartBuffer->buffer[peek_idx];
        /* Increment the pointer and wrap around */
        peek_idx++;
        if (peek_idx == RINGBUFFER_SIZE) peek_idx = 0;

        buffer++;
        copied++;
    }
    CRITICAL_SECTION_END

    /* Report back how many bytes have been copied into the buffer...*/
    return copied;
}

/**
 See what byte is at the head of the ring buffer

 @params val - The first available byte
 @returns Number of valid bytes returned
 */
uint16_t uartrb_peekc(ft900_uart_regs_t *dev, uint8_t *val)
{
    uint16_t copied = 0;
    RingBuffer_t *uartBuffer = (dev == UART0)?&uart0BufferRx:&uart1BufferRx;

    CRITICAL_SECTION_BEGIN
    if (uartBuffer->wr_idx != uartBuffer->rd_idx)
    {
        *val = uartBuffer->buffer[uartBuffer->rd_idx];
        copied = 1;
    }
    CRITICAL_SECTION_END

    return copied;
}

void uartrb_timeout(ft900_uart_regs_t *dev)
{
    if (dev == UART0)
    {
        uart0Timeout = 1;
    }
    else
    {
        uart1Timeout = 1;
    }
}

void uartrb_flush_read(ft900_uart_regs_t *dev)
{
    RingBuffer_t *uartBuffer = (dev == UART0)?&uart0BufferRx:&uart1BufferRx;

    CRITICAL_SECTION_BEGIN
    uartBuffer->rd_idx = uartBuffer->wr_idx;
    uartrb_flow_int(dev, uartBuffer);
    CRITICAL_SECTION_END
}
/* end */
