#include <ft900.h>

#include "tinyprintf.h"     // For tfp_printf
#include "lwip/apps/mqtt.h" // For MQTT_TLS_PORT
#include "FreeRTOS.h"       // For pvPortMalloc
#include "iot.h"            // For USE_MQTT_BROKER



#if (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
void iot_sntp_start()
{
    char* sntp_servers[] = {"pool.ntp.org", "time.windows.com", "time.google.com"};
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
    } while (sntp_ip.addr == 0);
    sntp_init();
}

void iot_sntp_stop()
{
    sntp_stop();
}

static time_t g_timenow = 0;

void iot_sntp_set_system_time(uint32_t sec)
{
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

