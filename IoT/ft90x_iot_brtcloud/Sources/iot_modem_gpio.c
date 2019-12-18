#include <ft900.h>
#include "tinyprintf.h"

/* FreeRTOS Headers. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

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



extern TaskHandle_t g_iot_app_handle; // used by iot_modem_gpio_process()
extern iot_handle g_handle;           // used to publish packets
extern GPIO_PROPERTIES g_oGpioProperties[GPIO_COUNT];
extern uint8_t g_ucGpioEnabled[GPIO_COUNT];

static uint8_t g_aucGpioIndex[GPIO_COUNT] = { 0, 1, 2, 3 };
static TimerHandle_t g_oGpioTimer[GPIO_COUNT] = { NULL, NULL, NULL, NULL };
static void ISR_gpio();





void iot_modem_gpio_init(int voltage)
{
    // Initialize the GPIO Input pins
    for (int i=0; i<GPIO_COUNT; i++) {
        gpio_dir(GPIO_INPUT_PIN_0 + i, pad_dir_input);
    }

    // Initialize the GPIO Output pins
    for (int i=0; i<GPIO_COUNT; i++) {
        gpio_dir(GPIO_OUTPUT_ENABLE_PIN_0 + i, pad_dir_output);

        // disable output enable pins at initialization
        gpio_write(GPIO_OUTPUT_ENABLE_PIN_0 + i, 0);

        if (i < 3) {
            gpio_dir(GPIO_OUTPUT_PIN_0 + i, pad_dir_output);
        }
        else { // i==3
            gpio_dir(GPIO_OUTPUT_PIN_3, pad_dir_output);
        }
    }

    // Initialize the GPIO voltage pins
    gpio_dir(GPIO_VOLTAGE_PIN_0, pad_dir_output);
    gpio_dir(GPIO_VOLTAGE_PIN_1, pad_dir_output);
    iot_modem_gpio_set_voltage(voltage);
}

void iot_modem_gpio_enable_interrupt()
{
    interrupt_attach(interrupt_gpio, (uint8_t)interrupt_gpio, ISR_gpio);
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

void iot_modem_gpio_process(int number)
{
    // This is called by the main task after receiving notification from timer
    char topic[MQTT_MAX_TOPIC_SIZE] = {0};
    char payload[3] = {0};

    tfp_snprintf( topic, sizeof(topic), TOPIC_GPIO, PREPEND_REPLY_TOPIC, iot_utils_getdeviceid(), number, MENOS_DEFAULT);
       tfp_snprintf( payload, sizeof(payload), PAYLOAD_EMPTY);
    iot_publish( g_handle, topic, payload, strlen(payload), 1 );
    DEBUG_PRINTF("PUB %s (%d) %s (%d)\r\n\r\n", topic, strlen(topic), payload, strlen(payload));
}

typedef struct tmrTimerControl /* The old naming convention is used to prevent breaking kernel aware debuggers. */
{
    const char                *pcTimerName;        /*<< Text name.  This is not used by the kernel, it is included simply to make debugging easier. */ /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
    ListItem_t                xTimerListItem;        /*<< Standard linked list item as used by all kernel features for event management. */
    TickType_t                xTimerPeriodInTicks;/*<< How quickly and often the timer expires. */
    void                     *pvTimerID;            /*<< An ID to identify the timer.  This allows the timer to be identified when the same callback is used for multiple timers. */
    TimerCallbackFunction_t    pxCallbackFunction;    /*<< The function that will be called when the timer expires. */
    #if( configUSE_TRACE_FACILITY == 1 )
        UBaseType_t            uxTimerNumber;        /*<< An ID assigned by trace tools such as FreeRTOS+Trace */
    #endif
    uint8_t                 ucStatus;            /*<< Holds bits to say if the timer was statically allocated or not, and if it is active or not. */
} xTIMER;

static void gpio_timer( TimerHandle_t xTimer )
{
    // Inform the main task to publish the notification packet
    uint8_t index = *((uint8_t*)xTimer->pvTimerID);

    DEBUG("GPIO timer %d\r\n", index);

    uint8_t status = gpio_read(GPIO_INPUT_PIN_0 + index);
    if (g_oGpioProperties[index].m_ucMode == GPIO_MODES_INPUT_HIGH_LEVEL ||
    	g_oGpioProperties[index].m_ucMode == GPIO_MODES_INPUT_HIGH_EDGE) {
    	if (status) {
    		// Set bit for GPIO X and bit for activation
    	    xTaskNotify(g_iot_app_handle, TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_GPIO0 + index) | TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_ACTIVATION), eSetBits);
    	}
    	else {
    		// Set bit for GPIO X and bit for deactivation
    	    xTaskNotify(g_iot_app_handle, TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_GPIO0 + index) | TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_DEACTIVATION), eSetBits);
    	}
    }
    else if (g_oGpioProperties[index].m_ucMode == GPIO_MODES_INPUT_LOW_LEVEL ||
      		 g_oGpioProperties[index].m_ucMode == GPIO_MODES_INPUT_LOW_EDGE) {
    	if (!status) {
    		// Set bit for GPIO X and bit for activation
    	    xTaskNotify(g_iot_app_handle, TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_GPIO0 + index) | TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_ACTIVATION), eSetBits);
    	}
    	else {
    		// Set bit for GPIO X and bit for deactivation
    	    xTaskNotify(g_iot_app_handle, TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_GPIO0 + index) | TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_DEACTIVATION), eSetBits);
    	}
    }
}

static inline void gpio_create_timer(int index)
{
    // Create the timer given the alert type and alert period
    char acTimerName[8] = {0};
    tfp_snprintf( acTimerName, sizeof(acTimerName), "GPIO%d", index );

    g_oGpioTimer[index] = xTimerCreate(acTimerName,
        pdMS_TO_TICKS(g_oGpioProperties[index].m_ulAlertperiod),
        (BaseType_t)g_oGpioProperties[index].m_ucAlert,
        &g_aucGpioIndex[index],
        gpio_timer
        );
}

static inline void gpio_create_timer_or_interrupt(int index, uint8_t pin, gpio_int_edge_t edge)
{
    if (edge == gpio_int_edge_raising) {
        // If already high, start the timer
        // Else, run an ISR to wait until it goes high
        if (gpio_read(pin)) {
            DEBUG("GPIO timer %d ENABLE\r\n", index);
            gpio_create_timer(index);
            DEBUG("GPIO timer %d ENABLED\r\n", index);
        }
        else {
            gpio_interrupt_enable(pin, edge);
        }
    }
    else {
        // If already low, start the timer
        // Else, run an ISR to wait until it goes low
        if (gpio_read(pin) == 0) {
            DEBUG("GPIO timer %d ENABLE\r\n", index);
            gpio_create_timer(index);
            DEBUG("GPIO timer %d ENABLED\r\n", index);
        }
        else {
            gpio_interrupt_enable(pin, edge);
        }
    }
}

int iot_modem_gpio_enable(GPIO_PROPERTIES* properties, int index, int enable)
{
    uint8_t pin = 0;
    uint8_t control_pin = GPIO_OUTPUT_ENABLE_PIN_0 + index;

    if (properties->m_ucDirection == pad_dir_input) {
        // If INPUT pin, use GPIO_INPUT_PIN_0 as base address
        pin = GPIO_INPUT_PIN_0 + index;

        if (enable) {
            if (properties->m_ulAlertperiod == 0) {
                // invalid parameter
                return 0;
            }

            // disable the output enable pin
            gpio_write(control_pin, 0);

            DEBUG("GPIO %d ENABLE\r\n", index);
            // HIGH LEVEL/EDGE
            if (properties->m_ucMode == GPIO_MODES_INPUT_HIGH_LEVEL ||
                properties->m_ucMode == GPIO_MODES_INPUT_HIGH_EDGE) {
                gpio_create_timer_or_interrupt(index, pin, gpio_int_edge_raising);
            }
            // LOW LEVEL/EDGE
            else if (properties->m_ucMode == GPIO_MODES_INPUT_LOW_LEVEL ||
                     properties->m_ucMode == GPIO_MODES_INPUT_LOW_EDGE) {
                gpio_create_timer_or_interrupt(index, pin, gpio_int_edge_falling);
            }
            DEBUG("GPIO %d ENABLED\r\n", index);
        }
        else {
            DEBUG("GPIO timer %d DISABLE\r\n", index);
            gpio_interrupt_disable(pin);
            if (g_oGpioTimer[index] != NULL) {
                if ( xTimerDelete( g_oGpioTimer[index], pdMS_TO_TICKS(1000)) != pdTRUE ) {
                    DEBUG_PRINTF( "xTimerDelete GPIO %d failed\r\n", index );
                }
                g_oGpioTimer[index] = NULL;
                DEBUG("GPIO timer %d DISABLED\r\n", index);
            }
        }
    }
    else {

        // If INPUT pin, use GPIO_INPUT_PIN_0 as base address
        pin = GPIO_OUTPUT_PIN_0 + index;
        if (index == 3) {
            pin = GPIO_OUTPUT_PIN_3;
        }

        if (enable) {
            // ENABLE
            // enable the output enable pin
            gpio_write(control_pin, 1);

        	// TODO
        }
        else {
        }
    }

    return 1;
}

void iot_modem_gpio_get_status(uint8_t* status, GPIO_PROPERTIES* properties, uint8_t* enabled)
{
    for (int i=0; i<GPIO_COUNT; i++) {
        if (properties[i].m_ucDirection == pad_dir_input) {
        	// input
            status[i] = gpio_read(GPIO_INPUT_PIN_0 + i);
        }
        else {
        	// output
            if (!enabled[i]) {
                // if output and disabled, read gpio status
            	if (i<3) {
            		status[i] = gpio_read(GPIO_OUTPUT_PIN_0 + i);
            	}
            	else {
            		status[i] = gpio_read(GPIO_OUTPUT_PIN_3);
            	}
            }
            else {
                // if output and enabled, just set to 0
                status[i] = 0;
            }
        }
    }
}

static void ISR_gpio()
{
    uint8_t pin = 0;

    for (int i=0; i<GPIO_COUNT; i++) {
        if (g_ucGpioEnabled[i] && !g_oGpioTimer[i]) {
            // INPUT direction
            if (g_oGpioProperties[i].m_ucDirection == pad_dir_input) {

            	pin = GPIO_INPUT_PIN_0 + i;

                // For HIGH-LEVEL and LOW-LEVEL
                if (g_oGpioProperties[i].m_ucMode == GPIO_MODES_INPUT_HIGH_LEVEL) {
                    if (gpio_read(pin+i) != 0) {
                        // debounce
                        delayms(5);
                        if (gpio_read(pin+i) != 0) {
                            gpio_interrupt_disable(pin+i);
                            gpio_create_timer(i);
                        }
                    }
                }
                else if (g_oGpioProperties[i].m_ucMode == GPIO_MODES_INPUT_LOW_LEVEL) {
                    if (gpio_read(pin+i) == 0) {
                        // debounce
                        delayms(5);
                        if (gpio_read(pin+i) == 0) {
                            gpio_interrupt_disable(pin+i);
                            gpio_create_timer(i);
                        }
                    }
                }
                // For HIGH-EDGE and LOW-EDGE
                else if (g_oGpioProperties[i].m_ucMode >= GPIO_MODES_INPUT_HIGH_EDGE) {
                    if (gpio_is_interrupted(pin+1)) {
                        gpio_interrupt_disable(pin+1);
                        gpio_create_timer(i);
                    }
                }
            }
            // OUTPUT direction
            else if (g_oGpioProperties[i].m_ucDirection == pad_dir_output) {
            }
        }
    }
}


