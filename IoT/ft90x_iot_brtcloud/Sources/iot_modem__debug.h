#ifndef SOURCES_IOT_MODEM__DEBUG_H_
#define SOURCES_IOT_MODEM__DEBUG_H_


#include "tinyprintf.h"

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {CRITICAL_SECTION_BEGIN;tfp_printf(__VA_ARGS__);CRITICAL_SECTION_END;} while (0)
#else
#define DEBUG_PRINTF(...)
#endif



#endif /* SOURCES_IOT_MODEM__DEBUG_H_ */
