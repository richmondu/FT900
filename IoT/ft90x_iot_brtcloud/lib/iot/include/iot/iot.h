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

#ifndef _IOT_H_
#define _IOT_H_

#include <iot_config.h>



///////////////////////////////////////////////////////////////////////////////////

/** @brief IoT Handle
 */
typedef void* iot_handle;

/** @brief TLS Certificates
 */
typedef struct _iot_certificates {
    const u8_t* ca;
    size_t ca_len;
    const u8_t* cert;
    size_t cert_len;
    const u8_t* pkey;
    size_t pkey_len;
} iot_certificates;

/** @brief MQTT Credentials
 */
typedef struct _iot_credentials {
    const char *server_host;
    u16_t server_port;
    const char *client_id;
    const char *client_user;
    const char *client_pass;
} iot_credentials;

/** @brief MQTT Subscription
 */
typedef struct _iot_subscribe_rcv {
    const char *topic;
    const char *payload;
    u16_t payload_len;
    u16_t payload_size;
    u16_t payload_off;
} iot_subscribe_rcv;

/** @brief Function definitions for callback functions used in the IoT APIs
 */
typedef int ( *iot_certificates_cb   )( iot_certificates* tls_certificates     );
typedef int ( *iot_credentials_cb    )( iot_credentials*  mqtt_credentials     );
typedef void ( *iot_subscribe_cb     )( iot_subscribe_rcv* mqtt_subscribe_recv );


/** @brief Establish secure IoT connectivity using TLS certificates and MQTT credentials
 *  @param certificates_cb Callback function for specifying the TLS certificates
 *  @param credentials_cb Callback function for specifying the MQTT credentials
 *  @returns Returns a handle to be used for succeeding IoT calls
 */
iot_handle iot_connect ( iot_certificates_cb certificates_cb, iot_credentials_cb credentials_cb );

/** @brief Disconnects IoT connectivity and cleans up resoures used
 *  @param handle Handle returned by the call to iot_connect()
 *  @returns Returns None
 */
void iot_disconnect( iot_handle handle );

/** @brief Send/publish data on a specified topic
 *  @param handle Handle returned by the call to iot_connect()
 *  @param topic Topic of data to publish to
 *  @param payload Data to send/publish
 *  @param payload_len Length of data to send/publish
 *  @param qos Quality of service (use 0 or 1)
 *  @returns Returns 0 if success, negative value err_t otherwise
 */
int iot_publish  ( iot_handle handle, const char* topic, const char* payload, int payload_len, int qos );

/** @brief Register callback function for a specified subscription topic
 *  @param handle Handle returned by the call to iot_connect()
 *  @param topic Topic of data to subscribe from
 *  @param subscribe_cb Callback function to be called when there is data on the specified topic
 *  @param qos Quality of service (use 0 or 1)
 *  @returns Returns 0 if success, negative value err_t otherwise
 */
int iot_subscribe( iot_handle handle, const char* topic, iot_subscribe_cb subscribe_cb, int qos );

/** @brief Unregister callback function for a specified subscription topic
 *  @param handle Handle returned by the call to iot_connect()
 *  @param topic Topic of data to unsubscribe from
 *  @returns Returns None
 */
void iot_unsubscribe( iot_handle handle, const char* topic );

/** @brief Check if iot is connected
 *  @param handle Handle returned by the call to iot_connect()
 *  @returns Returns 0 if connected, negative value err_t otherwise
 */
int iot_is_connected( iot_handle handle );

///////////////////////////////////////////////////////////////////////////////////



#endif /* _IOT_H_ */
