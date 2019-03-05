#ifndef AUDIO_H
#define AUDIO_H


#define SAMPLING_RATE_44100HZ 1
#define SAMPLING_RATE_48KHZ   2
#define SAMPLING_RATE_32KHZ   3
#define SAMPLING_RATE_8KHZ    4

#define AUDIO_FIFO_SIZE       (2048) // I2SM TX FIFO can hold up to a max of 2048 bytes


void audio_setup(void (*speaker_isr)(void), int samplingRate);

void audio_speaker_begin();
void audio_speaker_end();
int  audio_speaker_ready();
void audio_speaker_clear();
void audio_play(char* data, int size);

void audio_mic_begin();
void audio_mic_end();
int  audio_mic_ready();


#endif // AUDIO_H
