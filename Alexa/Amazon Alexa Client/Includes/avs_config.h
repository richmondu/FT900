#ifndef AVS_CONFIG_H
#define AVS_CONFIG_H



///////////////////////////////////////////////////////////////////////////////////
// Set your device ID
///////////////////////////////////////////////////////////////////////////////////
#define AVS_CONFIG_DEVICE_ID            0x00000001


///////////////////////////////////////////////////////////////////////////////////
// Set your server IP address
///////////////////////////////////////////////////////////////////////////////////
#define AVS_CONFIG_SERVER_ADDR          PP_HTONL(LWIP_MAKEU32(192, 168, 100, 12))


///////////////////////////////////////////////////////////////////////////////////
// Choose your sampling rate
//   SAMPLING_RATE_44100HZ
//   SAMPLING_RATE_48KHZ
//   SAMPLING_RATE_32KHZ
//   SAMPLING_RATE_16KHZ
//   SAMPLING_RATE_8KHZ
///////////////////////////////////////////////////////////////////////////////////
#define AVS_CONFIG_SAMPLING_RATE        SAMPLING_RATE_16KHZ


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
#endif // (COMMUNICATION_IO==2) // WiFi



#include <avs/avs_config_defaults.h>
#endif // AVS_CONFIG_H
