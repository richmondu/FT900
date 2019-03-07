#ifndef AVS_H
#define AVS_H


int  avs_init();
void avs_free();

int  avs_connect();
void avs_disconnect(int lSocket);

int  avs_record_request(const char* pcFileName, int (*fxnCallbackRecord)(void));
int  avs_send_request(int lSocket, const char* pcFileName);
int  avs_recv_response(int lSocket, const char* pcFileName);
int  avs_play_response(const char* fileName);

const ip_addr_t* avs_get_server_addr();
int  avs_get_server_port();


#endif // AVS_H
