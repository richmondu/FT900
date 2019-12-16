#ifndef _IOT_MODEM_H_
#define _IOT_MODEM_H_



#define ENABLE_UART 1
#if ENABLE_UART
#define ENABLE_UART_ATCOMMANDS 1
#endif // ENABLE_UART
#define ENABLE_GPIO 1
#define ENABLE_I2C  0
#define ENABLE_NOTIFICATIONS 1



#define PREPEND_REPLY_TOPIC "server/"
#define WRONG_SYNTAX "wrong syntax"



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
#if ENABLE_NOTIFICATIONS
#define API_TRIGGER_NOTIFICATION      "trigger_notification"
#define API_RECEIVE_NOTIFICATION      "recv_notification"
#endif // ENABLE_NOTIFICATIONS


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
#define PAYLOAD_API_GET_GPIOS           "{\"value\":{\"voltage\":%d,\"gpios\":[{\"enabled\":%d,\"direction\":%d,\"status\":%d},{\"enabled\":%d,\"direction\":%d,\"status\":%d},{\"enabled\":%d,\"direction\":%d,\"status\":%d},{\"enabled\":%d,\"direction\":%d,\"status\":%d}]}}"
#define PAYLOAD_API_GET_GPIO_VOLTAGE    "{\"value\":{\"voltage\":%d}}"
#define PAYLOAD_API_GET_GPIO_PROPERTIES "{\"value\":{\"direction\":%d,\"mode\":%d,\"alert\":%d,\"alertperiod\":%d,\"polarity\":%d,\"width\":%d,\"mark\":%d,\"space\":%d}}"
#endif // ENABLE_GPIO

#if ENABLE_I2C
#define PAYLOAD_API_GET_I2CS            "{\"value\":{\"i2cs\":[{\"enabled\":%d},{\"enabled\":%d},{\"enabled\":%d},{\"enabled\":%d}]}}"
#endif // ENABLE_I2C


////////////////////////////////////////////////////////////////////////////////////
// UART Commands
////////////////////////////////////////////////////////////////////////////////////

#define UART_ATCOMMANDS_NUM 15

#define UART_ATCOMMAND_MOBILE        "AT+M"
#define UART_ATCOMMAND_EMAIL         "AT+E"
#define UART_ATCOMMAND_NOTIFY        "AT+N"
#define UART_ATCOMMAND_MODEM         "AT+O"
#define UART_ATCOMMAND_STORAGE       "AT+S"
#define UART_ATCOMMAND_DEFAULT       "AT+D"
#define UART_ATCOMMAND_CONTINUE      "ATC"
#define UART_ATCOMMAND_ECHO          "ATE"
#define UART_ATCOMMAND_HELP          "ATH"
#define UART_ATCOMMAND_INFO          "ATI"
#define UART_ATCOMMAND_MORE          "ATM"
#define UART_ATCOMMAND_PAUSE         "ATP"
#define UART_ATCOMMAND_RESET         "ATR"
#define UART_ATCOMMAND_UPDATE        "ATU"
#define UART_ATCOMMAND_STATUS        "AT"

#define UART_ATCOMMAND_DESC_MOBILE   "Send message as SMS to verified mobile number"
#define UART_ATCOMMAND_DESC_EMAIL    "Send message as email to verified email address"
#define UART_ATCOMMAND_DESC_NOTIFY   "Send message as mobile app notification to verified user"
#define UART_ATCOMMAND_DESC_MODEM    "Send message to other IoT modem devices"
#define UART_ATCOMMAND_DESC_STORAGE  "Send message to storage"
#define UART_ATCOMMAND_DESC_DEFAULT  "Send default message to configured endpoints"
#define UART_ATCOMMAND_DESC_CONTINUE "Continue device functions"
#define UART_ATCOMMAND_DESC_ECHO     "Echo on/off (toggle)"
#define UART_ATCOMMAND_DESC_HELP     "Display help information on commands"
#define UART_ATCOMMAND_DESC_INFO     "Display device information"
#define UART_ATCOMMAND_DESC_MORE     "Display more information on error"
#define UART_ATCOMMAND_DESC_PAUSE    "Pause/Resume (toggle)"
#define UART_ATCOMMAND_DESC_RESET    "Reset device"
#define UART_ATCOMMAND_DESC_UPDATE   "Enter firmware update (UART entry point inside bootloader)"
#define UART_ATCOMMAND_DESC_STATUS   "Display device status"

#define UART_ATCOMMAND_PLUS          '+'


#define UART_PROPERTIES_BAUDRATE_COUNT 16

typedef struct _UART_PROPERTIES {
	uint8_t m_ucBaudrate;    // g_uwBaudrates
	uint8_t m_ucParity;      // ft900_uart_simple.h uart_parity_t
	uint8_t m_ucFlowcontrol; // ft900_uart_simple.h uart_flow_t
	uint8_t m_ucStopbits;    // ft900_uart_simple.h uart_stop_bits_t
	uint8_t m_ucDatabits;    // ft900_uart_simple.h uart_data_bits_t
} UART_PROPERTIES;



typedef struct _GPIO_PROPERTIES {
    uint8_t  m_ucDirection;   // ["Input", "Output"]
    uint8_t  m_ucMode;        // ["High Level", "Low Level", "High Edge", "Low Edge"] , ["Level", "Clock", "Pulse"]
    uint8_t  m_ucAlert;       // ["Once", "Continuously"]
    uint32_t m_ulAlertperiod;
    uint8_t  m_ucPolarity;    // ["Positive", "Negative"]
    uint32_t m_ulWidth;
    uint32_t m_ulMark;
    uint32_t m_ulSpace;
} GPIO_PROPERTIES;

#define GPIO_INPUT_PIN_0   12 // Input pins: 12,13,14,15
#define GPIO_OUTPUT_PIN_0   8 // Output pins: 8,9,10,11
#define GPIO_VOLTAGE_PIN_0 16
#define GPIO_VOLTAGE_PIN_1 17



void iot_modem_uart_enable_interrupt();
void iot_modem_uart_enable(UART_PROPERTIES* properties, int enable, int disable);
void iot_modem_uart_command_process();
void iot_modem_uart_command_help();

void iot_modem_gpio_init(int voltage);
void iot_modem_gpio_set_voltage(int voltage);
void iot_modem_gpio_enable(GPIO_PROPERTIES* properties, int number, int enable);
void iot_modem_gpio_get_status(uint8_t* status, uint8_t* direction, uint8_t* enabled);



#endif /* _IOT_MODEM_H_ */
