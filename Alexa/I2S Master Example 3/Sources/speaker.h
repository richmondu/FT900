#ifndef SPEAKER_H
#define SPEAKER_H


#define SAMPLING_RATE_44100HZ 1
#define SAMPLING_RATE_48KHZ   2
#define SAMPLING_RATE_32KHZ   3
#define SAMPLING_RATE_8KHZ    4

void speaker_setup(void (*speaker_isr)(void), int samplingRate);
void speaker_play(char* data, int size);
void speaker_begin();
void speaker_end();
int  speaker_ready();


#endif // SPEAKER_H
