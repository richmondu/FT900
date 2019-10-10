/**
 * \file config.h
 *
 * \brief Configuration options (set of defines)
 *
 *  This set of compile-time options may be used to enable
 *  or disable features selectively, and reduce the global
 *  memory footprint.
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

// Added this for optimization purposes
// Remove this if not using mbedtls for iot
#include <iot_config.h> // For USE_MBEDTLS_MAX_SIZES



/*-----------------------------------------------------------*/

#define CIPHERSUITE_OPTION_1         1 // strong:    RSA_AES128_CBC_SHA, RSA_AES256_CBC_SHA
#define CIPHERSUITE_OPTION_2         2 // stronger:  RSA_AES128_GCM_SHA256, RSA_AES256_GCM_SHA384
#define CIPHERSUITE_OPTION_3         3 // strongest: ECDHE_RSA_AES128_CBC_SHA, ECDHE_RSA_AES256_CBC_SHA
#define CIPHERSUITE_OPTION_4         4 // strongest: ECDHE_ECDSA_AES128_CBC_SHA256, ECDHE_ECDSA_AES128_GCM_SHA256
#define CIPHERSUITE_OPTION_DEFAULT   CIPHERSUITE_OPTION_4 // set to 1 if RSA
#define USE_CIPHERSUITE              CIPHERSUITE_OPTION_DEFAULT

#ifndef USE_MBEDTLS_MAX_SIZES
#define USE_MBEDTLS_MAX_SIZES        1
#endif

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
    #define USE_ECDSA_OVER_RSA       0
#elif (USE_CIPHERSUITE == CIPHERSUITE_OPTION_4)
    #define USE_GCM_OVER_CBC         0
    #define USE_ECC_CIPHERSUITE      1
    #define USE_ECDSA_OVER_RSA       1
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
        #if USE_ECDSA_OVER_RSA
            #define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256
        #else
            #define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
        #endif
    #else
        #if USE_ECDSA_OVER_RSA
            #define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256
        #else
            #define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
        #endif
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
	// ECDSA-related configurations
	#if USE_ECDSA_OVER_RSA
		#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
		#define MBEDTLS_ECDSA_C
		#define MBEDTLS_ASN1_WRITE_C
		// ECC optimizations
		#define MBEDTLS_ECP_NIST_OPTIM
		#define MBEDTLS_ECP_FIXED_POINT_OPTIM		0
		#define MBEDTLS_ECP_WINDOW_SIZE				2
		#define MBEDTLS_ECP_MAX_BITS 				256
	#else
		#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED // required by our chosen ciphersuites
	#endif
	#define MBEDTLS_ECDH_C               		// required by our chosen ciphersuites
	#define MBEDTLS_ECP_C                		// required by MBEDTLS_ECDH_C
	#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#else // USE_ECC_CIPHERSUITE
	// RSA-related configurations
	#define MBEDTLS_KEY_EXCHANGE_RSA_ENABLED	// required by our chosen ciphersuites
	#define MBEDTLS_RSA_C                		// required by our chosen ciphersuites
	#define MBEDTLS_PKCS1_V15            		// required by MBEDTLS_RSA_C
#endif // USE_ECC_CIPHERSUITE

/*-----------------------------------------------------------*/

// X509 Certificate related configurations
#define MBEDTLS_X509_CRT_PARSE_C     // required by MBEDTLS_KEY_EXCHANGE_RSA_ENABLED
#define MBEDTLS_X509_USE_C           // required by MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_ASN1_PARSE_C         // required by MBEDTLS_X509_USE_C
#define MBEDTLS_PK_PARSE_C           // required by MBEDTLS_X509_USE_C
#define MBEDTLS_PK_C                 // required by MBEDTLS_PK_PARSE_C
#define MBEDTLS_PEM_PARSE_C          // required by format of our given X509 certificates
#define MBEDTLS_BASE64_C             // required by MBEDTLS_PEM_PARSE_C
#define MBEDTLS_OID_C                // required by MBEDTLS_X509_USE_C
#define MBEDTLS_BIGNUM_C             // required by MBEDTLS_X509_USE_C

/*-----------------------------------------------------------*/

// SHA Hash related configuration
#define MBEDTLS_SHA1_C               // required by our chosen ciphersuites
#define MBEDTLS_SHA256_C             // required by MBEDTLS_ENTROPY_C
#ifdef MBEDTLS_SSL_PROTO_TLS1
#define MBEDTLS_MD5_C
#endif

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
#if (USE_MBEDTLS_MAX_SIZES==3)
#define MBEDTLS_SSL_MAX_CONTENT_LEN  (1024+512) // Support AWS IoT ATS
#define MBEDTLS_MPI_MAX_SIZE         (256)      // Support AWS IoT ATS
#elif (USE_MBEDTLS_MAX_SIZES==2)
#define MBEDTLS_SSL_MAX_CONTENT_LEN  (4096+910) // Support AWS IoT ATS
#define MBEDTLS_MPI_MAX_SIZE         (256)      // Support AWS IoT ATS
#elif (USE_MBEDTLS_MAX_SIZES==1)
#define MBEDTLS_SSL_MAX_CONTENT_LEN  (4096)     // Support Azure IoT
#define MBEDTLS_MPI_MAX_SIZE         (512)      // Support Azure IoT
#else
#define MBEDTLS_SSL_MAX_CONTENT_LEN  (3072)     // Works with AWS IoT/Greengrass and GCP IoT
#define MBEDTLS_MPI_MAX_SIZE         (256)      // Works with AWS IoT/Greengrass and GCP IoT
#endif
#define MBEDTLS_AES_ROM_TABLES       // decreases code size by 896 bytes
#define MBEDTLS_MPI_WINDOW_SIZE 1    // decreases code size by 808 bytes
#define MBEDTLS_SHA256_SMALLER       // decreases code size by 2944 bytes
#define MBEDTLS_TLS_DEFAULT_ALLOW_SHA1_IN_KEY_EXCHANGE
#define MBEDTLS_TLS_DEFAULT_ALLOW_SHA1_IN_CERTIFICATES

/*-----------------------------------------------------------*/

// Platform related configuration
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_STD_CALLOC
#define MBEDTLS_PLATFORM_STD_SNPRINTF
#define MBEDTLS_PLATFORM_STD_FREE

#define MBEDTLS_PLATFORM_MEMORY
/* CALLOC and FREE will get pointed to tls_malloc and tls_free in
 * "lwip/apps/altcp_tls_mbedtls_opts.c"
 */
#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS

#define MBEDTLS_PLATFORM_SNPRINTF_ALT

/* Support for port 443 */
#if (MQTT_BROKER_PORT == 443)
#define MBEDTLS_SSL_ALPN
#endif

/*-----------------------------------------------------------*/

#include "mbedtls/check_config.h"

#endif /* MBEDTLS_CONFIG_H */
