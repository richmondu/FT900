#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

#include "tinyprintf.h" // for tfp_snprintf and tfp_printf needed by platform.c
#include "aws_clientcredential.h" // for USE_MQTT_BROKER switch
#include "FreeRTOS.h" // for portable.h needed by platform.c
#include <string.h> // for memset needed by platform.c



/*-----------------------------------------------------------*/

#define CIPHERSUITE_OPTION_1         1 // strong:    RSA_AES128_CBC_SHA, RSA_AES256_CBC_SHA
#define CIPHERSUITE_OPTION_2         2 // stronger:  RSA_AES128_GCM_SHA256, RSA_AES256_GCM_SHA384
#define CIPHERSUITE_OPTION_3         3 // strongest: ECDHE_RSA_AES128_CBC_SHA, ECDHE_RSA_AES256_CBC_SHA
#define CIPHERSUITE_OPTION_DEFAULT   CIPHERSUITE_OPTION_1
#define USE_CIPHERSUITE              CIPHERSUITE_OPTION_DEFAULT

/*-----------------------------------------------------------*/

#if (USE_CIPHERSUITE == CIPHERSUITE_OPTION_1)
    #define USE_GCM_OVER_CBC         0
    #define USE_ECC_CIPHERSUITE      0
#elif (USE_CIPHERSUITE == CIPHERSUITE_OPTION_2)
    #define USE_GCM_OVER_CBC         1
    #define USE_ECC_CIPHERSUITE      0
#elif (USE_CIPHERSUITE == CIPHERSUITE_OPTION_3)
    #define USE_GCM_OVER_CBC         0
    #define USE_ECC_CIPHERSUITE      1
#endif

/*-----------------------------------------------------------*/

// TLS related configuration
#define MBEDTLS_SSL_TLS_C            // TLS requirement
#define MBEDTLS_SSL_CLI_C            // TLS client requirement
#define MBEDTLS_CIPHER_C             // required by MBEDTLS_SSL_TLS_C
#define MBEDTLS_MD_C                 // required by MBEDTLS_SSL_TLS_C
#define MBEDTLS_ENTROPY_C            // required by TLS client usage mbedtls_entropy_xxx()
#define MBEDTLS_CTR_DRBG_C           // required by TLS client usage mbedtls_ctr_drbg_xxx()
#define MBEDTLS_SSL_PROTO_TLS1_2     // required by AWS IoT, not by AWS Greengrass

/*-----------------------------------------------------------*/

// Ciphersuites supported
// AWS Greengrass supports these minimum ciphersuites
// AWS IoT also supports these ciphersuites
#if USE_ECC_CIPHERSUITE
    #if USE_GCM_OVER_CBC
    #else
        #define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
    #endif
#else
    #if USE_GCM_OVER_CBC
        #define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_RSA_WITH_AES_128_GCM_SHA256,MBEDTLS_TLS_RSA_WITH_AES_256_GCM_SHA384
    #else
        #define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA,MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA
    #endif
#endif

/*-----------------------------------------------------------*/

// Key exchange-related configurations
#if USE_ECC_CIPHERSUITE
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED // required by our chosen ciphersuites
#define MBEDTLS_ECDH_C               // required by our chosen ciphersuites
#define MBEDTLS_ECP_C                // required by MBEDTLS_ECDH_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#else // USE_ECC_CIPHERSUITE
#define MBEDTLS_KEY_EXCHANGE_RSA_ENABLED // required by our chosen ciphersuites
#endif // USE_ECC_CIPHERSUITE

/*-----------------------------------------------------------*/

// RSA-related configurations
#define MBEDTLS_RSA_C                // required by our chosen ciphersuites
#define MBEDTLS_OID_C                // required by MBEDTLS_RSA_C
#define MBEDTLS_PKCS1_V15            // required by MBEDTLS_RSA_C
#define MBEDTLS_BIGNUM_C             // required by MBEDTLS_RSA_C

/*-----------------------------------------------------------*/

// X509 Certificate related configurations
#define MBEDTLS_X509_CRT_PARSE_C     // required by MBEDTLS_KEY_EXCHANGE_RSA_ENABLED
#define MBEDTLS_X509_USE_C           // required by MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_ASN1_PARSE_C         // required by MBEDTLS_X509_USE_C
#define MBEDTLS_PK_PARSE_C           // required by MBEDTLS_X509_USE_C
#define MBEDTLS_PK_C                 // required by MBEDTLS_PK_PARSE_C
#define MBEDTLS_PEM_PARSE_C          // required by format of our given X509 certificates
#define MBEDTLS_BASE64_C             // required by MBEDTLS_PEM_PARSE_C

/*-----------------------------------------------------------*/

// SHA Hash related configuration
#define MBEDTLS_SHA1_C               // required by our chosen ciphersuites
#define MBEDTLS_SHA256_C             // required by MBEDTLS_ENTROPY_C

/*-----------------------------------------------------------*/

// AES symmetric encryption related configuration
#define MBEDTLS_AES_C                // required by our chosen ciphersuites

#if USE_GCM_OVER_CBC
#define MBEDTLS_GCM_C                // required by our chosen ciphersuites
#else // USE_GCM_OVER_CBC
#define MBEDTLS_CIPHER_MODE_CBC      // required by our chosen ciphersuites
#endif // USE_GCM_OVER_CBC

/*-----------------------------------------------------------*/

// Optimization related configuration
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    #define MBEDTLS_SSL_MAX_CONTENT_LEN     (1536)
#elif (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT)
    #if USE_ECC_CIPHERSUITE
        #define MBEDTLS_SSL_MAX_CONTENT_LEN (3072+320)
    #else
        #define MBEDTLS_SSL_MAX_CONTENT_LEN (3072)
    #endif
#elif (USE_MQTT_BROKER == MQTT_BROKER_MOSQUITTO)
    #define MBEDTLS_SSL_MAX_CONTENT_LEN     (3072)
    #define MBEDTLS_SHA512_C         // required by my self-generated X509 certificates
#endif
#define MBEDTLS_AES_ROM_TABLES       // decreases code size by 896 bytes
#define MBEDTLS_MPI_WINDOW_SIZE 1    // decreases code size by 808 bytes
#define MBEDTLS_MPI_MAX_SIZE 256     // decreases code size by 64 bytes
#define MBEDTLS_SHA256_SMALLER       // decreases code size by 2944 bytes
#define MBEDTLS_TLS_DEFAULT_ALLOW_SHA1_IN_KEY_EXCHANGE

/*-----------------------------------------------------------*/

// Platform related configuration
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_STD_CALLOC
#define MBEDTLS_PLATFORM_STD_SNPRINTF
#define MBEDTLS_PLATFORM_STD_FREE
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS
#define MBEDTLS_PLATFORM_SNPRINTF_ALT

/*-----------------------------------------------------------*/

// Modifications were added in x509_crt.c to reduce some .txt memory
// The modifications were enclosed in
// MBEDTLS_X509_CRT_INFO and MBEDTLS_X509_CRT_VERIFY_INFO

/*-----------------------------------------------------------*/


#include "mbedtls/check_config.h"

#endif /* MBEDTLS_CONFIG_H */
