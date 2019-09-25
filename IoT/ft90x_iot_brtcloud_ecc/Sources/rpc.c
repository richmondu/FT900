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

#include <ft900.h>
#include "tinyprintf.h"
#include "FreeRTOS.h"
#include "net.h"
#include "rpc.h"
#include <stdio.h>
#include <string.h>



///////////////////////////////////////////////////////////////////////////////////
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {CRITICAL_SECTION_BEGIN;tfp_printf(__VA_ARGS__);CRITICAL_SECTION_END;} while (0)
#else
#define DEBUG_PRINTF(...)
#endif
///////////////////////////////////////////////////////////////////////////////////


#if 0
int get_gpio( int number )
{
    uint8_t state;

    gpio_dir((uint8_t)number, pad_dir_output);
    //gpio_pull((uint8_t)number, pad_pull_pullup);

    state = gpio_read((uint8_t)number);

    DEBUG_PRINTF( "%s[%d]=[%d]\r\n", API_GET_GPIO, number, state );
    return (int)state;
}

void set_gpio( int number, int value )
{
    DEBUG_PRINTF( "%s[%d]=[%d]\r\n", API_SET_GPIO, number, value?1:0 );

    gpio_dir((uint8_t)number, pad_dir_output);
    //gpio_pull((uint8_t)number, pad_pull_none);

    gpio_write((uint8_t)number, value?1:0);
}

void init_rtc( void )
{
    // Initialise the RTC to run indefinitely...
    rtc_init();

    set_rtc(0);

    // Use auto refresh mode
    rtc_option(rtc_option_auto_refresh, 1);

    // Start the RTC...
    rtc_start();
}

void set_rtc( uint32_t secs )
{
    // Additional input for time
    time_t time = secs;
    struct tm *time_tm = localtime(&time);

    if (time_tm->tm_year > 70) {
        DEBUG_PRINTF("secs = %u\r\n", (int)secs);
        DEBUG_PRINTF("set %04d-%02d-%02d %02d:%02d:%02d %d\r\n",
            time_tm->tm_year+1900, time_tm->tm_mon+1, time_tm->tm_mday,
            time_tm->tm_hour, time_tm->tm_min, time_tm->tm_sec, time_tm->tm_wday);
    }

    // Set the time to RTC
    if (rtc_write(time_tm) < 0) {
        DEBUG_PRINTF("iot_rtc_set_time failed! rtc_write\r\n");
        return;
    }
}

uint32_t get_rtc( void )
{
    struct tm time_tm = {0};

    // Get the time from RTC
    if (rtc_read(&time_tm)) {
        DEBUG_PRINTF("iot_rtc_get_time failed! rtc_read\r\n");
        return 0;
    }

    DEBUG_PRINTF("get %04d-%02d-%02d %02d:%02d:%02d %d\r\n",
        time_tm.tm_year+1900, time_tm.tm_mon+1, time_tm.tm_mday,
        time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec, time_tm.tm_wday);

    uint32_t secs = mktime(&time_tm);
    return secs;
}

void get_mac( uint8_t* mac )
{
    uint8_t* ptr = net_get_mac();
    memcpy(mac, ptr, 6);
    //int ret = net_get_mac_eeprom(mac);
    DEBUG_PRINTF("%02X:%02X:%02X:%02X:%02X:%02X\r\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void set_mac( uint8_t* mac_str )
{
    uint8_t mac[6] = {0};

    uint8_t* ptr = mac_str;
    for (int i=0; i<6; i++) {
        mac[i] = (uint8_t)strtoul((char *)ptr, (char **)&ptr, 16);
        ptr++;
    }

    DEBUG_PRINTF("%02X:%02X:%02X:%02X:%02X:%02X\r\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    net_set_mac_eeprom(mac);
}
#endif
void restart_task( void *param )
{
    vTaskDelay( pdMS_TO_TICKS(1000) );
    chip_reboot();
}

