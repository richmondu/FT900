#include <ft900.h>
#include <time.h>



void time_duration_setup()
{
    /* Enable the RTC... */
    sys_enable(sys_device_timer_wdt);

    /* Initialise the RTC to run indefinitely... */
    rtc_init();

    /* Initialize RTC time to 0 */
    /* We'll only use it for performance measurement */
    struct tm time = {0};
    rtc_write(&time);

    /* Start RTC */
    rtc_start();
}

void time_duration_get_time(struct tm* tm_time)
{
    rtc_read(tm_time);
}

long long time_duration_get(struct tm* tm_time1, struct tm* tm_time0)
{
    // handle cases for faster performance
    if (tm_time1->tm_hour == tm_time0->tm_hour &&
        tm_time1->tm_min == tm_time0->tm_min) {
        return tm_time1->tm_sec - tm_time0->tm_sec;
    }
    else if (tm_time1->tm_hour == tm_time0->tm_hour) {
        return (tm_time1->tm_min*60 + tm_time1->tm_sec) - (tm_time0->tm_min*60 + tm_time0->tm_sec);
    }
    else {
        long long ll_time_end   = tm_time1->tm_hour*3600 + tm_time1->tm_min*60 + tm_time1->tm_sec;
        long long ll_time_begin = tm_time0->tm_hour*3600 + tm_time0->tm_min*60 + tm_time0->tm_sec;
        return ll_time_end - ll_time_begin;
    }
}


