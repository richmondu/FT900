#ifndef COMM_WRAPPER_H
#define COMM_WRAPPER_H


int  comm_get_server_port(void);
const void* comm_get_server_addr(void);
int  comm_err(void);
int  comm_errno(void);

int  comm_connect(void);
void comm_disconnect(void);
int  comm_isconnected(void);

void comm_setsockopt(int lTimeoutSecs, int lIsSend);

int  comm_send(void *pvBuffer, int lSize);
int  comm_recv(void *pvBuffer, int lSize);


#endif // COMM_WRAPPER_H
