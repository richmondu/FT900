/*
 * ============================================================================
 * History
 * =======
 * 29 Oct 2018 : Created
 *
 * Copyright (C) Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 * ("Bridgetek") subject to the licence terms set out
 * http://brtchip.com/BRTSourceCodeLicenseAgreement/ ("the Licence Terms").
 * You must read the Licence Terms before downloading or using the Software.
 * By installing or using the Software you agree to the Licence Terms. If you
 * do not agree to the Licence Terms then do not download or use the Software.
 *
 * Without prejudice to the Licence Terms, here is a summary of some of the key
 * terms of the Licence Terms (and in the event of any conflict between this
 * summary and the Licence Terms then the text of the Licence Terms will
 * prevail).
 *
 * The Software is provided "as is".
 * There are no warranties (or similar) in relation to the quality of the
 * Software. You use it at your own risk.
 * The Software should not be used in, or for, any medical device, system or
 * appliance. There are exclusions of Bridgetek liability for certain types of loss
 * such as: special loss or damage; incidental loss or damage; indirect or
 * consequential loss or damage; loss of income; loss of business; loss of
 * profits; loss of revenue; loss of contracts; business interruption; loss of
 * the use of money or anticipated savings; loss of information; loss of
 * opportunity; loss of goodwill or reputation; and/or loss of, damage to or
 * corruption of data.
 * There is a monetary cap on Bridgetek's liability.
 * The Software may have subsequently been amended by another user and then
 * distributed by that other user ("Adapted Software").  If so that user may
 * have additional licence terms that apply to those amendments. However, Bridgetek
 * has no liability in relation to those amendments.
 * ============================================================================
 */

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
