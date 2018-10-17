#ifndef _IOT_H_
#define _IOT_H_

#include <ft900.h>
#include "lwip/sockets.h"
#include "lwip/ip4_addr.h"
#include "lwip/netdb.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"
#include "lwip/altcp_tls.h"
#include "lwip/apps/sntp.h"
#include "net.h"
#include <iot_config.h>



///////////////////////////////////////////////////////////////////////////////////

void iot_init();
void iot_free();
const char* iot_getbrokername();
uint16_t iot_getbrokerport();
const char* iot_getid();
const char* iot_getusername();
const char* iot_getpassword();
const char* iot_getdeviceid();

const uint8_t* iot_certificate_getca(size_t* len);
const uint8_t* iot_certificate_getcert(size_t* len);
const uint8_t* iot_certificate_getpkey(size_t* len);
const uint8_t* iot_sas_getkey(size_t* len);

#if USE_PAYLOAD_TIMESTAMP
int64_t iot_rtc_get_time_epoch();
const char* iot_rtc_get_time_iso(int format);
#endif // USE_PAYLOAD_TIMESTAMP

///////////////////////////////////////////////////////////////////////////////////



#endif /* _IOT_H_ */
