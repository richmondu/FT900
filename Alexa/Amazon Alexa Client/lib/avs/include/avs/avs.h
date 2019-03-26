#ifndef AVS_H
#define AVS_H


int  avs_init(void);
void avs_free(void);

int  avs_connect(void);
void avs_disconnect(void);

// - Read 2KB from microphone
// - Convert 2KB stereo to 1KB mono
// - Save 1KB to SD card
int  avs_record_request(const char* pcFileName, int (*fxnCallbackRecord)(void));

// - Read 4KB from SD card
// - Convert 4KB 16-bit to 2KB 8-bit
// - Send 2KB to RPI
int  avs_send_request  (const char* pcFileName);

// - Recv 2KB from RPI
// - Convert 8-bit to 16-bit
// - Save 4KB to SD card
int  avs_recv_response (const char* pcFileName);

// - Read 4KB from SD card
// - For each 1KB, convert from mono (1KB) to stereo (2KB)
// - Play 2KB on speaker
int  avs_play_response (const char* pcFileName);

// - Recv 512 bytes from RPI
// - Convert 8-bit mono (512) to 16-bit stereo (2KB)
// - Play 2KB on speaker
int  avs_recv_and_play_response(void);

// - Recv 2KB from RPI
// - Convert 8-bit to 16-bit (1KB)
// - Save 4KB to SD card
// - thread
//   Read 4KB from SD card
//   For each 1KB, convert from mono (1KB) to stereo (2KB)
//   Play 2KB on speaker
int  avs_recv_and_play_response_threaded(const char* pcFileName);

const ip_addr_t* avs_get_server_addr(void);
int  avs_get_server_port(void);
int  avs_err(void);


#endif // AVS_H
