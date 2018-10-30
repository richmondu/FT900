#ifndef _IOT_UTILS_H_
#define _IOT_UTILS_H_

#include <ft900.h>
#include "lwip/sockets.h"
#include "lwip/ip4_addr.h"
#include "lwip/netdb.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"
#include "lwip/altcp_tls.h"
#include "lwip/apps/sntp.h"
#include "net.h"
#include "iot.h"
#include "iot_config.h"



///////////////////////////////////////////////////////////////////////////////////

void iot_utils_init();
void iot_utils_free();

int iot_utils_getcertificates( iot_certificates* tls_certificates );
int iot_utils_getcredentials( iot_credentials* mqtt_credentials );
const char* iot_utils_getdeviceid();

#if USE_PAYLOAD_TIMESTAMP
int64_t iot_utils_gettimeepoch();
const char* iot_utils_gettimeiso( int format );
#endif // USE_PAYLOAD_TIMESTAMP

///////////////////////////////////////////////////////////////////////////////////



#endif /* _IOT_UTILS_H_ */
