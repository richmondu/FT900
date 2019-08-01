/*
 * ============================================================================
 * History
 * =======
 * 18 Jun 2019 : Created
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

#ifndef SOURCES_RPC_H_
#define SOURCES_RPC_H_


#define API_GET_GPIO                "get_gpio"
#define API_SET_GPIO                "set_gpio"
#define API_GET_RTC                 "get_rtc"
#define API_SET_RTC                 "set_rtc"
#define API_GET_MAC                 "get_mac"
#define API_GET_IP                  "get_ip"
#define API_GET_SUBNET              "get_subnet"
#define API_GET_GATEWAY             "get_gateway"
#define API_SET_MAC                 "set_mac"
#define API_GET_STATUS              "get_status"
#define API_SET_STATUS              "set_status"
#define API_WRITE_UART              "write_uart"
#define API_TRIGGER_NOTIFICATIONS   "trigger_notifications"
#define API_STATUS_RESTART          "restart"
#define API_STATUS_RUNNING          "running"
#define API_STATUS_RESTARTING       "restarting"

int      get_gpio( int number );
void     set_gpio( int number, int value );
void     init_rtc( void );
void     set_rtc ( uint32_t secs );
uint32_t get_rtc ( void );
void     get_mac ( uint8_t* mac );
void     set_mac ( uint8_t* mac );

void     restart_task( void *param );

#endif /* SOURCES_RPC_H_ */
