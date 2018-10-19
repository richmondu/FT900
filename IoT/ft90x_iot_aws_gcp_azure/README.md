# FT900 IoT Cloud demo


This project is an improvement of my FT900 AWS IoT demo located at https://github.com/richmondu/FT900/tree/master/IoT/aws_demos_ft_greengrass_lwip_mbedtls . 
This new project demonstrates:

    1. Secure connectivity with IoT cloud providers: Amazon Web Services (AWS), Google Cloud Platform (GCP) and Microsoft Azure
       The choice of cloud provider is configurable with a macro USE_MQTT_BROKER.
       User needs to update iot_config.h to configure settings corresponding to their account of their chosen cloud provider.
    2. Use of LWIP's MQTT library with ALTCP_TLS instead of MQTT library from Amazon FreeRTOS
       Also refer to the MQTT and TLS related bug fixes contributed to LWIP open-source community 
    3. Improvement of net.c Ethernet abstraction layer for simplification of user application code
       This abstracts the user from Ethernet initialization. Now, user simply calls net_init() and poll for net_is_ready().
    4. MQTT subscription in addition to MQTT publish
    5. Generation of security tokens created with current time retrieved from SNTP server
       GCP and Azure requires authentication using security tokens instead of certificates.
       Using security token enables to save memory since CA and client certificate dont need to be saved in the device anymore.
    6. Improved handling of network unplugging and replugging for stability and reliability
    7. Adding of timestamps in the MQTT-published packets by using RTC library 
       The RTC is initialized with time queried from SNTP.
    It also contains 3 set of device certificates that can be used to connect to all 3 cloud services


Below are the MQTT settings and TLS credentials needed to connect to IoT cloud services of AWS, GCP and Azure. 

### Amazon AWS IoT Core
    1. MQTT Endpoint: NAME.iot.REGION.amazonaws.com
    2. MQTT ClientId: DEVICE_ID (or THING_NAME if registered with a THING)
    3. MQTT Username: NONE
    4. MQTT Password: NONE
    5. TLS CA: REQUIRED (required for production, optional for testing)
    6. TLS Certificate: REQUIRED
    7. TLS PrivateKey: REQUIRED
    8. MQTT PublishTopic: ANY
    9. MQTT SubscribeTopic: ANY
    * CLOUD: CA, device certificate and private key must be registered in Amazon AWS IoT Core
      AWS IoT actually provides certificate generation.
    * DEVICE: Sends CA, device certificate, device private key for TLS connection

### Amazon AWS Greengrass
    1. MQTT Endpoint: local Greengrass device hostname or IP address
    2. MQTT ClientId: DEVICE_ID (or THING_NAME if registered with a THING)
    3. MQTT Username: NONE
    4. MQTT Password: NONE
    5. TLS CA: REQUIRED (Greengrass Group CA; dynamically generated, expires 7 days default, configurable 30 days max)
    6. TLS Certificate: REQUIRED
    7. TLS PrivateKey: REQUIRED
    8. MQTT PublishTopic: ANY
    9. MQTT SubscribeTopic: ANY
    * CLOUD: CA, device certificate and private key must be registered in Amazon AWS IoT Core
      AWS IoT actually provides certificate generation.
    * DEVICE: Sends GreengrassGroupCA, device certificate, device private key for TLS connection
    
### Google Cloud Platform IoT Core
    1. MQTT Endpoint: mqtt.googleapis.com
    2. MQTT ClientId: projects/PROJECT_ID/locations/LOCATION_ID/registries/REGISTRY_ID/devices/DEVICE_ID
    3. MQTT Username: ANY
    4. MQTT Password: JSON Web Token (JWT) security token (contains signature created with asymmetric device private key)
    5. TLS CA: NONE
    6. TLS Certificate: REQUIRED (registered in cloud)
    7. TLS PrivateKey: REQUIRED (used to generate JWT Token)
    8. MQTT PublishTopic: /devices/DEVICE_ID/events
    9. MQTT SubscribeTopic: NOT SUPPORTED
    * CLOUD: Certificate must be registered in Google Cloud IoT Core
    * DEVICE: No certificate is sent for TLS connection
    
### Microsoft Azure IoT Hub
    A. Authentication with SAS Security Token
       1. MQTT Endpoint: HUB_NAME.azure-devices.net
       2. MQTT ClientId: DEVICE_ID
       3. MQTT Username: HUB_NAME.azure-devices.net/DEVICE_ID/api-version=2016-11-14
       4. MQTT Password: Shared Access Signature (SAS) security token (contains signature created with symmetric shared access key)
       5. TLS CA: https://github.com/Azure/azure-iot-sdk-c/blob/master/certs/ms.der
       6. TLS Certificate: NULL
       7. TLS PrivateKey: NULL
       8. MQTT PublishTopic: devices/DEVICE_ID/messages/events/
       9. MQTT SubscribeTopic: devices/DEVICE_ID/messages/devicebound/#
       * CLOUD: copy the shared access key for SAS TOKEN generation
       * DEVICE: send ms.der as CA for TLS connection
         
    B. Authentication with X.509 Self-Signed Certificates
       1. MQTT Endpoint: HUB_NAME.azure-devices.net
       2. MQTT ClientId: DEVICE_ID
       3. MQTT Username: HUB_NAME.azure-devices.net/DEVICE_ID/api-version=2016-11-14
       4. MQTT Password: NULL
       5. TLS CA: https://github.com/Azure/azure-iot-sdk-c/blob/master/certs/ms.der
       6. TLS Certificate: REQUIRED
       7. TLS PrivateKey: REQUIRED
       8. MQTT PublishTopic: devices/DEVICE_ID/messages/events/
       9. MQTT SubscribeTopic: devices/DEVICE_ID/messages/devicebound/#
       * CLOUD: set the "Thumbprint" of device certificate (double click certificate->Details Tab->Thumbprint)
       * DEVICE: send ms.der, device certificate and private key for TLS connection
              
### Notes:
       1. Use MQTT.FX to troubleshoot and test validity of MQTT settings and TLS certificates.   
       2. mbedTLS configurables MBEDTLS_SSL_MAX_CONTENT_LEN and MBEDTLS_MPI_MAX_SIZE have to be increased to 3.5KB and 512 respectively, to support Azure IoT connectivity. 


Below are the IoT cloud solutions architecture used for this demo application.

### Amazon AWS Architecture
       1. BACKEND: ft900 -> Greengrass -> IoT Core -> Lambda -> DynamoDB 
       2. FRONTEND: browser -> (Dashboard webpage) S3 -> API Gateway -> Lambda -> DynamoDB

### Google Cloud Architecture
       0. Refer to FT900IoTDemo_SetupGuide_GoogleCloud.docx
       1. BACKEND: ft900 -> IoT Core -> Pub/Sub -> Dataflow -> BigQuery
       2. FRONTEND: bigqueryclient.js -> BigQuery
          FRONTEND: browser -> (Dashboard webpage using bigqueryclient.js) Storage -> BigQuery [TODO]
          
### Microsoft Azure Architecture
       0. Refer to FT900IoTDemo_SetupGuide_MicrosoftAzure.docx
       1. BACKEND: ft900 -> IoT Hub -> Stream Analytics -> CosmosDB
       2. FRONTEND: cosmosdbclient.js -> CosmosDB
          FRONTEND: browser -> (Dashboard webpage using cosmosdbclient.js) Storage -> CosmosDB [TODO]
       

