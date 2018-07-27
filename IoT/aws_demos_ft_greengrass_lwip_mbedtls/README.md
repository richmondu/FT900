MbedTLS has been integrated to FT900 to enable secure IoT connectivity via MQTT protocol over LWIP running on Ethernet connection. 
This enables FT900 to communicate with MQTT brokers that require secure TLS/SSL connection, 
such as Amazon AWS Greengrass and AWS IoT cloud, as well as local Mosquitto broker (with TLS/SSL enabled).


### mbedTLS configuration

1. The TLS client configuration for FT900 AWS IoT demo only enables the following ciphersuites due to memory constraints.

   1. MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA
   2. MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA

   These ciphersuites where chosen because these are the minimum ciphersuites supported by Amazon AWS Greengrass.
   Refer to https://docs.aws.amazon.com/greengrass/latest/developerguide/gg-sec.html

   Moreover, these ciphersuites are also supported by Amazon AWS IoT.
   Refer to https://docs.aws.amazon.com/iot/latest/developerguide/iot-security-identity.html 
   (Note that AES128-SHA & AES256-SHA covers the ciphersuites above. This has been proven by the successful test results.)

   Additional ciphersuites may be later added to support Microsoft Azure and Google Cloud.


2. To use these 2 ciphersuites, the following macros and its dependencies are enabled:

   // TLS related configuration
   1. MBEDTLS_SSL_TLS_C // TLS requirement
   2. MBEDTLS_SSL_CLI_C // TLS client requirement
   3. MBEDTLS_CIPHER_C // required by MBEDTLS_SSL_TLS_C
   4. MBEDTLS_MD_C // required by MBEDTLS_SSL_TLS_C
   5. MBEDTLS_ENTROPY_C // required by TLS client usage mbedtls_entropy_xxx()
   6. MBEDTLS_CTR_DRBG_C // required by TLS client usage mbedtls_ctr_drbg_xxx()
   7. MBEDTLS_SSL_PROTO_TLS1_2 // required by AWS IoT, not by AWS Greengrass
   8. MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA,MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA

   // RSA related configurations
   1. MBEDTLS_KEY_EXCHANGE_RSA_ENABLED // required by our chosen ciphersuites
   2. MBEDTLS_RSA_C // required by our chosen ciphersuites
   3. MBEDTLS_OID_C // required by MBEDTLS_RSA_C
   4. MBEDTLS_PKCS1_V15 // required by MBEDTLS_RSA_C
   5. MBEDTLS_BIGNUM_C // required by MBEDTLS_RSA_C

   // X509 Certificate related configurations
   1. MBEDTLS_X509_CRT_PARSE_C // required by MBEDTLS_KEY_EXCHANGE_RSA_ENABLED
   2. MBEDTLS_X509_USE_C // required by MBEDTLS_X509_CRT_PARSE_C
   3. MBEDTLS_ASN1_PARSE_C // required by MBEDTLS_X509_USE_C
   4. MBEDTLS_PK_PARSE_C // required by MBEDTLS_X509_USE_C
   5. MBEDTLS_PK_C // required by MBEDTLS_PK_PARSE_C
   6. MBEDTLS_PEM_PARSE_C // required by format of our given X509 certificates
   7. MBEDTLS_BASE64_C // required by MBEDTLS_PEM_PARSE_C

   // SHA Hash related configuration
   1. MBEDTLS_SHA1_C // required by our chosen ciphersuites
   2. MBEDTLS_SHA256_C // required by MBEDTLS_ENTROPY_C

   // AES symmetric encryption related configuration
   1. MBEDTLS_AES_C // required by our chosen ciphersuites
   2. MBEDTLS_CIPHER_MODE_CBC // required by our chosen ciphersuites

   // Optimization related configuration
   1. MBEDTLS_SSL_MAX_CONTENT_LEN 1536
   2. MBEDTLS_AES_ROM_TABLES // decreases code size by 896 bytes
   3. MBEDTLS_MPI_WINDOW_SIZE 1 // decreases code size by 808 bytes
   4. MBEDTLS_MPI_MAX_SIZE 256 // decreases code size by 64 bytes
   5. MBEDTLS_SHA256_SMALLER // decreases code size by 2944 bytes
   6. MBEDTLS_TLS_DEFAULT_ALLOW_SHA1_IN_KEY_EXCHANGE

   To view the complete list of configurations enabled, please refer to mbedtls_config.h


3. To use this application, aws_clientcredential.h should be modified for the following configurations:
   1. IP address, gateway, subnet mask
   2. MQTT broker address
   3. TLS CA certificate, device certificate, device private key 


### Test results
This application has been tested to work successfully with the following test setup:

1. MQTT w/TLS
   1. AWS IoT, debug and release modes
   2. AWS Greengrass (installed on a Raspberry PI 3B), debug and release modes
   3. Mosquitto (configured w/TLS), debug and release modes

2. MQTT w/o TLS
   1. Mosquitto (configured w/o TLS), debug and release modes


### Test instructions

1. MQTT w/TLS
   1. AWS IoT
      - default configuration
      - set IP address, subnet mask and gateway of FT900 device
      - compile and run
      - then goto http://iotgs-iotgss3bucket-2uxeg22cmka4.s3-website-ap-southeast-1.amazonaws.com/
   2. AWS Greengrass (installed on a Raspberry PI 3B)
      - set USE_MQTT_BROKER to MQTT_BROKER_AWS_GREENGRASS in aws_clientcredential.h
      - set IP address, subnet mask and gateway of FT900 device
      - compile and run
      - then goto http://iotgs-iotgss3bucket-2uxeg22cmka4.s3-website-ap-southeast-1.amazonaws.com/
   3. Mosquitto (configured w/TLS)
      - set USE_MQTT_BROKER to MQTT_BROKER_MOSQUITTO in aws_clientcredential.h
      - set IP address, subnet mask and gateway of FT900 device
      - compile and run
      - then subscribe to mosquitto broker using mosquitto_sub.exe to verify packets sent


### Troubleshooting utilities

1. MQTT.FX is a helpful GUI-based MQTT client tool which can be used for troubleshooting MQTT connection
   Refer to http://mqttfx.jensd.de/

2. mosquitto also provides MQTT clients called mosquitto_pub.exe and mosquitto_sub.exe
   for publishing and subscribing MQTT messages to/from MQTT topics
   Refer to https://mosquitto.org/


### Memory footprint

1. The available memory footprint for sensor data has been tripled from 25kB to 75kB.
   1. code size for AWS Greengrass demo: 181 kB (70% of 256 kB)
      - text: 130856 (128 kB)
      - data: 23332 (22 kB)
      - bss:  30292 (30 kB)
      - total: 181 kB flash memory (70% of 256 kB)
      - available memory for sensor data: 75 kB (30% of 256 kB)
   2. code size for different test combinations:
      - AWS_GREENGRASS, release mode: 184892 (181 kB)
      - AWS_GREENGRASS, debug mode: 216012 (211 kB)
      - AWS_IOT, release mode: 198848 (194 kB)
      - AWS_IOT, debug mode: 233200 (228 kB)
      - MOSQUITTO, release mode: 198416 (194 kB)
      - MOSQUITTO, debug mode: 230824 (225 kB)

2. The optimizations performed include the following:
   1. Use small send and recv buffer size of MQTT
      Whats the use of big buffers if our MQTT packets are small.
      Set these buffers small but big enough to fit our expected send and recv packets.
   2. Make MQTT packet small by making Greengrass append geolocation and timestamp
      Previously, FT900 is adding geolocatin and timestamp information to all MQTT packets.
      Now, we make the Greengrass append this information to make MQTT packets small.
   3. Use small TCP MSS buffer size for LWIP. 
      Broadcasting small MSS buffer size makes peer send small packets.
   4. Make ethernet buffer size based on lwIP configured TCP MSS buffer 
      so that buffer size is not wasted.
   5. Disable MQTT subscriptions. Can be re-enabled by a macro.
      Our demo application does not use MQTT subscriptions.
      Previously, we had MQTT subscriptions for querying the time from Greengrass/SNTP.
   6. Use dynamic allocations on TLS connection and application logic. 
      After TLS connection, a lot of allocated dynamic memory is available for reuse.
   7. Make IP address initialization use SOCKETS_inet_addr_quick
      Saves a few bytes but more efficient.
   8. Use tfp_snprintf consistently instead of mixing snprintf and tfp_snprintf.
      Having two separate snprintf versions linked is efficient in terms of memory used.
      Use only one so only 1 is linked.
   9. Limit debug logs
      Provided centralized debugging macro for minimal and verbose debug logs.
   10. Removed 1 unnecessary macro in mbedTLS configuration.
      Also used mbedTLS configurables intended for memory footprint optimization.


### Scalability

1. USE_DHCP
   - To support multiple device scenario in a network environment, support for DHCP has been added. 
   - It can be enabled by setting the macro USE_DHCP.
   - Enabling USE_DHCP will enable LWIP_DHCP and NET_USE_EEPROM
     NET_USE_EEPROM enables reading of MAC ADDRESS stored in EEPROM.
     Note that if each ft900 device have the same MAC ADDRESS, it will cause conflict with the DHCP server.
   - Advantage: you don't have to set the IP address and gateway address manually
   - Disadvantage: enabling this adds 10kb memory footprint (for RELEASE mode) or 14kb (for DEBUG mode).
   
2. Device certificate and private key
   - Each FT900 device should have its own certificate and private key.
   - AWS or any other server will not allow 2 different devices to use the same certificate and private key simultaneously.
   - You must generate a certificate and private key for each FT900 device.
   

### Security

1. TLS Server certificate verification
   - Server certificate verification is currently disabled by default.
   - It can be enabled by setting the macro USE_ROOTCA
   - Advantage: prevent man-in-the-middle attacks
   - Disadvantage: enabling this adds 2kb memory footprint (for DEBUG mode).
   - Note that this has been tested working on AWS Greengrass scenario. Not yet tested on AWS IoT scenario due to data memory issue.
   - For production release, server certificate verification is a must.
   
2. Elliptic Curve Cryptography (ECC) ciphersuite support and AES_GCM support
   - Stronger ciphersuites are now supported and tested on FT900
   - However, note that the heap size has been increased to support stronger security.
   - Ciphersuite is now configurable by setting the macro USE_CIPHERSUITE
   - CIPHERSUITE_OPTION_1: // AES_CBC
      - MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA
      - MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA
   - CIPHERSUITE_OPTION_2: // AES_GCM is stronger than CBC
      - MBEDTLS_TLS_RSA_WITH_AES_128_GCM_SHA256
      - MBEDTLS_TLS_RSA_WITH_AES_256_GCM_SHA384
   - CIPHERSUITE_OPTION_3: // ECDHE_RSA is strong than RSA
      - MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA
      - MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
   - By default, USE_CIPHERSUITE is set to CIPHERSUITE_OPTION_1 which is a good enough encryption.
