/**
 * @file wifi.c
 * @brief WiFi Interface.
 */

/* FreeRTOS includes. */
#include <stdint.h>
#include <wifi.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* WiFi driver includes. */

/* Socket and WiFi interface includes. */
#include "at.h"



///////////////////////////////////////////////////////////////////////////////////
/* Debug logs */
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else // DEBUG
#define DEBUG_PRINTF(...)
#endif // DEBUG
///////////////////////////////////////////////////////////////////////////////////



/**
 * @brief WiFi initialization status.
 */
static BaseType_t xWIFIInitDone = pdFALSE;

/**
 * @brief Maximum time to wait in ticks for obtaining the WiFi semaphore
 * before failing the operation.
 */

//static const TickType_t xSemaphoreWaitTicks = pdMS_TO_TICKS( wificonfigMAX_SEMAPHORE_WAIT_TIME_MS );

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_On( void )
{
    WIFIReturnCode_t xRetVal = eWiFiFailure;

    /* One time WiFi initialization */
    if( xWIFIInitDone == pdFALSE )
    {
        int8_t atRet = at_init(UART1, NULL);
        if(AT_OK == atRet)
        {
            atRet = at_set_cwmode(at_mode_station);
            atRet |= at_set_cipmux(at_enable);
            if(AT_OK == atRet)
            {
                xWIFIInitDone = pdTRUE;
                xRetVal = eWiFiSuccess;
            }
        }
        else
        {
            DEBUG_PRINTF("AT init failed with code %d \r\n", atRet);
        }
    }

    return xRetVal;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Off( void )
{
    /*command not implemented in ES WiFi drivers. */
    WIFIReturnCode_t xRetVal = eWiFiNotSupported;

    return xRetVal;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_ConnectAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    WIFIReturnCode_t xRetVal = eWiFiFailure;
    if(xWIFIInitDone == pdTRUE)
    {
        struct at_set_cwjap_s cwjap_data;
        memcpy(cwjap_data.ssid, pxNetworkParams->pcSSID, pxNetworkParams->ucSSIDLength);
        memcpy(cwjap_data.pwd, pxNetworkParams->pcPassword, pxNetworkParams->ucPasswordLength);
        memset(cwjap_data.bssid, 0x00, sizeof(cwjap_data.bssid));
        int8_t ret = at_set_cwjap(&cwjap_data);
        if(ret == AT_OK)
            xRetVal = eWiFiSuccess;
        else
            DEBUG_PRINTF("Connect failed with code %d \r\n", ret);

    }
    return xRetVal;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Disconnect( void )
{
    WIFIReturnCode_t xRetVal = eWiFiFailure;
    at_cwqap();
    DEBUG_PRINTF("Disconnect from AP \r\n");
    return xRetVal;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Reset( void )
{
    return eWiFiSuccess;
}


/*-----------------------------------------------------------*/

static enum at_mode device_to_wifi_mode(WIFIDeviceMode_t xDeviceMode)
{
    switch(xDeviceMode)
    {
        case eWiFiModeStation:
            return at_mode_station;

        case eWiFiModeAP:
            return at_mode_soft_ap;

        default:
            return at_mode_station_soft_ap;
    }
}

WIFIReturnCode_t WIFI_SetMode( WIFIDeviceMode_t xDeviceMode )
{
    WIFIReturnCode_t xRetVal = eWiFiFailure;

    enum at_mode x = device_to_wifi_mode(xDeviceMode);
    int8_t at_ret = at_set_cwmode(x);
    if(AT_OK == at_ret)
        xRetVal = eWiFiSuccess;
    return xRetVal;
}

/*-----------------------------------------------------------*/

static WIFIDeviceMode_t wifi_to_device_mode(enum at_mode x)
{
    switch(x)
    {
        case at_mode_station:
            return eWiFiModeStation;

        case at_mode_soft_ap:
            return eWiFiModeAP;

        default:
            return eWiFiModeAP;
    }
}

WIFIReturnCode_t WIFI_GetMode( WIFIDeviceMode_t * pxDeviceMode )
{
    WIFIReturnCode_t xRetVal = eWiFiFailure;
    enum at_mode x;
    int8_t at_ret = at_query_cwmode(&x);
    if(at_ret == AT_OK)
    {
        *pxDeviceMode = wifi_to_device_mode(x);
        xRetVal = eWiFiSuccess;
    }
    return xRetVal;
}
/*-----------------------------------------------------------*/


WIFIReturnCode_t WIFI_NetworkAdd( const WIFINetworkProfile_t * const pxNetworkProfile,
                                  uint16_t * pusIndex )
{
    WIFIReturnCode_t xRetVal = eWiFiNotSupported;

    return xRetVal;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkGet( WIFINetworkProfile_t * pxNetworkProfile,
                                  uint16_t usIndex )

{
    WIFIReturnCode_t xRetVal = eWiFiNotSupported;

    return xRetVal;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkDelete( uint16_t usIndex )
{
    WIFIReturnCode_t xRetVal = eWiFiNotSupported;

    return xRetVal;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Ping( uint8_t * pucIPAddr,
                            uint16_t usCount,
                            uint32_t ulIntervalMS )
{
    WIFIReturnCode_t xRetVal = eWiFiFailure;



    return xRetVal;
}

/*-----------------------------------------------------------*/
WIFIReturnCode_t WIFI_GetIP( uint8_t * pucIPAddr )
{
    WIFIReturnCode_t xRetVal = eWiFiFailure;
    int8_t at_ret = at_query_cipsta((char*)pucIPAddr, NULL, NULL);
    if (AT_OK == at_ret)
    {
        xRetVal = eWiFiSuccess;
    }

    return xRetVal;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetMAC( uint8_t * pucMac )
{
    WIFIReturnCode_t xRetVal = eWiFiFailure;



    return xRetVal;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetHostIP( char * pcHost,
                                 uint8_t * pucIPAddr )
{
    WIFIReturnCode_t xRetVal = eWiFiFailure;

    /* Try to acquire the semaphore. */

    return xRetVal;
}

/*-----------------------------------------------------------*/

/*Scan fails if the command buffer is too small to fit in scan result and it returns
 * IO error, see es_wifi_conf.h*/
WIFIReturnCode_t WIFI_Scan( WIFIScanResult_t * pxBuffer,
                            uint8_t ucNumNetworks )
{
    WIFIReturnCode_t xRetVal = eWiFiFailure;

    return xRetVal;
}

/*-----------------------------------------------------------*/
WIFIReturnCode_t WIFI_StartAP( void )
{
    /*WIFI_ConfigureAP configures and start the soft AP . */
    WIFIReturnCode_t xRetVal = eWiFiNotSupported;

    return xRetVal;
}

/*-----------------------------------------------------------*/
WIFIReturnCode_t WIFI_StopAP( void )
{
    /*SoftAP mode stops after a timeout. */
    WIFIReturnCode_t xRetVal = eWiFiNotSupported;

    return xRetVal;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_ConfigureAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    WIFIReturnCode_t xRetVal = eWiFiFailure;

    return xRetVal;
}

/*-----------------------------------------------------------*/
WIFIReturnCode_t WIFI_SetPMMode( WIFIPMMode_t xPMModeType,
                                 const void * pvOptionValue )
{
    WIFIReturnCode_t xRetVal = eWiFiNotSupported;

    return xRetVal;
}

/*-----------------------------------------------------------*/
WIFIReturnCode_t WIFI_GetPMMode( WIFIPMMode_t * pxPMModeType,
                                 void * pvOptionValue )
{
    WIFIReturnCode_t xRetVal = eWiFiNotSupported;

    return xRetVal;
}
