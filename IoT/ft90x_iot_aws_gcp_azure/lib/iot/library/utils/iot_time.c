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
#include "FreeRTOS.h"       // For pvPortMalloc
#include "../../include/iot/iot_utils.h"      // For USE_MQTT_BROKER

#if USE_PAYLOAD_TIMESTAMP
#include "ext_rtc.h"        // For ext_rtc_xxx
#endif //USE_PAYLOAD_TIMESTAMP



//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



#if USE_PAYLOAD_TIMESTAMP
// Uses external RTC (for Rev A and B)
static int iot_rtc_set_time_ext(uint32_t secs)
{
    // Disable all interrupts
    ext_rtc_disable_interrupt(ALL_INT);

    // Initialize  RTC
    ext_rtc_init(NULL);

    // Additional input for time
    time_t time = secs;
    struct tm *time_tm = localtime(&time);
    DEBUG_PRINTF("secs = %u\r\n", secs);
    DEBUG_PRINTF("set %04d-%02d-%02d %02d:%02d:%02d %d\r\n",
        time_tm->tm_year, time_tm->tm_mon, time_tm->tm_mday,
        time_tm->tm_hour, time_tm->tm_min, time_tm->tm_sec, time_tm->tm_wday);

    // Set the time to RTC
    ext_rtc_time_t time_rtc = {0};
    time_rtc.year      = (uint8_t)(time_tm->tm_year-100);
    time_rtc.month     = (uint8_t)time_tm->tm_mon+1;
    time_rtc.date      = (uint8_t)time_tm->tm_mday;
    time_rtc.day       = (uint8_t)time_tm->tm_wday;
    time_rtc.hour      = (uint8_t)time_tm->tm_hour;
    time_rtc.min       = (uint8_t)time_tm->tm_min;
    time_rtc.sec       = (uint8_t)time_tm->tm_sec;
    time_rtc.fmt_12_24 = HOUR_FORMAT_24;
    time_rtc.AM_PM     = AM_HOUR_FORMAT;

    if (ext_rtc_write(&time_rtc)) {
        DEBUG_PRINTF("iot_rtc_set_time_ext failed! ext_rtc_write\r\n");
        return 0;
    }

    return 1;
}

// Uses external RTC (for Rev A and B)
static int iot_rtc_get_time_ext(struct tm* time_tm)
{
    ext_rtc_time_t time;

    // Get the time from RTC
    if (ext_rtc_read(&time)) {
        DEBUG_PRINTF("iot_rtc_get_time_ext failed! ext_rtc_read\r\n");
        return 0;
    }

    time_tm->tm_year = time.year+100;
    time_tm->tm_mon  = time.month-1;
    time_tm->tm_mday = time.date;
    time_tm->tm_wday = time.day;
    time_tm->tm_hour = time.hour;
    time_tm->tm_min  = time.min;
    time_tm->tm_sec  = time.sec;

    DEBUG_PRINTF("get1 %04d-%02d-%02d %02d:%02d:%02d %d\r\n",
        time.year, time.month, time.date,
        time.hour, time.min, time.sec, time.day);
    DEBUG_PRINTF("get2 %04d-%02d-%02d %02d:%02d:%02d %d\r\n",
        time_tm->tm_year, time_tm->tm_mon, time_tm->tm_mday,
        time_tm->tm_hour, time_tm->tm_min, time_tm->tm_sec, time_tm->tm_wday);

    return 1;
}


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

// Uses internal RTC (for Rev C)
static int iot_rtc_get_time(struct tm* time_tm)
{
    // Get the time from RTC
    if (rtc_read(time_tm)) {
        DEBUG_PRINTF("iot_rtc_get_time failed! rtc_read\r\n");
        return 0;
    }

    DEBUG_PRINTF("get2 %04d-%02d-%02d %02d:%02d:%02d %d\r\n",
        time_tm->tm_year, time_tm->tm_mon, time_tm->tm_mday,
        time_tm->tm_hour, time_tm->tm_min, time_tm->tm_sec, time_tm->tm_wday);

    return 1;
}


int64_t iot_utils_gettimeepoch()
{
    int64_t time_epoch = 0;
    struct tm time_tm = {0};

    // if 90x series is NOT rev C, use external RTC
    // Otherwise if using rev C, use internal RTC
    if (sys_check_ft900_revB()) {
        if (!iot_rtc_get_time_ext(&time_tm)) {
            DEBUG_PRINTF("iot_utils_gettimeepoch failed! iot_rtc_get_time_ext\r\n");
            return time_epoch;
        }
    }
    else {
        if (!iot_rtc_get_time(&time_tm)) {
            DEBUG_PRINTF("iot_utils_gettimeepoch failed! iot_rtc_get_time\r\n");
            return time_epoch;
        }
    }

    time_epoch = mktime(&time_tm);
    DEBUG_PRINTF("time_epoch = %u\r\n", time_epoch);
    return time_epoch*1000;
}

const char* iot_utils_gettimeiso(int format)
{
    static char timeStampIso[24] = {0};
    struct tm time_tm = {0};

    // if 90x series is NOT rev C, use external RTC
    // Otherwise if using rev C, use internal RTC
    if (sys_check_ft900_revB()) {
        if (!iot_rtc_get_time_ext(&time_tm)) {
            DEBUG_PRINTF("iot_utils_gettimeiso failed! iot_rtc_get_time_ext\r\n");
            return NULL;
        }
    }
    else {
        if (!iot_rtc_get_time(&time_tm)) {
            DEBUG_PRINTF("iot_utils_gettimeiso failed! iot_rtc_get_time\r\n");
            return NULL;
        }
    }

    switch (format) {
    case 0: // YYYY-MM-DDTHH:mm:SS.000
        tfp_snprintf(timeStampIso, sizeof(timeStampIso),
            "%04d-%02d-%02dT%02d:%02d:%02d.000",
            time_tm.tm_year+1900,
            time_tm.tm_mon+1,
            time_tm.tm_mday,
            time_tm.tm_hour,
            time_tm.tm_min,
            time_tm.tm_sec);
        break;
    case 1: // YYYYMMDDHH:mm:SS
        tfp_snprintf(timeStampIso, sizeof(timeStampIso),
            "%04d%02d%02d%02d:%02d:%02d",
            time_tm.tm_year+1900,
            time_tm.tm_mon+1,
            time_tm.tm_mday,
            time_tm.tm_hour,
            time_tm.tm_min,
            time_tm.tm_sec);
        break;
    }

    return (const char*)timeStampIso;
}
#endif // USE_PAYLOAD_TIMESTAMP



#if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT) || USE_PAYLOAD_TIMESTAMP

static time_t g_timenow = 0;

void iot_sntp_start()
{
    char* sntp_servers[] = {"time.google.com", "time.windows.com"};
    int sntp_servers_count = sizeof(sntp_servers)/sizeof(sntp_servers[0]);
    ip_addr_t sntp_ip = {0};
    struct hostent *host= NULL;

    do {
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
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
        // if 90x series is NOT rev C, use external RTC
        // Otherwise if using rev C, use internal RTC
        if (sys_check_ft900_revB()) { 
            iot_rtc_set_time_ext(sec);
        }
        else {
            iot_rtc_set_time(sec);
        }
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

