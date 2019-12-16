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
#include "json.h"


#include <string.h>
#include <stdlib.h>





///////////////////////////////////////////////////////////////////////////////////
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {CRITICAL_SECTION_BEGIN;tfp_printf(__VA_ARGS__);CRITICAL_SECTION_END;} while (0)
#else
#define DEBUG_PRINTF(...)
#endif
///////////////////////////////////////////////////////////////////////////////////



extern GPIO_PROPERTIES g_oGpioProperties[GPIO_COUNT];
extern uint8_t g_oGpioEnabled[GPIO_COUNT];

extern TaskHandle_t g_iot_app_handle; // used by iot_modem_gpio_process()
extern iot_handle g_handle; // used to publish packets




void iot_modem_gpio_init(int voltage)
{
	// Initialize the GPIO Input pins
	for (int i=0; i<GPIO_COUNT; i++) {
		gpio_dir(GPIO_INPUT_PIN_0 + i, pad_dir_input);
	    //gpio_pull(GPIO_INPUT_PIN_0 + i, pad_pull_pullup);
	}

	// Initialize the GPIO Output pins
	for (int i=0; i<GPIO_COUNT; i++) {
		gpio_dir(GPIO_OUTPUT_PIN_0 + i, pad_dir_output);
	    //gpio_pull(GPIO_OUTPUT_PIN_0 + i, pad_pull_none);
	}

	// Initialize the GPIO voltage pins
	gpio_dir(GPIO_VOLTAGE_PIN_0, pad_dir_output);
	gpio_dir(GPIO_VOLTAGE_PIN_1, pad_dir_output);
	iot_modem_gpio_set_voltage(voltage);
}

void iot_modem_gpio_set_voltage(int voltage)
{
	if (!voltage) {
		// Set to 3.3 V
		gpio_write(GPIO_VOLTAGE_PIN_0, 0);
		gpio_write(GPIO_VOLTAGE_PIN_1, 1);
	}
	else {
		// Set to 5 V
		gpio_write(GPIO_VOLTAGE_PIN_0, 1);
		gpio_write(GPIO_VOLTAGE_PIN_1, 0);
	}
}

void iot_modem_gpio_enable(GPIO_PROPERTIES* properties, int number, int enable)
{
	uint8_t pin = /* TODO OFFSET plus NUMBER */ number;

	// INPUT
	if (properties->m_ucDirection == 0) {
		// HIGH-LEVEL, LOW-LEVEL
		if (properties->m_ucMode == GPIO_MODES_INPUT_HIGH_LEVEL) {
			if (enable) {

			}
			else {

			}
		}
		// LOW-LEVEL
		else if (properties->m_ucMode == GPIO_MODES_INPUT_LOW_LEVEL) {
			if (enable) {

			}
			else {

			}
		}
		// HIGH-EDGE, LOW-EDGE
		else if (properties->m_ucMode == GPIO_MODES_INPUT_HIGH_EDGE) {
			if (enable) {
				gpio_interrupt_enable(pin, gpio_int_edge_falling);
			}
			else {
				gpio_interrupt_disable(pin);
			}
		}
		else if (properties->m_ucMode == GPIO_MODES_INPUT_LOW_EDGE) {
			if (enable) {
				gpio_interrupt_enable(pin, gpio_int_edge_raising);
			}
			else {
				gpio_interrupt_disable(pin);
			}
		}
	}
	// OUTPUT
	else {
		if (enable) {
			// ENABLE
		}
		else {
			// DISABLE
		}
	}
}

void iot_modem_gpio_get_status(uint8_t* status, uint8_t* direction, uint8_t* enabled)
{
	for (int i=0; i<GPIO_COUNT; i++) {

		if (direction[i] == 0) { // input
			status[i] = gpio_read(GPIO_INPUT_PIN_0 + i);
		}
		else { // output
			if (enabled[i] == 0) { // disabled
				status[i] = gpio_read(GPIO_OUTPUT_PIN_0 + i);
			}
			else { // enabled
				// if output and enabled, just set to 0
				status[i] = 0;
			}
		}
	}
}

void ISR_gpio()
{
	uint8_t pin = /* TODO base */ 0;

	for (int i=0; i<GPIO_COUNT; i++) {
		if (g_oGpioEnabled[i]) {
			// INPUT direction and HIGH-EDGE/LOW-EDGE
			if (g_oGpioProperties[i].m_ucDirection == pad_dir_input) {
				// HIGH-EDGE/LOW-EDGE
				if (g_oGpioProperties[i].m_ucMode == GPIO_MODES_INPUT_HIGH_LEVEL) {
					if (gpio_read(pin+i /*TODO*/) != 0) {
						xTaskNotifyFromISR(g_iot_app_handle, TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_GPIO0), eSetBits, NULL);
					}
				}
				else if (g_oGpioProperties[i].m_ucMode == GPIO_MODES_INPUT_LOW_LEVEL) {
					if (gpio_read(pin+i /*TODO*/) == 0) {
						xTaskNotifyFromISR(g_iot_app_handle, TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_GPIO1), eSetBits, NULL);
					}
				}
				else if (g_oGpioProperties[i].m_ucMode == GPIO_MODES_INPUT_HIGH_EDGE) {
					if (gpio_is_interrupted(pin+i /*TODO*/)) {
						xTaskNotifyFromISR(g_iot_app_handle, TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_GPIO2), eSetBits, NULL);
					}
				}
				else if (g_oGpioProperties[i].m_ucMode == GPIO_MODES_INPUT_LOW_EDGE) {
					if (gpio_is_interrupted(pin+i /*TODO*/)) {
						xTaskNotifyFromISR(g_iot_app_handle, TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_GPIO3), eSetBits, NULL);
					}
				}
			}
			// OUTPUT direction
			else if (g_oGpioProperties[i].m_ucDirection == pad_dir_output) {

			}
		}
	}
}

void iot_modem_gpio_enable_interrupt()
{
	interrupt_attach(interrupt_gpio, (uint8_t)interrupt_gpio, ISR_gpio);
}

void iot_modem_gpio_process(int number)
{
    char topic[MQTT_MAX_TOPIC_SIZE] = {0};
    char payload[3] = {0};
    tfp_snprintf( topic, sizeof(topic), TOPIC_GPIO, PREPEND_REPLY_TOPIC, iot_utils_getdeviceid(), number, MENOS_DEFAULT);
   	tfp_snprintf( payload, sizeof(payload), PAYLOAD_EMPTY);
    iot_publish( g_handle, topic, payload, strlen(payload), 1 );
    DEBUG_PRINTF("PUB %s (%d) %s (%d)\r\n\r\n", topic, strlen(topic), payload, strlen(payload));
}

