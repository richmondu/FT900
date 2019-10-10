/*
 * Amazon FreeRTOS V1.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file aws_mqtt_agent_config.h
 * @brief MQTT agent config options.
 */

#ifndef _AWS_MQTT_AGENT_CONFIG_H_
#define _AWS_MQTT_AGENT_CONFIG_H_

#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief Controls whether or not to report usage metrics to the
 * AWS IoT broker.
 *
 * If mqttconfigENABLE_METRICS is set to 1, a string containing
 * metric information will be included in the "username" field of
 * the MQTT connect messages.
 */
#define mqttconfigENABLE_METRICS    ( 0 )
//#define mqttconfigENABLE_METRICS    ( 0 ) TODO // RMU

/**
 * @defgroup Metrics The metrics reported to the AWS IoT broker.
 *
 * If mqttconfigENABLE_METRICS is set to 1, these will be included
 * in the "username" field of MQTT connect messages.
 */
/** @{ */
#if mqttconfigENABLE_METRICS
#define mqttconfigMETRIC_SDK         "SDK=AmazonFreeRTOS"               /**< The SDK used by this device. */
#define mqttconfigMETRIC_VERSION     "Version="tskKERNEL_VERSION_NUMBER /**< The version number of this SDK. */
#define mqttconfigMETRIC_PLATFORM    "Platform=STM32L475"               /**< The platform that this SDK is running on. */
#endif
/** @} */

/**
 * @brief The maximum time interval in seconds allowed to elapse between 2 consecutive
 * control packets.
 */
#define mqttconfigKEEP_ALIVE_INTERVAL_SECONDS         ( 1200 )

/**
 * @brief Defines the frequency at which the client should send Keep Alive messages.
 *
 * Even though the maximum time allowed between 2 consecutive control packets
 * is defined by the mqttconfigKEEP_ALIVE_INTERVAL_SECONDS macro, the user
 * can and should send Keep Alive messages at a slightly faster rate to ensure
 * that the connection is not closed by the server because of network delays.
 * This macro defines the interval of inactivity after which a keep alive messages
 * is sent.
 */
#define mqttconfigKEEP_ALIVE_ACTUAL_INTERVAL_TICKS    ( pdMS_TO_TICKS( 300000 ) )

/**
 * @brief The maximum interval in ticks to wait for PINGRESP.
 *
 * If PINGRESP is not received within this much time after sending PINGREQ,
 * the client assumes that the PINGREQ timed out.
 */
#define mqttconfigKEEP_ALIVE_TIMEOUT_TICKS            ( 1000 )

/**
 * @brief The maximum time in ticks for which the MQTT task is permitted to block.
 *
 * Since ST board's WiFi module does not have any mechanism to wake up the MQTT task
 * whenever data is received on a connected socket, this value must be small to ensure
 * that the MQTT task keeps waking up frequently and processes the publish messages
 * received from the broker, if any.
 */
#define mqttconfigMQTT_TASK_MAX_BLOCK_TICKS           ( 1000 )

/**
 * @defgroup MQTTTask MQTT task configuration parameters.
 */
/** @{ */
#define mqttconfigMQTT_TASK_STACK_DEPTH    ( 768 )
#define mqttconfigMQTT_TASK_PRIORITY       ( configMAX_PRIORITIES - 3 )
/** @} */

/**
 * @brief Maximum number of MQTT clients that can exist simultaneously.
 */
#define mqttconfigMAX_BROKERS            ( 1 )

/**
 * @brief Maximum number of parallel operations per client.
 */
#define mqttconfigMAX_PARALLEL_OPS       ( 2 )

/**
 * @brief Time in milliseconds after which the TCP send operation should timeout.
 */
#define mqttconfigTCP_SEND_TIMEOUT_MS    ( 20 )

/**
 * @brief Added disabling of MQTT_Delete to reduce code size
 */
#define mqttconfigDISABLE_DELETE			1

/**
 * @brief Added disabling of MQTT_Disconnect to reduce code size
 */
#define mqttconfigDISABLE_DISCONNECT		0

/**
 * @brief Added disabling of MQTT_Subscribe/Unsubscribe to reduce code size
 */
#define mqttconfigDISABLE_SUBSCRIBE			0
#define mqttconfigENABLE_SUBSCRIPTION_MANAGEMENT 1

#endif /* _AWS_MQTT_AGENT_CONFIG_H_ */
