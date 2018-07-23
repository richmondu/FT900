MbedTLS has been integrated to FT900 to enable secure IoT connectivity via MQTT protocol over LWIP running on Ethernet connection. This enables FT900 to communicate with MQTT brokers that require secure TLS/SSL connection, such as Amazon AWS Greengrass and AWS IoT cloud, as well as local Mosquitto broker (with TLS/SSL enabled).


mbedTLS configuration

1. The TLS client configuration for FT900 AWS IoT demo only enables the following ciphersuites due to memory constraints.
   a. MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA
   b. MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA

   These ciphersuites where chosen because these are the minimum ciphersuites supported by Amazon AWS Greengrass.
   Moreover, these ciphersuites are also supported by Amazon AWS IoT.
   Additional ciphersuites may be later added to support Microsoft Azure and Google Cloud.

2. To use these 2 ciphersuites, the following macros and its dependencies are enabled:
   a. MBEDTLS_SSL_TLS_C
   b. MBEDTLS_SSL_CLI_C
   c. MBEDTLS_SSL_PROTO_TLS1_2
   d. MBEDTLS_KEY_EXCHANGE_RSA_ENABLED
   e. MBEDTLS_RSA_C
   f. MBEDTLS_SHA1_C
   g. MBEDTLS_X509_CRT_PARSE_C
   h. MBEDTLS_PEM_PARSE_C
   i. MBEDTLS_CIPHER_MODE_CBC
   j. MBEDTLS_PKCS1_V15

   To view the complete list of configurations enabled, please refer to mbedtls_config.h

3. To use this application, aws_clientcredential.h should be modified for the following configurations:
   a. IP address, gateway, subnet mask
   b. MQTT broker address
   c. TLS CA certificate, device certificate, device private key 


Test results

This application has been tested to work successfully with the following test setup:

1. MQTT w/TLS
   a. AWS IoT
   b. AWS Greengrass
   c. Mosquitto (configured w/TLS)

2. MQTT w/o TLS
   a. Mosquitto (configured w/o TLS)



