#ifndef ULAW_H
#define ULAW_H


void pcm16_to_ulaw(int src_length, const char *src_samples, char *dst_samples);

void ulaw_to_pcm16(int src_length, const char *src_samples, char *dst_samples);
void ulaw_to_pcm16_stereo(int src_length, const char *src_samples, char *dst_samples);


#endif // ULAW_H

