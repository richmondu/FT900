# FT900 IoT Cloud demo


This demo is an improvement of the FT900 AWS IoT demo. It demonstrates:

    1. Secure connectivity with IoT cloud providers: Amazon AWS, Google Cloud and Microsoft Azure
       The choice of cloud provider is configurable with a macro USE_MQTT_BROKER
    2. Use of LWIP's MQTT library with ALTCP_TLS instead of MQTT library from Amazon FreeRTOS
       Also refer to the MQTT and TLS related bug fixes contributed to LWIP open-source community 
    3. Improvement of net.c Ethernet abstraction layer for simplification of user application code
       This abstracts the user from Ethernet initialization. Now, user simply calls net_init() and poll for net_is_ready().
    4. MQTT subscription in addition to MQTT publish
    5. Generation of security tokens created with current time retrieved from SNTP server
       GCP and Azure requires authentication using security tokens instead of certificates
    6. Improved handling of network unplugging and replugging for stability and reliability
    7. Adding of timestamps in the MQTT-published packets by using RTC library 
       The RTC is initialized with time queried from SNTP.
    It also contains 3 set of device certificates that can be used to connect to all 3 cloud services


### Amazon AWS IoT Core
    1. Endpoint: NAME.iot.REGION.amazonaws.com
    2. ClientId: DEVICE_ID
    3. Username: NONE
    4. Password: NONE
    5. CA: OPTIONAL
    6. Certificate: REQUIRED
    7. PrivateKey: REQUIRED
    8. PublishTopic: ANY
    9. SubscribeTopic: ANY
    * CA, device certificate and private key must be registered in Amazon AWS IoT Core

### Google Cloud IoT Core
    1. Endpoint: mqtt.googleapis.com
    2. ClientId: projects/PROJECT_ID/locations/LOCATION_ID/registries/REGISTRY_ID/devices/DEVICE_ID
    3. Username: ANY
    4. Password: JSON Web Token (JWT) security token (contains signature created with asymmetric device private key)
    5. CA: OPTIONAL
    6. Certificate: OPTIONAL
    7. PrivateKey: OPTIONAL
    8. PublishTopic: /devices/DEVICE_ID/events
    9. SubscribeTopic: NOT SUPPORTED
    * Device certificate must be registered in Google Cloud IoT Core

### Microsoft Azure IoT Hub
    1. Endpoint: HUB_NAME.azure-devices.net
    2. ClientId: DEVICE_ID
    3. Username: HUB_NAME/DEVICE_ID
    4. Password: Shared Access Signature (SAS) security token (contains signature created with symmetric shared access key)
    5. CA: OPTIONAL
    6. Certificate: OPTIONAL
    7. PrivateKey: OPTIONAL
    8. PublishTopic: /devices/DEVICE_ID/messages/events
    9. SubscribeTopic: NOT YET TESTED
    * CA, device certificate and private key are optional.
  

