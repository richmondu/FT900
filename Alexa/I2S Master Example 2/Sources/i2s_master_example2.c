/**
  @file i2s_master_example2.c
  @brief
  I2S Master Example 2

  Play the microphone input from the Wolfson WM8731 codec to the output

 */
/*
 * ============================================================================
 * History
 * =======
 * 2017-10-09 : Created
 *
 * (C) Copyright Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 *  ("Bridgetek ") subject to the licence terms set out
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
#include <ft900.h>
#include "tinyprintf.h"
#include "audio.h"



#define USE_SAMPLING_RATE SAMPLING_RATE_16KHZ



#if defined(__FT900__)
#define GPIO_UART0_TX 	48
#define GPIO_UART0_RX 	49
#elif defined(__FT930__)
#define GPIO_UART0_TX 	23
#define GPIO_UART0_RX 	22
#endif


static char g_buffer[AUDIO_FIFO_SIZE];

void setup(void);
void loop(void);
void myputc(void* p, char c);
void i2s_ISR(void);

int main(void)
{
    setup();
    for(;;) loop();
    return 0;
}

void setup()
{
    /* Enable the UART Device... */
    sys_enable(sys_device_uart0);
    /* Set UART0 GPIO functions to UART0_TXD and UART0_RXD... */
    gpio_function(GPIO_UART0_TX, pad_uart0_txd); /* UART0 TXD */
    gpio_function(GPIO_UART0_RX, pad_uart0_rxd); /* UART0 RXD */
    uart_open(UART0,                    /* Device */
              1,                        /* Prescaler = 1 */
              UART_DIVIDER_9600_BAUD,  /* Divider = 1302 */
              uart_data_bits_8,         /* No. Data Bits */
              uart_parity_none,         /* Parity */
              uart_stop_bits_1);        /* No. Stop Bits */
    /* Initialise printf functionality */
    init_printf(UART0, myputc);

    /* Print out a welcome message... */
    uart_puts(UART0,
        "\x1B[2J" /* ANSI/VT100 - Clear the Screen */
        "\x1B[H"  /* ANSI/VT100 - Move Cursor to Home */
        "Copyright (C) Bridgetek Pte Ltd \r\n"
        "--------------------------------------------------------------------- \r\n"
        "Welcome to I2S Example 2... \r\n"
        "\r\n"
        "Play the microphone input from the Wolfson Microelectronics WM8731\r\n"
        "codec to the output. \r\n"
        "--------------------------------------------------------------------- \r\n"
        );

    audio_setup(i2s_ISR, USE_SAMPLING_RATE);
}

void loop()
{
    /* Do Nothing */
}

/* ISR which will stream data to the I2S slave device when it is needed */
void i2s_ISR(void)
{
    /* If the FIFO is empty... */
    if (audio_speaker_ready())
    {
        audio_record((uint8_t*)g_buffer, AUDIO_FIFO_SIZE);
        audio_play((uint8_t*)g_buffer, AUDIO_FIFO_SIZE);

        audio_speaker_clear();
    }
}

/* putc function for printf */
void myputc(void* p, char c)
{
    uart_write(p,c);
}
