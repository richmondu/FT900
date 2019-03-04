#ifndef ULAW_H
#define ULAW_H


void pcm16_to_ulaw(int src_length, const char *src_samples, char *dst_samples);
unsigned char linear2ulaw(short pcm_val);

void ulaw_to_pcm16(int src_length, const char *src_samples, char *dst_samples);
short ulaw2linear(unsigned char	u_val);


#endif // ULAW_H

