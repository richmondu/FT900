#include <ft900.h>
#include "tinyprintf.h"

/* FreeRTOS Headers. */
#include "FreeRTOS.h"
#include "task.h"

/* netif Abstraction Header. */
#include "net.h"

/* IOT Headers. */
#include <iot_config.h>
#include "iot/iot.h"
#include "iot/iot_utils.h"

/* IoT Modem */
#include "iot_modem.h"
#include "iot_modem__debug.h"
#include "json.h"


#include <string.h>
#include <stdlib.h>





#if ENABLE_ADC

void iot_modem_adc_init(int voltage)
{
    DEBUG_PRINTF( "ADC INIT\r\n" );

    // MODIFY ME


    iot_modem_adc_set_voltage(voltage);
}

int iot_modem_adc_enable(DEVICE_PROPERTIES* properties)
{
    DEBUG_PRINTF( "ADC ENABLE\r\n" );

    // MODIFY ME

    if (properties->m_ucEnabled) {

    }
    else {

    }

    return 1;
}

void iot_modem_adc_set_properties(DEVICE_PROPERTIES* properties)
{
    DEBUG_PRINTF( "ADC SETPROP\r\n" );

    // MODIFY ME

    if (properties->m_ucClass == DEVICE_CLASS_ANENOMOMETER) {

    }
}

uint32_t iot_modem_adc_get_sensor_reading(DEVICE_PROPERTIES* properties)
{
    // MODIFY ME

    return 1;
}

void iot_modem_adc_set_voltage(int voltage)
{
    // MODIFY ME - REFER TO ADC_VOLTAGE

	if (voltage == ADC_VOLTAGE_N5_P5) {

	}
	else if (voltage == ADC_VOLTAGE_N10_P10) {

	}
	else if (voltage == ADC_VOLTAGE_0_P10) {

	}
}

#endif // ENABLE_ADC
