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
 *  @returns Returns 0 if success, negative value err_t otherwise
 */
int iot_publish  ( iot_handle handle, const char* topic, const char* payload, int payload_len );

/** @brief Register callback function for a specified subscription topic
 *  @param handle Handle returned by the call to iot_connect()
 *  @param topic Topic of data to subscribe from
 *  @param subscribe_cb Callback function to be called when there is data on the specified topic
 *  @returns Returns 0 if success, negative value err_t otherwise
 */
int iot_subscribe( iot_handle handle, const char* topic, iot_subscribe_cb subscribe_cb );

///////////////////////////////////////////////////////////////////////////////////



#endif /* _IOT_H_ */
