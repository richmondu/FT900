#ifndef AUDIO_H
#define AUDIO_H


#define SAMPLING_RATE_44100HZ 1
#define SAMPLING_RATE_48KHZ   2
#define SAMPLING_RATE_32KHZ   3
#define SAMPLING_RATE_16KHZ   4
#define SAMPLING_RATE_8KHZ    5

#define AUDIO_FIFO_SIZE       (2048) // I2SM TX FIFO can hold up to a max of 2048 bytes


void audio_setup(void (*audio_isr)(void), int sampling_rate);

void audio_speaker_begin();
void audio_speaker_end();
int  audio_speaker_ready();
void audio_speaker_clear();
void audio_play(char* data, int size);

void audio_mic_begin();
void audio_mic_end();
int  audio_mic_ready();
void audio_mic_clear();
void audio_record(char* data, int size);


#endif // AUDIO_H
