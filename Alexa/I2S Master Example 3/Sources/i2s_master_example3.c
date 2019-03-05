/**
  @file main.c
  @brief
  FreeRTOS LWIP Example.

 */
/*
 * ============================================================================
 * History
 * =======
 * 2019-02-26 : Created v1
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
#include "ft900.h"
#include "tinyprintf.h"

/* FreeRTOS Headers. */
#include "FreeRTOS.h"
#include "task.h"

/* Audio Headers. */
#include "audio.h"



////////////////////////////////////////////////////////////////////////////////////////
// Raw audio file to use
////////////////////////////////////////////////////////////////////////////////////////

#define USE_SAMPLING_RATE SAMPLING_RATE_32KHZ

#if (USE_SAMPLING_RATE == SAMPLING_RATE_44100HZ)
extern __flash__ uint8_t raw_audio[]        asm("response_44100_raw");
extern __flash__ uint8_t raw_audio_end[]    asm("response_44100_raw_end");
#elif (USE_SAMPLING_RATE == SAMPLING_RATE_48KHZ)
extern __flash__ uint8_t raw_audio[]        asm("response_48k_raw");
extern __flash__ uint8_t raw_audio_end[]    asm("response_48k_raw_end");
#elif (USE_SAMPLING_RATE == SAMPLING_RATE_32KHZ)
extern __flash__ uint8_t raw_audio[]        asm("response_32k_raw");
extern __flash__ uint8_t raw_audio_end[]    asm("response_32k_raw_end");
#elif (USE_SAMPLING_RATE == SAMPLING_RATE_8KHZ)
extern __flash__ uint8_t raw_audio[]        asm("response_8k_raw");
extern __flash__ uint8_t raw_audio_end[]    asm("response_8k_raw_end");
#endif



////////////////////////////////////////////////////////////////////////////////////////
// Enable/disable logging
////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



////////////////////////////////////////////////////////////////////////////////////////
// Macro definitions
////////////////////////////////////////////////////////////////////////////////////////

#define TASK_STACK_SIZE          (500)           //Task Stack Size
#define TASK_PRIORITY            (1)             //Task Priority



////////////////////////////////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////////////////////////////////

int g_len = 0;
char* g_data = NULL;
int g_offset = 0;
int g_start = 0;


/** tfp_printf putc
 *  @param p Parameters
 *  @param c The character to write */
void myputc(void* p, char c) {
    uart_write((ft900_uart_regs_t*) p, (uint8_t) c);
}

void play_audio_task(void *pvParameters);

int main(void)
{
    sys_reset_all();
    interrupt_disable_globally();
    /* enable uart */
    sys_enable(sys_device_uart0);
    gpio_function(48, pad_func_3);
    gpio_function(49, pad_func_3);

    uart_open(UART0, 1, UART_DIVIDER_9600_BAUD, uart_data_bits_8, uart_parity_none, uart_stop_bits_1);
    /* Enable tfp_printf() functionality... */
    init_printf(UART0, myputc);

    /* Print out a welcome message... */
    uart_puts(UART0,
            "\x1B[2J" /* ANSI/VT100 - Clear the Screen */
            "\x1B[H" /* ANSI/VT100 - Move Cursor to Home */
            "Copyright (C) Bridgetek Pte Ltd \r\n"
            "--------------------------------------------------------------------- \r\n"
            "Welcome to I2S Master Example 3... \r\n"
            "\r\n"
            "Demonstrate playing a 16KHz 16-bit audio file in speaker\r\n"
            "--------------------------------------------------------------------- \r\n");

    if (xTaskCreate(play_audio_task, "audio_task", TASK_STACK_SIZE, NULL, TASK_PRIORITY, NULL) != pdTRUE) {
        DEBUG_PRINTF("Client failed\r\n");
    }

    DEBUG_PRINTF("Starting Scheduler.. \r\n");
    /* Start the scheduler so the created tasks start executing. */
    vTaskStartScheduler();

    DEBUG_PRINTF("Should never reach here!\r\n");

    for (;;)
        ;
}



////////////////////////////////////////////////////////////////////////////////////////
// Registered ISR
////////////////////////////////////////////////////////////////////////////////////////

void play_audio_isr(void)
{
    if (!g_start) {
        return;
    }

    if (g_offset < 0) {
        if (audio_speaker_ready()) {
            // Silence mode
            char temp[AUDIO_FIFO_SIZE] = {0};
            audio_play(temp, AUDIO_FIFO_SIZE);
            g_offset++;
            audio_speaker_clear();
        }
        return;
    }

    //DEBUG_PRINTF("play_audio %d\r\n", g_offset);

    /* If the FIFO is empty... */
    if (audio_speaker_ready())
    {
        // Compute size to transfer
        int size = AUDIO_FIFO_SIZE/2;
        if (g_len - g_offset < size) {
            size = g_len - g_offset;
        }

        // Copy audio data to buffer
        char temp[AUDIO_FIFO_SIZE/2];
        memcpy_pm2dat(temp, raw_audio + g_offset, size);

        // Duplicate short for stereo 2 channel
        for (int i=0, j=0; i<size; i+=2, j+=4) {
            g_data[j]   = temp[i];
            g_data[j+1] = temp[i+1];
            g_data[j+2] = temp[i];
            g_data[j+3] = temp[i+1];
        }

        // Play buffer to speaker
        audio_play(g_data, size*2);

        // Increment offset
        g_offset += size;

        audio_speaker_clear();
    }

    // Restart counter if last segment of file
    if (g_offset == g_len) {
        g_offset = -150;
    }
}



////////////////////////////////////////////////////////////////////////////////////////
// Main task
////////////////////////////////////////////////////////////////////////////////////////

void play_audio_task(void *pvParameters)
{
    // Setup audio
    audio_setup(&play_audio_isr, USE_SAMPLING_RATE);

    // Setup buffer and size
    g_len = (raw_audio_end - raw_audio) - 1;
    g_data = (char*)pvPortMalloc(AUDIO_FIFO_SIZE);
    if (!g_data) {
        DEBUG_PRINTF("vTask pvPortMalloc failed!\r\n");
        return;
    }
    DEBUG_PRINTF("Size of file %d bytes\r\n", g_len);
    g_start = 1;

    // Do Nothing
    for (;;)
        ;
}

