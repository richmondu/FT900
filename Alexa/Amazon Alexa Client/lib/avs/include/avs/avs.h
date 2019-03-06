#ifndef AVS_H
#define AVS_H


int avsInit();
void avsFree();

int  avsConnect();
void avsDisconnect(int lSocket);

int  avsRecordAlexaRequest(const char* pcFileName, int (*fxnCallbackRecord)(void));
int  avsSendAlexaRequest(int lSocket, const char* pcFileName);
int  avsRecvAlexaResponse(int lSocket, const char* pcFileName);
int  avsPlayAlexaResponse(const char* fileName);

const ip_addr_t* avsGetServerAddress();
int  avsGetServerPort();


#endif // AVS_H
