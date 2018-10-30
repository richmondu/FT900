# FT900 IoT Library

The IoT Library simplifies IoT development by abstracting MQTT protocol, together with secure authentication (TLS certificates or JWT/SAS security tokens), time management (SNTP and RTC) and IoT connectivity with the leading cloud platforms Amazon AWS, Google Cloud and Microsoft Azure.

### IoT API interface

    iot_connect - Establish secure IoT connectivity using TLS certificates and MQTT credentials
    iot_publish - Send/publish (sensor) data on a specified publish topic
    iot_subscribe - Register callback function for a specified subscription topic
    iot_disconnect - Disconnects IoT connectivity and cleans up resoures used   
    Refer to iot.h for the function definitions and documentation.

### IoT Utilities

    iot_utils_getcertificates - Sets the TLS certificates needed for the secure authentication
    iot_utils_getcredentials - Sets the MQTT credentials needed for the cloud connectivity
    iot_utils_gettimestampepoch - Get the current time since epoch in decimal format
    iot_utils_gettimestampiso - Get the current time since epoch in string format
    Refer to iot_utils.h for the function definitions and documentation.
    
