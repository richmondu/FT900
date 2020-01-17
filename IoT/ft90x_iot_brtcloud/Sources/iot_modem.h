#ifndef _IOT_MODEM_H_
#define _IOT_MODEM_H_



#define ENABLE_NOTIFICATIONS               0

#define ENABLE_UART                        0
#if ENABLE_UART
#define ENABLE_UART_ATCOMMANDS             1
#endif // ENABLE_UART
#define ENABLE_GPIO                        0

#define ENABLE_I2C                         0
#define ENABLE_ADC                         1
#define ENABLE_ONEWIRE                     1
#define ENABLE_TPROBE                      1



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
// MQTT BUFFER SIZES
////////////////////////////////////////////////////////////////////////////////////

#define MQTT_MAX_TOPIC_SIZE                64
#define MQTT_MAX_PAYLOAD_SIZE              256
#define PREPEND_REPLY_TOPIC                "server/"
#define WRONG_SYNTAX                       "wrong syntax"
#define TODO_STRING                        "todo"
#define NUMBER_STRING                      "number"
#define ENABLE_STRING                      "enable"
#define ENABLED_STRING                     "enabled"
#define STATUS_STRING                      "status"
#define VOLTAGE_STRING                     "voltage"


////////////////////////////////////////////////////////////////////////////////////
// Notifications
////////////////////////////////////////////////////////////////////////////////////

#define MENOS_MOBILE                       "mobile"
#define MENOS_EMAIL                        "email"
#define MENOS_NOTIFICATION                 "notification"
#define MENOS_MODEM                        "modem"
#define MENOS_STORAGE                      "storage"
#define MENOS_DEFAULT                      "default"

#define MENOS_MESSAGE                      "message"
#define MENOS_RECIPIENT                    "recipient"
#define MENOS_SENDER                       "sender"


////////////////////////////////////////////////////////////////////////////////////
// Task notify
////////////////////////////////////////////////////////////////////////////////////

typedef enum _TASK_NOTIFY_BIT {
    TASK_NOTIFY_BIT_UART,
    TASK_NOTIFY_BIT_GPIO0,
    TASK_NOTIFY_BIT_GPIO1,
    TASK_NOTIFY_BIT_GPIO2,
    TASK_NOTIFY_BIT_GPIO3,
    TASK_NOTIFY_BIT_I2C0,
    TASK_NOTIFY_BIT_I2C1,
    TASK_NOTIFY_BIT_I2C2,
    TASK_NOTIFY_BIT_I2C3,
    TASK_NOTIFY_BIT_ADC0,
    TASK_NOTIFY_BIT_ADC1,
    TASK_NOTIFY_BIT_1WIRE,
    TASK_NOTIFY_BIT_TPROBE,
    TASK_NOTIFY_BIT_ACTIVATION,
} TASK_NOTIFY_BIT;

#define TASK_NOTIFY_BIT(x)                 ( 1 << (x) )
#define TASK_NOTIFY_FROM_UART(y)           ( (y) & TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_UART) )
#define TASK_NOTIFY_FROM_GPIO(y,z)         ( (y) & TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_GPIO0+(z)) )
#define TASK_NOTIFY_FROM_I2C(y,z)          ( (y) & TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_I2C0+(z)) )
#define TASK_NOTIFY_FROM_ADC(y,z)          ( (y) & TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_ADC0+(z)) )
#define TASK_NOTIFY_FROM_1WIRE(y)          ( (y) & TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_1WIRE) )
#define TASK_NOTIFY_FROM_TPROBE(y)         ( (y) & TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_TPROBE) )
#define TASK_NOTIFY_ACTIVATION(y)          ( (y) & TASK_NOTIFY_BIT(TASK_NOTIFY_BIT_ACTIVATION) )
#define TASK_NOTIFY_CLEAR_BITS             0xFFFFFFFF

typedef enum _ALERT_TYPE {
    ALERT_TYPE_ONCE,
    ALERT_TYPE_CONTINUOUS,
} ALERT_TYPE;


////////////////////////////////////////////////////////////////////////////////////
// APIs
////////////////////////////////////////////////////////////////////////////////////

// device status
#define API_GET_STATUS                     "get_status"
#define API_SET_STATUS                     "set_status"

// uart
#if ENABLE_UART
#define API_GET_UARTS                      "get_uarts"
#define API_GET_UART_PROPERTIES            "get_uart_prop"
#define API_SET_UART_PROPERTIES            "set_uart_prop"
#define API_ENABLE_UART                    "enable_uart"
#endif // ENABLE_UART

// gpio
#if ENABLE_GPIO
#define API_GET_GPIOS                      "get_gpios"
#define API_GET_GPIO_PROPERTIES            "get_gpio_prop"
#define API_SET_GPIO_PROPERTIES            "set_gpio_prop"
#define API_ENABLE_GPIO                    "enable_gpio"
#define API_GET_GPIO_VOLTAGE               "get_gpio_voltage"
#define API_SET_GPIO_VOLTAGE               "set_gpio_voltage"
#endif // ENABLE_UART

// i2c
#if ENABLE_I2C
#define API_GET_I2C_DEVICES                "get_i2c_devs"
#define API_ENABLE_I2C_DEVICE              "enable_i2c_dev"
#define API_GET_I2C_DEVICE_PROPERTIES      "get_i2c_dev_prop"
#define API_SET_I2C_DEVICE_PROPERTIES      "set_i2c_dev_prop"
#endif // ENABLE_I2C

// adc
#if ENABLE_ADC
#define API_GET_ADC_DEVICES                "get_adc_devs"
#define API_ENABLE_ADC_DEVICE              "enable_adc_dev"
#define API_GET_ADC_DEVICE_PROPERTIES      "get_adc_dev_prop"
#define API_SET_ADC_DEVICE_PROPERTIES      "set_adc_dev_prop"
#define API_GET_ADC_VOLTAGE                "get_adc_voltage"
#define API_SET_ADC_VOLTAGE                "set_adc_voltage"
#endif // ENABLE_ADC

// 1wire
#if ENABLE_ONEWIRE
#define API_GET_1WIRE_DEVICES              "get_1wire_devs"
#define API_ENABLE_1WIRE_DEVICE            "enable_1wire_dev"
#define API_GET_1WIRE_DEVICE_PROPERTIES    "get_1wire_dev_prop"
#define API_SET_1WIRE_DEVICE_PROPERTIES    "set_1wire_dev_prop"
#endif // ENABLE_ONEWIRE

// tprobe
#if ENABLE_TPROBE
#define API_GET_TPROBE_DEVICES             "get_tprobe_devs"
#define API_ENABLE_TPROBE_DEVICE           "enable_tprobe_dev"
#define API_GET_TPROBE_DEVICE_PROPERTIES   "get_tprobe_dev_prop"
#define API_SET_TPROBE_DEVICE_PROPERTIES   "set_tprobe_dev_prop"
#endif // ENABLE_TPROBE



// notification
#if ENABLE_NOTIFICATIONS
#define API_TRIGGER_NOTIFICATION           "trigger_notification"
#define API_STATUS_NOTIFICATION            "status_notification"
#define API_RECEIVE_NOTIFICATION           "recv_notification"
#endif // ENABLE_NOTIFICATIONS


////////////////////////////////////////////////////////////////////////////////////
// UART Properties
////////////////////////////////////////////////////////////////////////////////////

#pragma pack(1)
typedef struct _UART_PROPERTIES {
    uint8_t m_ucBaudrate;                  // g_uwBaudrates
    uint8_t m_ucParity;                    // ft900_uart_simple.h uart_parity_t
    uint8_t m_ucFlowcontrol;               // ft900_uart_simple.h uart_flow_t
    uint8_t m_ucStopbits;                  // ft900_uart_simple.h uart_stop_bits_t
    uint8_t m_ucDatabits;                  // ft900_uart_simple.h uart_data_bits_t
} UART_PROPERTIES;
#pragma pack(reset)

#define UART_PROPERTIES_BAUDRATE_COUNT     16
#define UART_PROPERTIES_BAUDRATE           "baudrate"
#define UART_PROPERTIES_PARITY             "parity"
#define UART_PROPERTIES_FLOWCONTROL        "flowcontrol"
#define UART_PROPERTIES_STOPBITS           "stopbits"
#define UART_PROPERTIES_DATABITS           "databits"


////////////////////////////////////////////////////////////////////////////////////
// UART Commands
////////////////////////////////////////////////////////////////////////////////////

#define UART_ATCOMMAND_MAX_BUFFER_SIZE     256
#define UART_ATCOMMAND_MAX_RECIPIENT_SIZE  64
#define UART_ATCOMMAND_MAX_MESSAGE_SIZE    192
#define UART_ATCOMMAND_MAX_STATUS_SIZE     64

#define UART_ATCOMMANDS_COUNT              15
#define UART_ATCOMMAND_MOBILE              "AT+M"
#define UART_ATCOMMAND_EMAIL               "AT+E"
#define UART_ATCOMMAND_NOTIFY              "AT+N"
#define UART_ATCOMMAND_MODEM               "AT+O"
#define UART_ATCOMMAND_STORAGE             "AT+S"
#define UART_ATCOMMAND_DEFAULT             "AT+D"
#define UART_ATCOMMAND_CONTINUE            "ATC"
#define UART_ATCOMMAND_ECHO                "ATE"
#define UART_ATCOMMAND_HELP                "ATH"
#define UART_ATCOMMAND_INFO                "ATI"
#define UART_ATCOMMAND_MORE                "ATM"
#define UART_ATCOMMAND_PAUSE               "ATP"
#define UART_ATCOMMAND_RESET               "ATR"
#define UART_ATCOMMAND_UPDATE              "ATU"
#define UART_ATCOMMAND_STATUS              "AT"

#define UART_ATCOMMAND_DESC_MOBILE         "Send message as SMS to verified mobile number"
#define UART_ATCOMMAND_DESC_EMAIL          "Send message as email to verified email address"
#define UART_ATCOMMAND_DESC_NOTIFY         "Send message as mobile app notification to verified user"
#define UART_ATCOMMAND_DESC_MODEM          "Send message to other IoT modem devices"
#define UART_ATCOMMAND_DESC_STORAGE        "Send message to storage"
#define UART_ATCOMMAND_DESC_DEFAULT        "Send default message to configured endpoints"
#define UART_ATCOMMAND_DESC_CONTINUE       "Continue device functions"
#define UART_ATCOMMAND_DESC_ECHO           "Echo on/off (toggle)"
#define UART_ATCOMMAND_DESC_HELP           "Display help information on commands"
#define UART_ATCOMMAND_DESC_INFO           "Display device information"
#define UART_ATCOMMAND_DESC_MORE           "Display more information on error"
#define UART_ATCOMMAND_DESC_PAUSE          "Pause/Resume (toggle)"
#define UART_ATCOMMAND_DESC_RESET          "Reset device"
#define UART_ATCOMMAND_DESC_UPDATE         "Enter firmware update (UART entry point inside bootloader)"
#define UART_ATCOMMAND_DESC_STATUS         "Display device status"


////////////////////////////////////////////////////////////////////////////////////
// GPIO Properties
////////////////////////////////////////////////////////////////////////////////////

#define GPIO_VOLTAGE_PIN_0                 16
#define GPIO_VOLTAGE_PIN_1                 17

#define GPIO_COUNT                         4
#define GPIO_INPUT_PIN_0                   12 // Input pins: 12,13,14,15
#define GPIO_OUTPUT_ENABLE_PIN_0           36 // Output enable pins: 36, 37, 38, 39
#define GPIO_OUTPUT_PIN_0                  56 // Output pins: 56, 57, 58, 11
#define GPIO_OUTPUT_PIN_3                  11


typedef enum _GPIO_MODES_INPUT {
    GPIO_MODES_INPUT_HIGH_LEVEL,
    GPIO_MODES_INPUT_LOW_LEVEL,
    GPIO_MODES_INPUT_HIGH_EDGE,
    GPIO_MODES_INPUT_LOW_EDGE,
} GPIO_MODES_INPUT;

typedef enum _GPIO_MODES_OUTPUT {
    GPIO_MODES_OUTPUT_LEVEL,
    GPIO_MODES_OUTPUT_PULSE,
    GPIO_MODES_OUTPUT_CLOCK,
} GPIO_MODES_OUTPUT;

typedef enum _GPIO_POLARITY {
    GPIO_POLARITY_NEGATIVE,
    GPIO_POLARITY_POSITIVE,
} GPIO_POLARITY;

typedef enum _GPIO_VOLTAGE {
    GPIO_VOLTAGE_3_3,
    GPIO_VOLTAGE_5,
	GPIO_VOLTAGE_COUNT
} GPIO_VOLTAGE;

#pragma pack(1)
typedef struct _GPIO_PROPERTIES {
    uint8_t  m_ucDirection;                // ["Input", "Output"]
    uint8_t  m_ucMode;                     // ["High Level", "Low Level", "High Edge", "Low Edge"] , ["Level", "Clock", "Pulse"]
    uint8_t  m_ucAlert;                    // ["Once", "Continuously"]
    uint32_t m_ulAlertperiod;
    uint8_t  m_ucPolarity;                 // ["Negative", "Positive"]
    uint32_t m_ulWidth;
    uint32_t m_ulMark;
    uint32_t m_ulSpace;
    uint32_t m_ulCount;
} GPIO_PROPERTIES;
#pragma pack(reset)


#define GPIO_PROPERTIES_DIRECTION          "direction"
#define GPIO_PROPERTIES_MODE               "mode"
#define GPIO_PROPERTIES_ALERT              "alert"
#define GPIO_PROPERTIES_ALERTPERIOD        "period"
#define GPIO_PROPERTIES_POLARITY           "polarity"
#define GPIO_PROPERTIES_WIDTH              "width"
#define GPIO_PROPERTIES_MARK               "mark"
#define GPIO_PROPERTIES_SPACE              "space"
#define GPIO_PROPERTIES_COUNT              "count"


////////////////////////////////////////////////////////////////////////////////////
// Device Properties
////////////////////////////////////////////////////////////////////////////////////

typedef enum _DEVICE_CLASS {
    DEVICE_CLASS_SPEAKER,
    DEVICE_CLASS_DISPLAY,
    DEVICE_CLASS_LIGHT,
    DEVICE_CLASS_POTENTIOMETER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_HUMIDITY,
	DEVICE_CLASS_ANENOMOMETER,
	// add here
	DEVICE_CLASS_COUNT
} DEVICE_CLASS;

typedef enum _DEVICE_MODE {
	DEVICE_MODE_SINGLE_THRESHOLD,
	DEVICE_MODE_DUAL_THRESHOLD,
	DEVICE_MODE_CONTINUOUS,
	// add here
	DEVICE_MODE_COUNT
} DEVICE_MODE;

typedef enum _DEVICE_ENDPOINT {
	DEVICE_ENDPOINT_MANUAL,
	DEVICE_ENDPOINT_HARDWARE,
	// add here
	DEVICE_ENDPOINT_HARDWARE_COUNT
} DEVICE_ENDPOINT;

typedef enum _DEVICE_ACTIVATE {
	DEVICE_ACTIVATE_WITHINRANGE,
	DEVICE_ACTIVATE_OUTOFRANGE,
	// add here
	DEVICE_ACTIVATE_COUNT
} DEVICE_ACTIVATE;


#pragma pack(1)

typedef struct _DEVICE_PROPERTIES {
    uint8_t  m_ucSlot;
    uint8_t  m_ucAddress;
    uint8_t  m_ucEnabled;
    uint8_t  m_ucClass;
    void*    m_pvClassAttributes;
} DEVICE_PROPERTIES;

typedef struct _DEVICE_ATTRIBUTES_SPEAKER_MIDI {
    uint32_t m_ulDuration;
    uint32_t m_ulDelay;
    uint8_t  m_ucPitch;
} DEVICE_ATTRIBUTES_SPEAKER_MIDI;

typedef struct _DEVICE_ATTRIBUTES_COMMON_THRESHOLD {
    uint32_t m_ulValue;
    uint32_t m_ulMinimum;
    uint32_t m_ulMaximum;
    uint8_t  m_ucActivate;
} DEVICE_ATTRIBUTES_COMMON_THRESHOLD;

typedef struct _DEVICE_ATTRIBUTES_COMMON_ALERT {
    uint8_t  m_ucType;
    uint32_t m_ulPeriod;
} DEVICE_ATTRIBUTES_COMMON_ALERT;

typedef struct _DEVICE_ATTRIBUTES_COMMON_HARDWARE {
    char*    m_pcDeviceName;
    char*    m_pcPeripheral;
    char*    m_pcSensorName;
    char*    m_pcAttribute;
} DEVICE_ATTRIBUTES_COMMON_HARDWARE;


typedef struct _DEVICE_ATTRIBUTES_SPEAKER {
    uint8_t  m_ucEndpoint;
    uint8_t  m_ucType;
    void*    m_pvValues;
    DEVICE_ATTRIBUTES_COMMON_HARDWARE  m_oHardware;
} DEVICE_ATTRIBUTES_SPEAKER;

typedef struct  {
    uint8_t  m_ucEndpoint;
    char*    m_pcText;
    DEVICE_ATTRIBUTES_COMMON_HARDWARE  m_oHardware;
} DEVICE_ATTRIBUTES_DISPLAY;

typedef struct _DEVICE_ATTRIBUTES_LIGHT {
    uint8_t  m_ucEndpoint;
    uint32_t m_ulColor;
    uint32_t m_ulBrightness;
    uint32_t m_ulTimeout;
    char*    m_pcText;
    DEVICE_ATTRIBUTES_COMMON_HARDWARE  m_oHardware;
} DEVICE_ATTRIBUTES_LIGHT;

typedef struct _DEVICE_ATTRIBUTES_POTENTIOMETER {
    uint8_t  m_ucMode;
    DEVICE_ATTRIBUTES_COMMON_THRESHOLD m_oThreshold;
    DEVICE_ATTRIBUTES_COMMON_ALERT     m_oAlert;
    DEVICE_ATTRIBUTES_COMMON_HARDWARE  m_oHardware;
} DEVICE_ATTRIBUTES_POTENTIOMETER;

typedef struct _DEVICE_ATTRIBUTES_TEMPERATURE {
    uint8_t  m_ucMode;
    DEVICE_ATTRIBUTES_COMMON_THRESHOLD m_oThreshold;
    DEVICE_ATTRIBUTES_COMMON_ALERT     m_oAlert;
    DEVICE_ATTRIBUTES_COMMON_HARDWARE  m_oHardware;
} DEVICE_ATTRIBUTES_TEMPERATURE;

typedef struct _DEVICE_ATTRIBUTES_ANENOMOMETER {
    uint8_t  m_ucMode;
    DEVICE_ATTRIBUTES_COMMON_THRESHOLD m_oThreshold;
    DEVICE_ATTRIBUTES_COMMON_ALERT     m_oAlert;
    DEVICE_ATTRIBUTES_COMMON_HARDWARE  m_oHardware;
} DEVICE_ATTRIBUTES_ANENOMOMETER;

#pragma pack(reset)


////////////////////////////////////////////////////////////////////////////////////
// I2C Properties
////////////////////////////////////////////////////////////////////////////////////

#define I2C_COUNT                                           4

#define DEVICE_PROPERTIES_ADDRESS                           "address"
#define DEVICE_PROPERTIES_CLASS                             "class"

#define DEVICE_PROPERTIES_ENDPOINT                          "endpoint"

#define DEVICE_PROPERTIES_TYPE                              "type"
#define DEVICE_PROPERTIES_DURATION                          "duration"
#define DEVICE_PROPERTIES_PITCH                             "pitch"
#define DEVICE_PROPERTIES_DELAY                             "delay"

#define DEVICE_PROPERTIES_TEXT                              "text"

#define DEVICE_PROPERTIES_COLOR                             "color"
#define DEVICE_PROPERTIES_BRIGHTNESS                        "brightness"
#define DEVICE_PROPERTIES_TIMEOUT                           "timeout"

#define DEVICE_PROPERTIES_MODE                              "mode"
#define DEVICE_PROPERTIES_THRESHOLD                         "threshold"
#define DEVICE_PROPERTIES_THRESHOLD_VALUE                   "value"
#define DEVICE_PROPERTIES_THRESHOLD_MINIMUM                 "min"
#define DEVICE_PROPERTIES_THRESHOLD_MAXIMUM                 "max"
#define DEVICE_PROPERTIES_THRESHOLD_ACTIVATE                "activate"
#define DEVICE_PROPERTIES_ALERT                             "alert"
#define DEVICE_PROPERTIES_ALERT_TYPE                        DEVICE_PROPERTIES_TYPE
#define DEVICE_PROPERTIES_ALERT_PERIOD                      "period"
#define DEVICE_PROPERTIES_HARDWARE                          "hardware"
#define DEVICE_PROPERTIES_HARDWARE_DEVICENAME               "devicename"
#define DEVICE_PROPERTIES_HARDWARE_PERIPHERAL               "peripheral"
#define DEVICE_PROPERTIES_HARDWARE_SENSORNAME               "sensorname"
#define DEVICE_PROPERTIES_HARDWARE_ATTRIBUTE                "attribute"


////////////////////////////////////////////////////////////////////////////////////
// ADC Properties
////////////////////////////////////////////////////////////////////////////////////

#define ADC_COUNT                                           2

typedef enum _ADC_VOLTAGE {
    ADC_VOLTAGE_N5_P5,
    ADC_VOLTAGE_N10_P10,
    ADC_VOLTAGE_0_P10,
    ADC_VOLTAGE_COUNT,
} ADC_VOLTAGE;

////////////////////////////////////////////////////////////////////////////////////
// ADC Properties
////////////////////////////////////////////////////////////////////////////////////

#define ONEWIRE_COUNT                                       1


////////////////////////////////////////////////////////////////////////////////////
// TPROBE Properties
////////////////////////////////////////////////////////////////////////////////////

#define TPROBE_COUNT                                        1



////////////////////////////////////////////////////////////////////////////////////
// Publish payload strings
////////////////////////////////////////////////////////////////////////////////////

#define PAYLOAD_EMPTY                                       "{}"
#define PAYLOAD_API_GET_STATUS                              "{\"value\":{\"%s\":%d,\"version\":\"%d.%d\"}}"
#define PAYLOAD_API_SET_STATUS                              "{\"value\":{\"%s\":%d}}"
#define PAYLOAD_API_GET_XXX_DEVICES_EMPTY                   "{\"value\":[]}"
#define PAYLOAD_API_GET_XXX_DEVICES                         "{\"value\":[{\"%s\":%d,\"%s\":%d}]}"
#define PAYLOAD_API_GET_XXX_DEVICE_PROPERTIES_EMPTY         "{\"value\":{}}"

#define PAYLOAD_API_GET_XXX_DEVICE_PROPERTIES_POTENTIOMETER "{\"value\":{\"%s\":%d,\"%s\":{\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d},\"%s\":{\"%s\":%d,\"%s\":%d},\"%s\":{\"%s\":\"%s\"}}}"
#define PAYLOAD_API_GET_XXX_DEVICE_PROPERTIES_TEMPERATURE   "{\"value\":{\"%s\":%d,\"%s\":{\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d},\"%s\":{\"%s\":%d,\"%s\":%d},\"%s\":{\"%s\":\"%s\"}}}"
#define PAYLOAD_API_GET_XXX_DEVICE_PROPERTIES_ANEMOMETER    "{\"value\":{\"%s\":%d,\"%s\":{\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d},\"%s\":{\"%s\":%d,\"%s\":%d},\"%s\":{\"%s\":\"%s\"}}}"
#define PAYLOAD_API_GET_XXX_DEVICE_PROPERTIES_SPEAKER       "{\"value\":{\"%s\":%d,\"%s\":%d,\"values\":{\"%s\":%d,\"%s\":%d,\"%s\":%d},\"%s\":{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}}}"
#define PAYLOAD_API_GET_XXX_DEVICE_PROPERTIES_DISPLAY       "{\"value\":{\"%s\":%d,\"%s\":\"%s\",\"%s\":{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}}}"
#define PAYLOAD_API_GET_XXX_DEVICE_PROPERTIES_LIGHT         "{\"value\":{\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"%s\"}}}"


#if ENABLE_UART
#define TOPIC_UART                                          "%s%s/trigger_notification/uart/%s"
#define PAYLOAD_API_GET_UARTS                               "{\"value\":{\"uarts\":[{\"%s\":%d}]}}"
#define PAYLOAD_API_GET_UART_PROPERTIES                     "{\"value\":{\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d}}"
#endif // ENABLE_UART

#if ENABLE_GPIO
#define TOPIC_GPIO                                          "%s%s/trigger_notification/gpio%d/%s"
#define PAYLOAD_API_GET_GPIOS                               "{\"value\":{\"%s\":%d,\"gpios\":[{\"%s\":%d,\"%s\":%d,\"%s\":%d},{\"%s\":%d,\"%s\":%d,\"%s\":%d},{\"%s\":%d,\"%s\":%d,\"status\":%d},{\"%s\":%d,\"%s\":%d,\"%s\":%d}]}}"
#define PAYLOAD_API_GET_GPIO_VOLTAGE                        "{\"value\":{\"%s\":%d}}"
#define PAYLOAD_API_GET_GPIO_PROPERTIES                     "{\"value\":{\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d,\"%s\":%d}}"
#define PAYLOAD_TRIGGER_GPIO_NOTIFICATION                   "{\"activate\":%d}"
#endif // ENABLE_GPIO

#if ENABLE_I2C
#define TOPIC_I2C                                           "%s%s/trigger_notification/i2c%d/%s"
#define PAYLOAD_API_GET_I2C_DEVICES                         "{\"value\":[{\"%s\":%d,\"%s\":%d,\"%s\":%d}]}"
#endif // ENABLE_I2C

#if ENABLE_ADC
#define TOPIC_ADC                                            "%s%s/trigger_notification/adc%d/%s"
#define PAYLOAD_API_GET_ADC_VOLTAGE                         "{\"value\":{\"%s\":%d}}"
#endif // ENABLE_ADC

#if ENABLE_ONEWIRE
#define TOPIC_1WIRE                                          "%s%s/trigger_notification/1wire%d/%s"
#endif // ENABLE_ONEWIRE

#if ENABLE_TPROBE
#define TOPIC_TPROBE                                         "%s%s/trigger_notification/tprobe%d/%s"
#endif // ENABLE_TPROBE



////////////////////////////////////////////////////////////////////////////////////
// UART Functions
////////////////////////////////////////////////////////////////////////////////////

void iot_modem_uart_enable_interrupt();
void iot_modem_uart_enable(UART_PROPERTIES* properties, int enable, int disable);
void iot_modem_uart_command_process();
void iot_modem_uart_command_help();


////////////////////////////////////////////////////////////////////////////////////
// GPIO Functions
////////////////////////////////////////////////////////////////////////////////////

void iot_modem_gpio_init(int voltage);
void iot_modem_gpio_enable_interrupt();
int  iot_modem_gpio_enable(GPIO_PROPERTIES* properties, int index, int enable);
void iot_modem_gpio_set_properties(int index, int direction, int polarity);
void iot_modem_gpio_set_voltage(int voltage);
uint8_t iot_modem_gpio_get_status(GPIO_PROPERTIES* properties, int index);
void iot_modem_gpio_process(int number, int activate);


////////////////////////////////////////////////////////////////////////////////////
// I2C Functions
////////////////////////////////////////////////////////////////////////////////////

void iot_modem_i2c_init();
int  iot_modem_i2c_enable(DEVICE_PROPERTIES* properties);
void iot_modem_i2c_set_properties(DEVICE_PROPERTIES* properties);
uint32_t iot_modem_i2c_get_sensor_reading(DEVICE_PROPERTIES* properties);


////////////////////////////////////////////////////////////////////////////////////
// ADC Functions
////////////////////////////////////////////////////////////////////////////////////

void iot_modem_adc_init(int voltage);
int  iot_modem_adc_enable(DEVICE_PROPERTIES* properties);
void iot_modem_adc_set_properties(DEVICE_PROPERTIES* properties);
uint32_t iot_modem_adc_get_sensor_reading(DEVICE_PROPERTIES* properties);
void iot_modem_adc_set_voltage(int voltage);


////////////////////////////////////////////////////////////////////////////////////
// 1WIRE Functions
////////////////////////////////////////////////////////////////////////////////////

void iot_modem_1wire_init();
int  iot_modem_1wire_enable(DEVICE_PROPERTIES* properties);
void iot_modem_1wire_set_properties(DEVICE_PROPERTIES* properties);
uint32_t iot_modem_1wire_get_sensor_reading(DEVICE_PROPERTIES* properties);


////////////////////////////////////////////////////////////////////////////////////
// TPROBE Functions
////////////////////////////////////////////////////////////////////////////////////

void iot_modem_tprobe_init();
int  iot_modem_tprobe_enable(DEVICE_PROPERTIES* properties);
void iot_modem_tprobe_set_properties(DEVICE_PROPERTIES* properties);
uint32_t iot_modem_tprobe_get_sensor_reading(DEVICE_PROPERTIES* properties);



#endif /* _IOT_MODEM_H_ */
