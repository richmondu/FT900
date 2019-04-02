#ifndef AUDIO_H
#define AUDIO_H
#include <ft900.h>


// Sampling rates
#define SAMPLING_RATE_44100HZ   1
#define SAMPLING_RATE_48KHZ     2
#define SAMPLING_RATE_32KHZ     3
#define SAMPLING_RATE_16KHZ     4
#define SAMPLING_RATE_8KHZ      5

// Fifo size of microphone and speaker
#define AUDIO_FIFO_SIZE         (2048)


// Speaker and Microphone setup
void audio_setup(void (*audio_isr)(void), int sampling_rate);

// Speaker APIs
#define audio_speaker_begin     i2s_start_tx
#define audio_speaker_end       i2s_stop_tx
#define audio_play              i2s_write
#define audio_speaker_clear()   { i2s_clear_int_flag(MASK_I2S_PEND_FIFO_TX_EMPTY); }
#define audio_speaker_ready()   (i2s_get_status() & MASK_I2S_PEND_FIFO_TX_EMPTY)

// Microphone APIs
#define audio_mic_begin         i2s_start_rx
#define audio_mic_end           i2s_stop_rx
#define audio_record            i2s_read
#define audio_mic_clear()       { i2s_clear_int_flag(MASK_I2S_PEND_FIFO_RX_FULL | MASK_I2S_PEND_FIFO_RX_OVER); }
#define audio_mic_ready()       (i2s_get_status() & (MASK_I2S_PEND_FIFO_RX_FULL | MASK_I2S_PEND_FIFO_RX_OVER))


// Audio ulaw 8-bit compression
void audio_pcm16_to_ulaw(int lSrcLen, const char *pcSrc, char *pcDst);
void audio_ulaw_to_pcm16(int lSrcLen, const char *pcSrc, char *pcDst);
void audio_ulaw_to_pcm16_stereo(int lSrcLen, const char *pcSrc, char *pcDst);


// Audio mono/stereo conversion
void audio_mono_to_stereo(char* pDst, char* pSrc, uint32_t ulSize);
void audio_stereo_to_mono(char* pDst, char* pSrc, uint32_t ulSize);



#endif // AUDIO_H
