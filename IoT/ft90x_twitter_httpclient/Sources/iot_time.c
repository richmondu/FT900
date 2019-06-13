/*
 * ============================================================================
 * History
 * =======
 * 29 Oct 2018 : Created
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

#include <ft900.h>          // For rtc_xxx, sys_check_ft900_revB
#include "tinyprintf.h"     // For tfp_printf
#include "lwip/apps/mqtt.h" // For MQTT_TLS_PORT
#include "lwip/apps/sntp.h" // For SNTP_OPMODE_POLL
#include "lwip/netdb.h"     // For hostent

#include "FreeRTOS.h"       // For pvPortMalloc



#define USE_PAYLOAD_TIMESTAMP 0



//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



#if USE_PAYLOAD_TIMESTAMP
// Uses internal RTC (for Rev C)
static int iot_rtc_set_time(uint32_t secs)
{
    // Initialise the RTC to run indefinitely...
    rtc_init();

    // Additional input for time
    time_t time = secs;
    struct tm *time_tm = localtime(&time);
    DEBUG_PRINTF("secs = %u\r\n", secs);
    DEBUG_PRINTF("set %04d-%02d-%02d %02d:%02d:%02d %d\r\n",
        time_tm->tm_year, time_tm->tm_mon, time_tm->tm_mday,
        time_tm->tm_hour, time_tm->tm_min, time_tm->tm_sec, time_tm->tm_wday);

    // Set the time to RTC
    if (rtc_write(time_tm) < 0) {
        DEBUG_PRINTF("iot_rtc_set_time failed! rtc_write\r\n");
        return 0;
    }

    // Use auto refresh mode
    rtc_option(rtc_option_auto_refresh, 1);

    // Start the RTC...
    rtc_start();

    return 1;
}

int iot_rtc_get_date_stamp(char* pcDate, int iSize)
{
    // Get the time from RTC
	struct tm time_tm;
    if (rtc_read(&time_tm)) {
        DEBUG_PRINTF("iot_rtc_get_date failed! rtc_read\r\n");
        return 0;
    }

    tfp_snprintf(pcDate, iSize, "%04d%02d%02d",
        time_tm.tm_year+1900, time_tm.tm_mon+1, time_tm.tm_mday);

    return 1;
}

int iot_rtc_get_amz_date(char* pcDate, int iSize)
{
    // Get the time from RTC
	struct tm time_tm;
    if (rtc_read(&time_tm)) {
        DEBUG_PRINTF("iot_rtc_get_date failed! rtc_read\r\n");
        return 0;
    }

    tfp_snprintf(pcDate, iSize, "%04d%02d%02dT%02d%02d%02dZ",
        time_tm.tm_year+1900, time_tm.tm_mon+1, time_tm.tm_mday,
        time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec);

    return 1;
}
#endif




#if 1//(USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT) || USE_PAYLOAD_TIMESTAMP

static time_t g_timenow = 0;

void iot_sntp_start()
{
    char* sntp_servers[] = {"time.google.com", "time.windows.com"};
    int sntp_servers_count = sizeof(sntp_servers)/sizeof(sntp_servers[0]);
    ip_addr_t sntp_ip = {0};
    struct hostent *host= NULL;

    do {
        sntp_setoperatingmode(0);
        for (int i=0; i<sntp_servers_count; i++) {
            host = gethostbyname(sntp_servers[i]);
            if (host == NULL) {
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }
            memcpy(&sntp_ip, host->h_addr_list[0], sizeof(ip_addr_t));
            sntp_setserver(0, &sntp_ip);
            break;
        }
    } while (sntp_ip.addr == 0 && net_is_ready());
    sntp_init();

    g_timenow = 0;

}

void iot_sntp_stop()
{
    sntp_stop();
}

void iot_sntp_set_system_time(uint32_t sec)
{
#if USE_PAYLOAD_TIMESTAMP
    if (g_timenow == 0) {
        iot_rtc_set_time(sec);
    }
#endif // USE_PAYLOAD_TIMESTAMP

#if 0 // debug the time received
    char buf[32] = {0};
    struct tm timeinfo = {0};
    time_t temp = sec;
    localtime_r(&temp, &timeinfo);
    strftime(buf, sizeof(buf), "%m.%d.%Y %H:%M:%S", &timeinfo);
    DEBUG_PRINTF("sntp_set_system_time: %s\r\n", buf);
#endif // debug the time received

    g_timenow = sec;
}

uint32_t iot_sntp_get_time()
{
    return g_timenow;
}

#endif

