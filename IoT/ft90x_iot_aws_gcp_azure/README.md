# FT900 IoT Cloud (AWS/GCP/Azure) demo


This project is an improvement of my FT900 AWS IoT demo located at https://github.com/richmondu/FT900/tree/master/IoT/aws_demos_ft_greengrass_lwip_mbedtls . 
This new project demonstrates the FT900 IoT framework including secure IoT cloud connectivity with AWS, GCP and Azure.

# IoT Framework
    1. Secure connectivity with IoT cloud providers using mbedTLS: Amazon Web Services (AWS), Google Cloud Platform (GCP) and Microsoft Azure
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
    8. Use of FT900 IoT Library, composed of API interface and utilities.
       https://github.com/richmondu/FT900/tree/master/IoT/ft90x_iot_aws_gcp_azure/lib/iot
    9. Customized Eclipse C/C++ IDE (Java plugin) to provide IoT Project Templates (AWS IoT, GCP IoT and Azure IoT) 
       to make it easy to develop cloud-connected IoT devices so developers can focus on ML/DL based data analytics.
       IoT Configurations Views are also provided so users can modify the MQTT credentials via UI instead of via code.
    It also contains 3 set of device certificates that can be used to connect to all 3 cloud services

# IoT Configurations
Below are the MQTT settings and TLS credentials needed to connect to IoT cloud services of AWS, GCP and Azure. 

### Amazon AWS IoT Core
    1. MQTT Endpoint: NAME-ats.iot.REGION.amazonaws.com (Default is now an ATS-endpoint)
    2. MQTT ClientId: DEVICE_ID (or THING_NAME if registered with a THING)
    3. MQTT Username: NONE
    4. MQTT Password: NONE
    5. TLS CA: REQUIRED (ATS server certificate by default since default endpoint is an ATS-endpoint)
       ATS server certificate https://www.amazontrust.com/repository/AmazonRootCA1.pem 
       Verisign server certificate https://www.symantec.com/content/en/us/enterprise/verisign/roots/VeriSign-Class%203-Public-Primary-Certification-Authority-G5.pem)
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
       5. TLS CA: https://github.com/Azure/azure-iot-sdk-c/blob/release_2018_06_08/certs/ms.der
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
       5. TLS CA: https://github.com/Azure/azure-iot-sdk-c/blob/release_2018_06_08/certs/ms.der
       6. TLS Certificate: REQUIRED
       7. TLS PrivateKey: REQUIRED
       8. MQTT PublishTopic: devices/DEVICE_ID/messages/events/
       9. MQTT SubscribeTopic: devices/DEVICE_ID/messages/devicebound/#
       * CLOUD: set the "Thumbprint" of device certificate (double click certificate->Details Tab->Thumbprint)
       * DEVICE: send ms.der, device certificate and private key for TLS connection

### Adafruit.IO
    1. MQTT Endpoint: io.adafruit.com
    2. MQTT ClientId: ANY
    3. MQTT Username: USERNAME
    4. MQTT Password: AIO KEY
    5. MQTT PublishTopic: USERNAME/feed/topic
    6. MQTT SubscribeTopic: USERNAME/feed/#
  
             
### Notes:
       1. Use MQTT.FX to troubleshoot and test validity of MQTT settings and TLS certificates.   
       2. mbedTLS configurables MBEDTLS_SSL_MAX_CONTENT_LEN and MBEDTLS_MPI_MAX_SIZE have to be increased to 3.5KB and 512 respectively, to support Azure IoT connectivity.
       3. The "MQTT publish topic" for Google Cloud IoT and Microsoft Azure IoT have fixed format. See above. Only AWS supports any topic.


# IoT Cloud Architecture

Below are the IoT cloud solutions architecture used for this demo application.

### Amazon AWS Architecture
       1. BACKEND: ft900 -> Greengrass -> IoT Core -> Lambda -> DynamoDB 
       2. FRONTEND: browser -> (Dashboard webpage) S3 -> API Gateway -> Lambda -> DynamoDB

### Google Cloud Architecture
       0. Refer to FT900IoTDemo_SetupGuide_GoogleCloud.docx for step-by-step Google Cloud IoT setup procedures
       1. BACKEND: ft900 -> IoT Core -> Pub/Sub -> Dataflow -> BigQuery
       2. FRONTEND: bigqueryclient.js -> BigQuery
          FRONTEND: browser -> (Dashboard webpage using bigqueryclient.js) Storage -> BigQuery [TODO]
          bigqueryclient.js demonstrates authenticating and querying of Big Query database
          
### Microsoft Azure Architecture
       0. Refer to FT900IoTDemo_SetupGuide_MicrosoftAzure.docx for step-by-step Microsoft Azure IoT setup procedures
       1. BACKEND: ft900 -> IoT Hub -> Stream Analytics -> CosmosDB
       2. FRONTEND: cosmosdbclient.js -> CosmosDB
          FRONTEND: browser -> (Dashboard webpage using cosmosdbclient.js) Storage -> CosmosDB [TODO]
          cosmosdbclient.js demonstrates authenticating and querying of CosmosDB database
       

# IoT Protocols

### MQTT over TLS

### HTTP over TLS



# IoT Authentication

### X.509 Certificate authentication

### JWT authentication

### SAS authentication

### SigV4 authentication

### OAuth authentication


# MQTT Messaging Systems and MQTT Clients

The following tools can be used to test the client and the cloud setup.
        
        1. MQTT.FX - an MQTT client that can be used to test the cloud setup, credentials and certificates.
        2. Eclipse Mosquitto - a simple MQTT message broker that can be used to test the FT900 client.
        3. Pivotal RabbitMQ - an enterprise message broker that supports MQTT and can be used to test the FT900 client.
        
TODO: Experiment with Apache Kafka as replacement for Mosquitto and RabbitMQ.

### RabbitMQ Setup

        // Installation
        1. Install Erlang http://www.erlang.org/downloads]
        2. Install RabbitMQ [https://www.rabbitmq.com/install-windows.html]
        3. Install RabbitMQ MQTT plugin [https://www.rabbitmq.com/mqtt.html]
           >> Open RabbitMQ Command Prompt
           >> rabbitmq-plugins enable rabbitmq_mqtt

        // Configuration
        4. Add environment variable RABBITMQ_CONFIG_FILE %APPDATA%\RabbitMQ\rabbitmq.config
        5. Create configuration file %APPDATA%\RabbitMQ\rabbitmq.config based on rabbitmq.config.example
        6. Update configuration file to enable the following
           {loopback_users, []},
           {ssl_options, [{cacertfile,           "rootca.pem"},
                          {certfile,             "ft900device1_cert.pem"},
                          {keyfile,              "ft900device1_pkey.pem"},
                          {verify,               verify_peer},
                          {fail_if_no_peer_cert, false},
                          {ciphers,  ["RSA-AES128-SHA", "RSA-AES256-SHA"]} ]}
           {default_user, <<"guest">>},
           {default_pass, <<"guest">>},
           {allow_anonymous, false},
           {tcp_listeners, [1883]},
           {ssl_listeners, [8883]}
        7. Restart RabbitMQ
           >> Open RabbitMQ Command Prompt
           >> rabbitmq-service.bat stop 
           >> rabbitmq-service.bat remove
           >> rabbitmq-service.bat install
           >> rabbitmq-service.bat start
        8. Copy certificates to %APPDATA%\RabbitMQ 
           rootca.pem, ft900device1_cert.pem, ft900device1_pkey.pem

        // Testing
        9. Install MQTT.FX [https://mqttfx.jensd.de/] - MQTT Client
        10. Create MQTT.FX Connection Profile
            Profile Name: My MQTT Client
            Profile Type: MQTT Broker
            Broker Address: <ip address of PC running RabbitMQ>
            Broker Port: 8883
            Client ID: ft900device1
            User Credentials User Name: guest
            User Credentials Password: guest
            SSL/TLS Enable SSL/TLS: Check
            SSL/TLS Protocol: TLSv1.2
            SSL/TLS Self-signed certificates:
              CA File: <full path to rootca.pem>
              Client Certificate File: <full path to ft900device1_cert.pem>
              Client Key File: <full path to ft900device1_pkey.pem>
        11. Select MQTT.FX Connection Profile 'My MQTT Client' and click Connect
        12. Go to MQTT.FX Subcribe tab and set topic to 'topic/subtopic' and click Subscribe button
        13. Go to MQTT.FX Publish tab and set topic to 'topic/subtopic' and 'hello world' message and click Publish button
        14. Go to MQTT.FX Subcribe tab and verify 'hello world' message appears

        // Testing with FT900
        15. Download https://github.com/richmondu/FT900/tree/master/IoT/ft90x_iot_aws_gcp_azure
        16. Update iot_config.h.
            // Set MQTT broker to use local MQTT settings
            #define USE_MQTT_BROKER MQTT_BROKER_LOCAL
            // Update IP address of PC running RabbitMQ
            #define MQTT_BROKER "192.168.100.5"
        17. Compile and run.


# FT900 Eclipse Toolchain

       FT900 Eclipse Toolchain now supports IoT Project templates for AWS, GCP and Azure. 
       This enables developers to easily create cloud-connected FT900 IoT applications.
       IoT Configuration views are also provided so users can set the MQTT credentials using Eclipse view instead of code.
