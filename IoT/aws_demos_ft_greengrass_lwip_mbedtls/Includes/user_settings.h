#ifndef __USER_SETTINGS__H__
#define __USER_SETTINGS__H__



/*
 * Switch between DHCP or static IP address
 * This is useful for multiple device scenario
 * Set this to 1 so IP address is not set manually
 * However, note that enabling this adds memory footprint of
 * 10kb for Release mode, 14kb for Debug mode
 * Set this to 0 if you need more memory for sensor code
 */
#define USE_DHCP 0 // recommended


/*
 * DO NOT FORGET TO UPDATE THESE VALUES FOR FT900 if USE_DHCP is 0
 * Setup the IP address, gateway and subnet mask of this FT900 device
 */
#include "secure_sockets.h" // SOCKETS_inet_addr_quick for lesser footprint
#if USE_DHCP
#define FT9XX_IP_ADDRESS SOCKETS_inet_addr_quick(0, 0, 0, 0) // no need to set since USE_DHCP is 1
#define FT9XX_IP_GATEWAY SOCKETS_inet_addr_quick(0, 0, 0, 0) // no need to set since USE_DHCP is 1
#else // USE_DHCP
#define FT9XX_IP_ADDRESS SOCKETS_inet_addr_quick(192, 168, 22, 200)
#define FT9XX_IP_GATEWAY SOCKETS_inet_addr_quick(192, 168, 22, 1)
#endif // USE_DHCP
#define FT9XX_IP_SUBNET  SOCKETS_inet_addr_quick(255, 255, 255, 0)


/*
 * Switch between secure MQTT and non-secure MQTT
 * Note that non-secure MQTT can be tested using Mosquitto local broker only
 * AWS does not support non-secure MQTT
 */
#define USE_TLS 1 // required; do not modify


/*
 * Switch between verifying Root CA certificate
 * If 0, root CA server certificate will not be verified.
 * This is prone to man-in-the-middle attacks.
 * However, note that the client certificate and private key are still verified.
 * For production release, this must be enabled.
 */
#define USE_ROOTCA 0 // optional for demo; highly recommended if production


/*
 * This moves the certificate from .data section to .text section
 * Thus allowing more data memory
 * Note that program memory increases
 * When using RootCA, the certificate optimization is really helpful
 * The RootCA, device certificate and private key will be moved to
 * .txt (program memory) instead of .data (data memory)
 * That is an estimate of 4.5kb of memory
 */
#define USE_CERT_OPTIMIZATION 1 // highly recommended



#endif // __USER_SETTINGS__H__
