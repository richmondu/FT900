#include <ft900.h>

#include "tinyprintf.h"     // For tfp_printf
#include "lwip/apps/mqtt.h" // For MQTT_TLS_PORT
#include "FreeRTOS.h"       // For pvPortMalloc
#include "iot.h"            // For USE_MQTT_BROKER

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
static int iot_rtc_set_time(uint32_t secs)
{
    /* Disable all interrupts */
    ext_rtc_disable_interrupt(ALL_INT);

    /* Initialize  RTC */
    ext_rtc_init(NULL);

    /* additional input for time */
    time_t time = secs;
    struct tm *time_tm = localtime(&time);
    DEBUG_PRINTF("secs = %u\r\n", secs);
    DEBUG_PRINTF("set %04d-%02d-%02d %02d:%02d:%02d %d\r\n",
        time_tm->tm_year, time_tm->tm_mon, time_tm->tm_mday,
        time_tm->tm_hour, time_tm->tm_min, time_tm->tm_sec, time_tm->tm_wday);

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
    if (ext_rtc_write(time_rtc)) {
        DEBUG_PRINTF("iot_rtc_set_time failed! ext_rtc_write\r\n");
        return 0;
    }

    return 1;
}

static int iot_rtc_get_time(struct tm* time_tm)
{
    ext_rtc_time_t time;

    if (ext_rtc_read(&time)) {
        DEBUG_PRINTF("iot_rtc_get_time failed! ext_rtc_read\r\n");
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

int64_t iot_rtc_get_time_epoch()
{
    int64_t time_epoch = 0;
    struct tm time_tm = {0};

    if (!iot_rtc_get_time(&time_tm)) {
        DEBUG_PRINTF("iot_rtc_get_time_epoch failed! iot_rtc_get_time\r\n");
        return time_epoch;
    }

    time_epoch = mktime(&time_tm);
    DEBUG_PRINTF("time_epoch = %u\r\n", time_epoch);
    return time_epoch*1000;
}

const char* iot_rtc_get_time_iso(int format)
{
    static char timeStampIso[24] = {0};
    struct tm time_tm = {0};


    if (!iot_rtc_get_time(&time_tm)) {
        DEBUG_PRINTF("iot_rtc_get_time_iso failed! iot_rtc_get_time\r\n");
        return NULL;
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
        iot_rtc_set_time(sec);
    }
#endif

#if 0
    char buf[32] = {0};
    struct tm timeinfo = {0};
    time_t temp = sec;
    localtime_r(&temp, &timeinfo);
    strftime(buf, sizeof(buf), "%m.%d.%Y %H:%M:%S", &timeinfo);
    DEBUG_PRINTF("sntp_set_system_time: %s\r\n", buf);
#endif
    g_timenow = sec;
}

uint32_t iot_sntp_get_time()
{
    return g_timenow;
}

#endif

