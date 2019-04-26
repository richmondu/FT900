/**
  @file wifi.c
  @brief
  ESP32 Wifi module.

 */
/*
 * ============================================================================
 * History
 * =======
 * 2019-04-25 : Created v1
 *
 * Copyright (C) Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 * ("Bridgetek") subject to the licence terms set out
 * http://brtchip.com/BRTSourceCodeLicenseAgreement/ ("the Licence Terms").
 * You must read the Licence Terms before downloading or using the Software.
 * By installing or using the Software you agree to the Licence Terms. If you
 * do not agree to the Licence Terms then do not download or use the Software.
 *
 * Without prejudice to the Licence Terms, here is a summary of some of the key
 * terms of the Licence Terms (and in the event of any conflict between this
 * summary and the Licence Terms then the text of the Licence Terms will
 * prevail).
 *
 * The Software is provided "as is".
 * There are no warranties (or similar) in relation to the quality of the
 * Software. You use it at your own risk.
 * The Software should not be used in, or for, any medical device, system or
 * appliance. There are exclusions of Bridgetek liability for certain types of loss
 * such as: special loss or damage; incidental loss or damage; indirect or
 * consequential loss or damage; loss of income; loss of business; loss of
 * profits; loss of revenue; loss of contracts; business interruption; loss of
 * the use of money or anticipated savings; loss of information; loss of
 * opportunity; loss of goodwill or reputation; and/or loss of, damage to or
 * corruption of data.
 * There is a monetary cap on Bridgetek's liability.
 * The Software may have subsequently been amended by another user and then
 * distributed by that other user ("Adapted Software").  If so that user may
 * have additional licence terms that apply to those amendments. However, Bridgetek
 * has no liability in relation to those amendments.
 * ============================================================================
 */
#if (COMMUNICATION_IO==2) // WiFi
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
WIFIReturnCode_t WIFI_GetIP( uint8_t * pucIPAddr, uint8_t * pucGateway, uint8_t * pucMask )
{
    WIFIReturnCode_t xRetVal = eWiFiFailure;
    int8_t at_ret = at_query_cipsta((char*)pucIPAddr, (char*)pucGateway, (char*)pucMask);
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
#endif
