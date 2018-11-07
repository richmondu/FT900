# FT900 IoT library

The IoT library simplifies IoT development by abstracting MQTT protocol, together with secure authentication (TLS certificates and JWT/SAS security tokens), time management (SNTP and RTC) and IoT connectivity with the leading cloud platforms Amazon AWS, Google Cloud and Microsoft Azure.

### Dependencies

    The IoT library depends on the following 3rd-party libraries
    LWIP - for MQTT, SNTP and ALTCP_TLS
    mbedTLS - for authentication and secure communication, also used by ALTCP_TLS
    FreeRTOS
    tinyprintf

### IoT interface

    iot_connect - Establish secure IoT connectivity using TLS certificates and MQTT credentials
    iot_publish - Send/publish (sensor) data on a specified publish topic
    iot_subscribe - Register callback function for a specified subscription topic
    iot_disconnect - Disconnects IoT connectivity and cleans up resoures used   
    Refer to iot.h for the function definitions and documentation.

### IoT utilities

    iot_utils_init, iot_utils_free
    iot_utils_getcertificates - Sets the TLS certificates needed for the secure authentication with AWS, GCP and Azure.
    iot_utils_getcredentials - Sets the MQTT credentials needed for the cloud connectivity with AWS, GCP and Azure.
    iot_utils_gettimestampepoch - Get the current time since epoch in decimal format
    iot_utils_gettimestampiso - Get the current time since epoch in string format
    iot_utils_getdeviceid - Get the device id
    Refer to iot_utils.h for the function definitions and documentation.
    
    These functions are implemented in iot_utils.c.
    iot_utils.c can be used for generic connectivity with other cloud services or local MQTT brokers.
    iot_utils_aws.c, iot_utils_gcp.c and iot_utils_azure.c are provided for connectivity with AWS, GCP and Azure, respectively.
    

### IoT configuration and certificates

    iot_config.h 
    - Must be updated by user to enable/disable IoT settings
    - iot_config_aws.h, iot_config_gcp.h, iot_config_azure.h must also be updated by user 
      to specify "raw" MQTT credentials and TLS certificates. 
    - iot_utils.c generates/derives the actual MQTT credentials based on the information stored in iot_config.h
      iot_utils_aws.c, iot_utils_gcp.c and iot_utils_azure.c are provided for AWS, GCP and Azure, respectively.
    
    Certificates folder 
    - Must contain the TLS certificates needed for IoT connectivity. 
    - The certificate names must correspond to the names registered in iot_config.h
    
    MQTT credentials
    - Amazon AWS IoT
	  - MQTT_BROKER = “IDENTIFIER.iot.REGION.amazonaws.com”
	  - MQTT_BROKER_PORT = 8883
	  - MQTT_CLIENT_NAME = DEVICE_ID or THING_NAME
	  - MQTT_CLIENT_USER = NULL // not needed
	  - MQTT_CLIENT_PASS = NULL // not needed	
    - Amazon AWS Greengrass
	  - MQTT_BROKER = IP address or host name of local Greengrass device
	  - MQTT_BROKER_PORT = 8883
	  - MQTT_CLIENT_NAME = DEVICE_ID or THING_NAME
	  - MQTT_CLIENT_USER = NULL // not needed
	  - MQTT_CLIENT_PASS = NULL // not needed
    - Google Cloud IoT
	  - MQTT_BROKER = “mqtt.googleapis.com”
	  - MQTT_BROKER_PORT = 8883
	  - MQTT_CLIENT_NAME = “projects/PROJECT_ID/locations/LOCATION_ID/registries/REGISTRY_ID/devices/DEVICE_ID”
	  - MQTT_CLIENT_USER = “ “ // any
	  - MQTT_CLIENT_PASS = JWT security token (generated with private key)
    - Microsoft Azure IoT (SAS security token authentication)
	  - MQTT_BROKER = “HUB_NAME.azure-devices.net”
	  - MQTT_BROKER_PORT = 8883
	  - MQTT_CLIENT_NAME = DEVICE_ID
	  - MQTT_CLIENT_USER = “HUB_NAME.azure-devices.net/DEVICE_ID/api-version=2016-11-14”
	  - MQTT_CLIENT_PASS = SAS security token (generated with shared access key)
    - Microsoft Azure IoT (TLS certificate authentication)
	  - MQTT_BROKER = “HUB_NAME.azure-devices.net”
	  - MQTT_BROKER_PORT = 8883
	  - MQTT_CLIENT_NAME = DEVICE_ID
	  - MQTT_CLIENT_USER = “HUB_NAME.azure-devices.net/DEVICE_ID/api-version=2016-11-14”
	  - MQTT_CLIENT_PASS = NULL // not needed

    TLS certificates
    - Sample for Amazon AWS IoT
	  - Rootca.pem
	  - Ft900device1_cert.pem
	  - Ft900device1_pkey.pem
    - Sample for Amazon AWS Greengrass
	  - Rootca_gg.pem
	  - Ft900device1_cert.pem
	  - Ft900device1_pkey.pem
    - Sample for Google Cloud IoT
	  - Ft900device1_cert.pem // not used by device, registered in cloud only
	  - Ft900device1_pkey.pem // used to generate the JWT security token
    - Sample for Microsoft Azure IoT (using SAS security token authentication)
	  - Rootca_azure.pem
	  - Ft900device1_sas_azure.pem // used to generate SAS security token
    - Sample for Microsoft Azure IoT (using TLS certificate authentication)
	  - Rootca_azure.pem
	  - Ft900device1_cert.pem
	  - Ft900device1_pkey.pem

### IoT sample usage
 
    net_init() // initialize network
    iot_utils_init() // initialize iot utilities
    
    while (1) {
    
        // wait until network is ready
        while ( !net_is_ready() ) { sleep( 1_second ) }
	
	// securely connect to MQTT server using TLS certificates and MQTT credentials
        iot_handle = iot_connect( iot_utils_getcertificates, iot_utils_getcredentials )
	
	// subscribe to an MQTT topic to receive messages sent by other devices or by server
        topic_sub = user_generate_subscribe_topic( iot_utils_getdeviceid() )
        iot_subscribe( iot_handle, topic_sub, subscribe_cb )
	
	// publish to an MQTT topic to send messages containing sensor data for data analytics
        while (1) {
            topic_pub = user_generate_publish_topic( iot_utils_getdeviceid() )
            topic_sub = user_generate_publish_payload( iot_utils_getdeviceid(), iot_utils_gettimestampepoch() )
            if ( iot_publish( iot_handle, topic_pub, payload, payload_len ) != 0 ) {
                break;
            }
        }
	
        iot_disconnect( iot_handle )
    }
    
    iot_utils_free() // cleanup resources used by iot utilities
