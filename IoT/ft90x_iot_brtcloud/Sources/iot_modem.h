#ifndef _IOT_MODEM_H_
#define _IOT_MODEM_H_



#define ENABLE_UART 1
#define ENABLE_GPIO 0
#define ENABLE_I2C  0


////////////////////////////////////////////////////////////////////////////////////
// Firmware version
////////////////////////////////////////////////////////////////////////////////////
#define VERSION_MAJOR 0
#define VERSION_MINOR 1
//#define VERSION (VERSION_MAJOR*100 + VERSION_MINOR)
//#define VERSION_STR "{}.{}".format(VERSION_MAJOR, VERSION_MINOR)


////////////////////////////////////////////////////////////////////////////////////
// Device status
////////////////////////////////////////////////////////////////////////////////////

typedef enum _DEVICE_STATUS {
	DEVICE_STATUS_STARTING,
	DEVICE_STATUS_RUNNING,
	DEVICE_STATUS_RESTART,
	DEVICE_STATUS_RESTARTING,
	DEVICE_STATUS_STOP,
	DEVICE_STATUS_STOPPING,
	DEVICE_STATUS_STOPPED,
	DEVICE_STATUS_START,
} DEVICE_STATUS;


////////////////////////////////////////////////////////////////////////////////////
// APIs
////////////////////////////////////////////////////////////////////////////////////

// device status
#define API_GET_STATUS                "get_status"
#define API_SET_STATUS                "set_status"

// uart
#if ENABLE_UART
#define API_GET_UARTS                 "get_uarts"
#define API_GET_UART_PROPERTIES       "get_uart_prop"
#define API_SET_UART_PROPERTIES       "set_uart_prop"
#define API_ENABLE_UART               "enable_uart"
#endif // ENABLE_UART

// gpio
#if ENABLE_GPIO
#define API_GET_GPIOS                 "get_gpios"
#define API_GET_GPIO_PROPERTIES       "get_gpio_prop"
#define API_SET_GPIO_PROPERTIES       "set_gpio_prop"
#define API_ENABLE_GPIO               "enable_gpio"
#define API_GET_GPIO_VOLTAGE          "get_gpio_voltage"
#define API_SET_GPIO_VOLTAGE          "set_gpio_voltage"
#endif // ENABLE_UART

// i2c
#if ENABLE_I2C
#define API_GET_I2CS                  "get_i2cs"
#define API_GET_I2C_DEVICE_PROPERTIES "get_i2c_dev_prop"
#define API_SET_I2C_DEVICE_PROPERTIES "set_i2c_dev_prop"
#define API_ENABLE_I2C                "enable_i2c"
#endif // ENABLE_I2C

// notification
#define API_TRIGGER_NOTIFICATION      "trigger_notification"


////////////////////////////////////////////////////////////////////////////////////
// Publish payload strings
////////////////////////////////////////////////////////////////////////////////////

#define PAYLOAD_EMPTY                   "{}"
#define PAYLOAD_API_GET_STATUS          "{\"value\":{\"status\":%d,\"version\":\"%d.%d\"}}"
#define PAYLOAD_API_SET_STATUS          "{\"value\":{\"status\":%d}}"

#if ENABLE_UART
#define PAYLOAD_API_GET_UARTS           "{\"value\":{\"uarts\":[{\"enabled\":%d}]}}"
#define PAYLOAD_API_GET_UART_PROPERTIES "{\"value\":{\"baudrate\":%d,\"parity\":%d,\"flowcontrol\":%d,\"stopbits\":%d,\"databits\":%d}}"
#endif // ENABLE_UART

#if ENABLE_GPIO
#define PAYLOAD_API_GET_GPIOS           "{\"value\":{\"gpios\":[{\"enabled\":%d,\"direction\":%d,\"status\":%d},{\"enabled\":%d,\"direction\":%d,\"status\":%d},{\"enabled\":%d,\"direction\":%d,\"status\":%d},{\"enabled\":%d,\"direction\":%d,\"status\":%d}]}}"
#define PAYLOAD_API_GET_GPIO_VOLTAGE    "{\"value\":{\"voltage\":%d}}"
#define PAYLOAD_API_GET_GPIO_PROPERTIES "{\"value\":{\"direction\":%d,\"mode\":%d,\"alert\":%d,\"alertperiod\":%d,\"polarity\":%d,\"width\":%d,\"mark\":%d,\"space\":%d}}"
#endif // ENABLE_GPIO

#if ENABLE_I2C
#define PAYLOAD_API_GET_I2CS            "{\"value\":{\"i2cs\":[{\"enabled\":%d},{\"enabled\":%d},{\"enabled\":%d},{\"enabled\":%d}]}}"
#endif // ENABLE_I2C


#endif /* _IOT_MODEM_H_ */
