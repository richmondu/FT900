# FT900 IoT Library

The IoT Library simplifies IoT development by abstracting MQTT protocol, together with secure authentication (TLS certificates or JWT/SAS security tokens), time management (SNTP and RTC) and IoT connectivity with the leading cloud platforms Amazon AWS, Google Cloud and Microsoft Azure.

### IoT API interface

    iot_connect - Establish secure IoT connectivity using TLS certificates and MQTT credentials
    iot_publish - Send/publish (sensor) data on a specified publish topic
    iot_subscribe - Register callback function for a specified subscription topic
    iot_disconnect - Disconnects IoT connectivity and cleans up resoures used   
    Refer to iot.h for the function definitions and documentation.

### IoT utilities

    iot_init, iot_free
    iot_utils_getcertificates - Sets the TLS certificates needed for the secure authentication with AWS, GCP and Azure.
    iot_utils_getcredentials - Sets the MQTT credentials needed for the cloud connectivity with AWS, GCP and Azure.
    iot_utils_gettimestampepoch - Get the current time since epoch in decimal format
    iot_utils_gettimestampiso - Get the current time since epoch in string format
    iot_utils_getdeviceid - Get the device id
    Refer to iot_utils.h for the function definitions and documentation.

### IoT configuration and certificates

    iot_config.h - Must be updated by user to specify "raw" MQTT credentials and TLS certificates. 
    Certificates folder - Must contain the TLS certificates needed for IoT connectivity.

### IoT sample usage

    net_init()
    iot_utils_init()
    while (1) {
        while (!net_is_ready()) {
            sleep()
        }
        iot_handle = iot_connect(iot_utils_getcertificates, iot_utils_getcredentials)
        iot_subscribe(iot_handle, topic_sub, subscribe_cb)
        while (1) {
            topic_pub = user_generate_publish_topic( iot_utils_getdeviceid() )
            topic_sub = user_generate_publish_payload( iot_utils_getdeviceid(), iot_utils_gettimestampepoch() )
            if (iot_publish(iot_handle, topic_pub, payload, payload_len) != 0) {
                break;
            }
        }
        iot_disconnect(iot_handle)
    }
    iot_utils_free()
