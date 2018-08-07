#ifndef __IOT_CONFIG__H__
#define __IOT_CONFIG__H__



/*
 * IP address settings for the FT9XX device.
 * DHCP is enabled by default; no longer configurable for a more production-ready solution
 * If you need to disable DHCP (to save 10kb-14kb of program memory),
 *   1. you can set LWIP_DHCP in lwipopts.h
 *   2. disable NET_USE_EEPROM in net.h (to hardcode the MAC address as well)
 *   3. and update ip address, gateway and subnet mask manually for each FT900 device
 */
#include "iot_secure_sockets.h" // for SOCKETS_inet_addr_quick
#define IOT_CONFIG_IP_ADDRESS SOCKETS_inet_addr_quick(0, 0, 0, 0)
#define IOT_CONFIG_IP_GATEWAY SOCKETS_inet_addr_quick(0, 0, 0, 0)
#define IOT_CONFIG_IP_SUBNET  SOCKETS_inet_addr_quick(255, 255, 255, 0)


/*
 * Switch between secure MQTT and non-secure MQTT
 * Note that non-secure MQTT can be tested using Mosquitto local broker only
 * AWS does not support non-secure MQTT
 */
#define IOT_CONFIG_USE_TLS 1 // required; modify only if not yet familiar with MQTT and IoT


/*
 * Switch for TLS cipher suite option (security level)
 */
#define CIPHERSUITE_OPTION_1 1 // strong:    RSA_AES128_CBC_SHA, RSA_AES256_CBC_SHA
#define CIPHERSUITE_OPTION_2 2 // stronger:  RSA_AES128_GCM_SHA256, RSA_AES256_GCM_SHA384
#define CIPHERSUITE_OPTION_3 3 // strongest: ECDHE_RSA_AES128_CBC_SHA, ECDHE_RSA_AES256_CBC_SHA
#define IOT_CONFIG_USE_CIPHERSUITE CIPHERSUITE_OPTION_1


/*
 * Switch between verifying TLS Root CA certificate
 * If 0, root CA server certificate will not be verified.
 * This is prone to man-in-the-middle attacks.
 * However, note that the client certificate and private key are still verified.
 * For production release, this must be enabled for two-way authentication.
 */
#define IOT_CONFIG_USE_ROOTCA 0 // optional for demo; highly recommended for production


/*
 * Switch between using TLS certificate optimization and not
 * This optimization option moves the certificate from .data section to .text section
 *   Thus allowing more data memory (but program memory increases)
 *   When using RootCA, the certificate optimization is really helpful
 *   The RootCA, device certificate and private key will be moved to
 *   .txt (program memory) instead of .data (data memory)
 *   That is an estimate of 4.5kb of memory
 *   We have already 60-75kb of program memory but not enough data memory.
 *   So this provides a balanced memory headroom (both program memory and data memory) for sensor integration.
 * Certificates should be stored in the certificates folder
 *   with the filenames: ca.crt, cert.crt and cert.key
 *   These certificates are compiled using USE_CERTIFICATE_OPTIMIZATION.bat
 *     which is called in the Pre-build step in the "Build Steps" tab in (Properties / C/C++ Build / Settings)
 *   These certificates are then linked to the binary by adding the object files in
 *     FT9XX GCC Linker/Miscellaneous/Other objects of "Tool Settings" tab in (Properties / C/C++ Build / Settings)
 * Alternatively, the certificates can be hardcoded in iot_clientcredential.h
 *   by setting IOT_CONFIG_USE_CERT_OPTIMIZATION to 0 in iot_config.h
 */
#define IOT_CONFIG_USE_CERT_OPTIMIZATION 1 // highly recommended for demo and production



#endif // __IOT_CONFIG__H__
