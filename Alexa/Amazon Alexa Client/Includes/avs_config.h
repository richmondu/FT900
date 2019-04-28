#ifndef AVS_CONFIG_H
#define AVS_CONFIG_H



///////////////////////////////////////////////////////////////////////////////////
// Set your device ID
// Should be in the range of [1, 16]
///////////////////////////////////////////////////////////////////////////////////
#define AVS_CONFIG_DEVICE_ID            1


///////////////////////////////////////////////////////////////////////////////////
// Set if connecting to different Alexa account
///////////////////////////////////////////////////////////////////////////////////
#define AVS_CONFIG_DIFFERENT_ACCOUNT    0


///////////////////////////////////////////////////////////////////////////////////
// Set your server IP address and port
///////////////////////////////////////////////////////////////////////////////////
#if (COMMUNICATION_IO==1) || (COMMUNICATION_IO==2)
#define AVS_CONFIG_SERVER_ADDR          PP_HTONL(LWIP_MAKEU32(192, 168, 100, 12))
#if AVS_CONFIG_DIFFERENT_ACCOUNT
#define AVS_CONFIG_SERVER_PORT          (11234 + AVS_CONFIG_DEVICE_ID)
#endif // AVS_CONFIG_DIFFERENT_ACCOUNT
#endif // (COMMUNICATION_IO==1) || (COMMUNICATION_IO==2)


///////////////////////////////////////////////////////////////////////////////////
// Configure your connectivity settings
///////////////////////////////////////////////////////////////////////////////////
#if (COMMUNICATION_IO==1)   // Ethernet
#define AVS_CONFIG_ETHERNET_USE_DHCP    1
#define AVS_CONFIG_ETHERNET_IP_ADDRESS  PP_HTONL(LWIP_MAKEU32( 0, 0, 0, 0 ))
#define AVS_CONFIG_ETHERNET_GATEWAY     PP_HTONL(LWIP_MAKEU32( 0, 0, 0, 0 ))
#define AVS_CONFIG_ETHERNET_MASK        PP_HTONL(LWIP_MAKEU32( 0, 0, 0, 0 ))
#define AVS_CONFIG_ETHERNET_DNS         PP_HTONL(LWIP_MAKEU32( 0, 0, 0, 0 ))
#elif (COMMUNICATION_IO==2) // WiFi
#define AVS_CONFIG_WIFI_SSID            "Virus-infected Wifi"
#define AVS_CONFIG_WIFI_PASSWORD        "MahirapTandaan"
#define AVS_CONFIG_WIFI_SECURITY        eWiFiSecurityWPA2
#elif (COMMUNICATION_IO==3) // RS485
// TODO
#endif // (COMMUNICATION_IO==2) // WiFi



#include <avs/avs_config_defaults.h>
#endif // AVS_CONFIG_H
