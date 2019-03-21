#ifndef TIME_DURATION_H
#define TIME_DURATION_H


void time_duration_setup();
void time_duration_get_time(struct tm* tm_time);
long long time_duration_get(struct tm* tm_time1, struct tm* tm_time0);


#endif // TIME_DURATION_H
