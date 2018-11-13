/*
 * ============================================================================
 * History
 * =======
 * 09 Nov 2018 : Created
 *
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

#ifndef _IOT_CONFIG_DEFAULTS_H_
#define _IOT_CONFIG_DEFAULTS_H_



///////////////////////////////////////////////////////////////////////////////////
#ifndef USE_MQTT_BROKER
#error "USE_MQTT_BROKER must be set"
#elif !(USE_MQTT_BROKER >= MQTT_BROKER_UNKNOWN && USE_MQTT_BROKER < MQTT_BROKER_COUNT)
#error "Unexpected USE_MQTT_BROKER value"
#endif // USE_MQTT_BROKER

#ifndef USE_DEVICE_ID
#error "USE_DEVICE_ID must be set"
#endif // DEVICE_ID
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
// USE_ROOT_CA
// If enabled, two-way authentication, client will also authenticate server
// If disabled, client authentication, no server authentication, prone to man-in-the-middle attacks
// This must be enabled in production environment
#ifndef USE_ROOT_CA
#define USE_ROOT_CA                   1
#endif

// USE_PAYLOAD_TIMESTAMP
// If enabled, RTC will be used.
// Note that enabling this increases memory footprint
// Disable this to save some memory footprint for use of sensor
#ifndef USE_PAYLOAD_TIMESTAMP
#define USE_PAYLOAD_TIMESTAMP         1
#endif // USE_PAYLOAD_TIMESTAMP

// USE_MQTT_PUBLISH, USE_MQTT_SUBSCRIBE
// By default, just do publish, no subscribe.
#ifndef USE_MQTT_PUBLISH
#define USE_MQTT_PUBLISH              1
#endif // USE_MQTT_PUBLISH

#ifndef USE_MQTT_SUBSCRIBE
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT) || (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
#define USE_MQTT_SUBSCRIBE            0 // Disabled by default; tested working
#elif (USE_MQTT_BROKER == MQTT_BROKER_GCP_IOT)
#define USE_MQTT_SUBSCRIBE            0 // Disabled by default; tested working for /config not /events
#elif (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT)
#define USE_MQTT_SUBSCRIBE            0 // Azure does not support MQTT subscribe
#else
#define USE_MQTT_SUBSCRIBE            0
#endif
#endif // USE_MQTT_SUBSCRIBE

// DEBUG_IOT_API
// Set to enable/disable logs in iot.c
#ifndef DEBUG_IOT_API
#define DEBUG_IOT_API                 1
#endif // DEBUG_IOT_API
///////////////////////////////////////////////////////////////////////////////////



#endif /* __IOT_CONFIG_DEFAULTS_H__ */
