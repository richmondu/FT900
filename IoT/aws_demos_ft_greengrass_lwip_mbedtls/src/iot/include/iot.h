/*
 * ============================================================================
 * Copyright (C) Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 * ("Bridgetek") subject to the licence terms set out
 * http://brtchip.com/BRTSourceCodeLicenseAgreement/ ("the Licence Terms").
 * You must read the Licence Terms before downloading or using the Software.
 * By installing or using the Software you agree to the Licence Terms. If you
 * do not agree to the Licence Terms then do not download or use the Software.
 *
 * Without prejudice to the Licence Terms, here is a summary of some of the key
 * terms of the Licence Terms (and in the event of any conflict between this
 * summary and the Licence Terms then the text of the Licence Terms will
 * prevail).
 *
 * The Software is provided "as is".
 * There are no warranties (or similar) in relation to the quality of the
 * Software. You use it at your own risk.
 * The Software should not be used in, or for, any medical device, system or
 * appliance. There are exclusions of Bridgetek liability for certain types of loss
 * such as: special loss or damage; incidental loss or damage; indirect or
 * consequential loss or damage; loss of income; loss of business; loss of
 * profits; loss of revenue; loss of contracts; business interruption; loss of
 * the use of money or anticipated savings; loss of information; loss of
 * opportunity; loss of goodwill or reputation; and/or loss of, damage to or
 * corruption of data.
 * There is a monetary cap on Bridgetek's liability.
 * The Software may have subsequently been amended by another user and then
 * distributed by that other user ("Adapted Software").  If so that user may
 * have additional licence terms that apply to those amendments. However, Bridgetek
 * has no liability in relation to those amendments.
 * ============================================================================
 */

#ifndef _IOT_H_
#define _IOT_H_



/* Standard includes. */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "net.h"
#include "FreeRTOS.h"
#include "task.h"

/* IOT includes. */
#include "iot_clientcredential.h"
#include "iot_config.h"

/* LWIP includes. */
#include "lwipopts.h"

/* MQTT includes. */
#include "aws_mqtt_lib.h"
#include "aws_mqtt_agent.h"
#include "aws_bufferpool.h"
#include "config_files/aws_bufferpool_config.h"
#include "config_files/aws_mqtt_agent_config.h"
#include "config_files/aws_mqtt_config.h"

/* MBEDTLS includes. */
#include "mbedtls_config.h"



#define iot_MAX_BUFFER_SIZE (bufferpoolconfigBUFFER_DATA_SIZE)

typedef BaseType_t iot_status;
typedef TaskFunction_t iot_task;
typedef MQTTAgentConnectParams_t iot_connect_params_t;
typedef MQTTAgentPublishParams_t iot_publish_params_t;
typedef MQTTAgentSubscribeParams_t iot_subscribe_params_t;

iot_status iot_setup( iot_task task );
iot_status iot_connect( const iot_connect_params_t* param );
iot_status iot_disconnect( void );
iot_status iot_publish( const iot_publish_params_t* param );
iot_status iot_subsribe( const iot_subscribe_params_t* param );



#endif
