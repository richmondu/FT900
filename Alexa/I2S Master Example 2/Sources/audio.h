#ifndef AUDIO_H
#define AUDIO_H
#include <ft900.h>


#define SAMPLING_RATE_44100HZ 1
#define SAMPLING_RATE_48KHZ   2
#define SAMPLING_RATE_32KHZ   3
#define SAMPLING_RATE_16KHZ   4
#define SAMPLING_RATE_8KHZ    5

#define AUDIO_FIFO_SIZE       (2048) // I2SM TX FIFO can hold up to a max of 2048 bytes


void audio_setup(void (*audio_isr)(void), int sampling_rate);

#define audio_speaker_begin     i2s_start_tx
#define audio_speaker_end       i2s_stop_tx
#define audio_play              i2s_write
#define audio_speaker_clear()   { i2s_clear_int_flag(MASK_I2S_PEND_FIFO_TX_EMPTY); }
#define audio_speaker_ready()   (i2s_get_status() & MASK_I2S_PEND_FIFO_TX_EMPTY)

#define audio_mic_begin         i2s_start_rx
#define audio_mic_end           i2s_stop_rx
#define audio_record            i2s_read
#define audio_mic_clear()       { i2s_clear_int_flag(MASK_I2S_PEND_FIFO_RX_FULL | MASK_I2S_PEND_FIFO_RX_OVER); }
#define audio_mic_ready()       (i2s_get_status() & (MASK_I2S_PEND_FIFO_RX_FULL | MASK_I2S_PEND_FIFO_RX_OVER))


#endif // AUDIO_H
