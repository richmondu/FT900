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

#ifndef __AWS_CLIENTCREDENTIAL__H__
#define __AWS_CLIENTCREDENTIAL__H__



/*
 * Switch between secure MQTT and non-secure MQTT
 * Note that non-secure MQTT can be tested using Mosquitto local broker only
 * AWS does not support non-secure MQTT
 */
#define USE_TLS        	1

/*
 * Switch for MQTT message broker to be used
 */
#define MQTT_BROKER_MOSQUITTO       0	// local Mosquitto broker
#define MQTT_BROKER_AWS_GREENGRASS  1	// local AWS Greengrass broker
#define MQTT_BROKER_AWS_IOT			2	// AWS IoT cloud
#define USE_MQTT_BROKER 	MQTT_BROKER_AWS_GREENGRASS
//#define USE_MQTT_BROKER 	MQTT_BROKER_AWS_IOT
//#define USE_MQTT_BROKER 	MQTT_BROKER_MOSQUITTO

/*
 * Switch between verifying Root CA certificate
 * If 0, root CA will not be verified. This is prone to man-in-the-middle attacks.
 * However, note that the client certificate and private key are still verified.
 */
#define USE_ROOTCA     	0 // Do not modify for now

/*
 * Switch to handle or not handle Ethernet unplug/plug scenario
 * Disabling this helps when debugging issues
 */
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
#define HANDLE_RECONNECTION 1
#else
#define HANDLE_RECONNECTION 0
#endif


/*
 * Setup the IP address, gateway and subnet mask of this FT900 device
 */
#define FT9XX_IP_ADDRESS "192.168.254.200"
#define FT9XX_IP_GATEWAY "192.168.254.254"
#define FT9XX_IP_SUBNET  "255.255.255.0"

/*
 * MQTT Broker endpoint.
 */
#if USE_TLS
	#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
		static const char clientcredentialMQTT_BROKER_ENDPOINT[] = "192.168.254.107"; // local Greengrass server
	#elif (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT)
		static const char clientcredentialMQTT_BROKER_ENDPOINT[] = "amasgua12bmkv.iot.us-east-1.amazonaws.com";
	#elif (USE_MQTT_BROKER == MQTT_BROKER_MOSQUITTO)
		static const char clientcredentialMQTT_BROKER_ENDPOINT[] = "192.168.254.102"; // local mosquitto server
	#endif
#else // USE_TLS
	static const char clientcredentialMQTT_BROKER_ENDPOINT[] = "192.168.254.102"; // local mosquitto server
#endif // USE_TLS

/*
 * Port number the MQTT broker is using.
 */
#if USE_TLS
#define clientcredentialMQTT_BROKER_PORT 8883
#else // USE_TLS
#define clientcredentialMQTT_BROKER_PORT 1883
#endif // USE_TLS

/*
 * When using Greengrass, this must be the thing name associated with the certificates
 */
#define clientcredentialMQTT_CLIENT_ID "HelloWorld_Publisher"

/*
 * Root CA certificate for the client certificate and private key to be used
 */
#if USE_TLS
#if USE_ROOTCA
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
static const char clientcredentialROOTCA_CERTIFICATE_PEM[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIEFDCCAvygAwIBAgIUEht3/gFpOYcz7hNP7D0z1NyGZ8MwDQYJKoZIhvcNAQEL\n"
"BQAwgagxCzAJBgNVBAYTAlVTMRgwFgYDVQQKDA9BbWF6b24uY29tIEluYy4xHDAa\n"
"BgNVBAsME0FtYXpvbiBXZWIgU2VydmljZXMxEzARBgNVBAgMCldhc2hpbmd0b24x\n"
"EDAOBgNVBAcMB1NlYXR0bGUxOjA4BgNVBAMMMTc3MzUxMDk4MzY4Nzo1NTMzYzhj\n"
"NS1lMGRiLTQxMzAtODA1NC1hZDA3OWMxMWJmZjYwIBcNMTgwNjI2MDY1NjE1WhgP\n"
"MjA5ODA2MjYwNjU2MTRaMIGoMQswCQYDVQQGEwJVUzEYMBYGA1UECgwPQW1hem9u\n"
"LmNvbSBJbmMuMRwwGgYDVQQLDBNBbWF6b24gV2ViIFNlcnZpY2VzMRMwEQYDVQQI\n"
"DApXYXNoaW5ndG9uMRAwDgYDVQQHDAdTZWF0dGxlMTowOAYDVQQDDDE3NzM1MTA5\n"
"ODM2ODc6NTUzM2M4YzUtZTBkYi00MTMwLTgwNTQtYWQwNzljMTFiZmY2MIIBIjAN\n"
"BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAgydmgmVjXVxXaaOYaTfWNKQs/j4P\n"
"S8qEySe4ElgA6HKicNxzmdKzZR15xybo1/+vFQxgBN1bnkVsv3/pvtdnM0A3sDSx\n"
"DMgvtoIOpuJxk130hByYHUZFtDcZth7sBC/9UTMwI12L9FwTBo7HOqqko32fNzuQ\n"
"ALCwiQSklZgUSrIoqDD1M5JyJ8H9Qemge2+pHwNjHGoBtPf6Vl+wzY2bHo4kH6KX\n"
"fCDPcCdf7FwjQbQIHE9DUAedim9ellSAFyBsdBaHjEaaP5IgCzAXSdiWkGv+ET8h\n"
"WPkch8Bu/BQaalMSUXj/vl3lwYJcSsvCb74CLN5yjt6BToWytc8/1ES9ywIDAQAB\n"
"ozIwMDAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBS85O1vhpmmY1yLKWAN472B\n"
"uWIVTDANBgkqhkiG9w0BAQsFAAOCAQEAIRskpVbB5WwUc6ILgIC0RmKtLdpvrbiT\n"
"Wwo2gQSFDS2jmis+KkxD5LmhI5tu2DpGeB7hfzbyhnBQJNIVOAwX7PSaEWdo9V7+\n"
"4BL9c51kE7f25qv5xPC0HAzOu26I8dF/26/qSfSdZHM04SrkOQTl0+8PBPQi7zVL\n"
"chKrtF1lFy0Bpw6HjD1NykSDKDICTmyjMpdOoDUUKIDLrttViybIpl7oIWEAjruo\n"
"OeEwDCNN+Y/ztsYrcA1FZ8FlSRL7GzVOnHeTzVvPaeSSG34gRXvnSsKk03VuubBy\n"
"Px6Mca4p4Kwd5r5sUrGLt6rBSOe26SABiPmzRCLm25el6rZG0fzm9w==\n"
"-----END CERTIFICATE-----\n";
#elif (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT)
static const char clientcredentialROOTCA_CERTIFICATE_PEM[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIE0zCCA7ugAwIBAgIQGNrRniZ96LtKIVjNzGs7SjANBgkqhkiG9w0BAQUFADCB\n"
"yjELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL\n"
"ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJp\n"
"U2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxW\n"
"ZXJpU2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0\n"
"aG9yaXR5IC0gRzUwHhcNMDYxMTA4MDAwMDAwWhcNMzYwNzE2MjM1OTU5WjCByjEL\n"
"MAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQLExZW\n"
"ZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJpU2ln\n"
"biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxWZXJp\n"
"U2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9y\n"
"aXR5IC0gRzUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvJAgIKXo1\n"
"nmAMqudLO07cfLw8RRy7K+D+KQL5VwijZIUVJ/XxrcgxiV0i6CqqpkKzj/i5Vbex\n"
"t0uz/o9+B1fs70PbZmIVYc9gDaTY3vjgw2IIPVQT60nKWVSFJuUrjxuf6/WhkcIz\n"
"SdhDY2pSS9KP6HBRTdGJaXvHcPaz3BJ023tdS1bTlr8Vd6Gw9KIl8q8ckmcY5fQG\n"
"BO+QueQA5N06tRn/Arr0PO7gi+s3i+z016zy9vA9r911kTMZHRxAy3QkGSGT2RT+\n"
"rCpSx4/VBEnkjWNHiDxpg8v+R70rfk/Fla4OndTRQ8Bnc+MUCH7lP59zuDMKz10/\n"
"NIeWiu5T6CUVAgMBAAGjgbIwga8wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8E\n"
"BAMCAQYwbQYIKwYBBQUHAQwEYTBfoV2gWzBZMFcwVRYJaW1hZ2UvZ2lmMCEwHzAH\n"
"BgUrDgMCGgQUj+XTGoasjY5rw8+AatRIGCx7GS4wJRYjaHR0cDovL2xvZ28udmVy\n"
"aXNpZ24uY29tL3ZzbG9nby5naWYwHQYDVR0OBBYEFH/TZafC3ey78DAJ80M5+gKv\n"
"MzEzMA0GCSqGSIb3DQEBBQUAA4IBAQCTJEowX2LP2BqYLz3q3JktvXf2pXkiOOzE\n"
"p6B4Eq1iDkVwZMXnl2YtmAl+X6/WzChl8gGqCBpH3vn5fJJaCGkgDdk+bW48DW7Y\n"
"5gaRQBi5+MHt39tBquCWIMnNZBU4gcmU7qKEKQsTb47bDN0lAtukixlE0kF6BWlK\n"
"WE9gyn6CagsCqiUXObXbf+eEZSqVir2G3l6BFoMtEMze/aiCKm0oHw0LxOXnGiYZ\n"
"4fQRbxC1lfznQgUy286dUV4otp6F01vvpX1FQHKOtw5rDgb7MzVIcbidJ4vEZV8N\n"
"hnacRHr2lVz2XTIIM6RUthg/aFzyQkqFOFSDX9HoLPKsEdao7WNq\n"
"-----END CERTIFICATE-----\n";
#elif (USE_MQTT_BROKER == MQTT_BROKER_MOSQUITTO)
static const char clientcredentialROOTCA_CERTIFICATE_PEM[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIDqjCCApKgAwIBAgIJALMDQ5jBUzRMMA0GCSqGSIb3DQEBDQUAMGoxFzAVBgNV\n"
"BAMMDkFuIE1RVFQgYnJva2VyMRYwFAYDVQQKDA1Pd25UcmFja3Mub3JnMRQwEgYD\n"
"VQQLDAtnZW5lcmF0ZS1DQTEhMB8GCSqGSIb3DQEJARYSbm9ib2R5QGV4YW1wbGUu\n"
"bmV0MB4XDTE4MDYxMDAyMzc1M1oXDTMyMDYwNjAyMzc1M1owajEXMBUGA1UEAwwO\n"
"QW4gTVFUVCBicm9rZXIxFjAUBgNVBAoMDU93blRyYWNrcy5vcmcxFDASBgNVBAsM\n"
"C2dlbmVyYXRlLUNBMSEwHwYJKoZIhvcNAQkBFhJub2JvZHlAZXhhbXBsZS5uZXQw\n"
"ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDSfRrDEYaJWhIDfB+7YsWR\n"
"qJefd0PpUq8pYFTqW9tiQhKNGdbPqnfW2LP82A/9FG+DpTCSBAwI5BTV/svtAexB\n"
"Km/II+n5toedV+5E7iOLUul2Fe8qXwAODdBUJ6xgJB10AiVKuUXFdnZj/SebZ2Ek\n"
"iEewaQpFkFh88ubbxsqaD0/RlP6/SDHtwf+wMvMwBE+0fphETPRFj5dvgphzQ+cA\n"
"m6QUBq99pcLEhsgkUW+rZPgwQ53JRf7UxFzqUL2L/iSO56+CYOZTai6wEodjh+se\n"
"1NQfC5vJ98d8q2Wo0x5xSK7rgy1Zk3gZuf3YaOC2zGsdlw1XgyFNWeYu1s/K0kqT\n"
"AgMBAAGjUzBRMB0GA1UdDgQWBBSN2a7w29Zegh9ZWINMYfqxwbiBkjAfBgNVHSME\n"
"GDAWgBSN2a7w29Zegh9ZWINMYfqxwbiBkjAPBgNVHRMBAf8EBTADAQH/MA0GCSqG\n"
"SIb3DQEBDQUAA4IBAQBv+ctidnabaQNR/CVlgQuICDsTCIMU5NQfP3aDxZCcPQEz\n"
"groDMNpMDcvAHufug8N7BtapsShqgoF4k7KV1MuJmTXwmUYIwAV70/iWDySInPbS\n"
"/RTuh5/KCVFyZZQumjZ6qltZfowu5CHL2ezgfSKmYHg/E/3odIAFH6c5zLDpv7pa\n"
"K54XFnKmXINIDXVCONPRqkrBdgPPnxZQTFebI1Rz9ONoxtR/AI+/IRkqrg89KhS8\n"
"snuvF/fZ12bUEJcYM49FVFXlwYq+1vnqk9AYmqAr6J19+uYsLUWEC4N6/c/c4ZtM\n"
"GlZZyhIcJF23OsouFO9IV4MoRJrK9Z+rCjyL91oe\n"
"-----END CERTIFICATE-----\n";
#endif
#endif // USE_ROOTCA
#endif // USE_TLS

#if USE_TLS
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS || USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT)
/*
 * Client certificate signed by the root CA certificate
 */
static const char clientcredentialCLIENT_CERTIFICATE_PEM[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIDWjCCAkKgAwIBAgIVAMIQwgFqTHGTLGUe8r0RC36FxWAWMA0GCSqGSIb3DQEB\n"
"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n"
"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0xODA2MjYwNzQ2\n"
"MjdaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n"
"dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCq1qHQj30Y/qL5ddKD\n"
"3WS+zMm4/8b/gNJw8X+fwcSm6dExKPUzZQ7u5EbMVB471PjNM/Q2LG+p9mJdKCaq\n"
"nBvCEnpVsRlRzQ7/RxRQmN+YCuhOcGjksMvyXoJmkzG5dyzFvlFNsbMaCP3zcsdR\n"
"91UpWWh8NbooJJ91ntLhbnTIPOrE0wtH3rxV6jTU1LC0S6RfqnIeJd4r0Og1oCcI\n"
"QFwkUZcLCICsu6Fm+yJYJ3TbpV19aELYjHic3L7RNjYU2slSM2FWisX03sGx+WCu\n"
"pNBpgetxXxrjhhh0KmrSbcCjJVIdX6cE9LQAnETxeiEXJFOXBamEpu8koVErdN5l\n"
"wVnFAgMBAAGjYDBeMB8GA1UdIwQYMBaAFHL1VHiPUszKBj5Arpv9FjN2D/OIMB0G\n"
"A1UdDgQWBBTYixFRzc+8uoOmJGK7/hOYkGt92DAMBgNVHRMBAf8EAjAAMA4GA1Ud\n"
"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEADUbPLO2Bg3RmM3HX3sGmQUDO\n"
"+HWIY9RDozYea5Tv2sA9VBwA0fdIw0ppFkqwIZ0q6gtFLwvHWZFlwYKQY0H73INI\n"
"DdSAVfNknbT5uxXK1vy8xpVWA5oownTjn4cyrpU0iTR1Nc9Y0kAKcAqWmI30KMxw\n"
"RKgXBs6jo7rwJttEgpsnje142w3CTbztbaL8ZTt192DI5Ovz65vlsznA1Y7Qqw2L\n"
"jz+E7WiTbRqNyrpJjNbt2w4Foo3qeqsHzOaesMfOrsKu1dCa1jfrFne+T/Z+l6LH\n"
"ITSGE9QuiJjYR9sDHY/dquYkp6MMGX1BK8DuMqsoCfLEMQZhCZs6Ppba06QWYw==\n"
"-----END CERTIFICATE-----\n";

/*
 * Private key associated with the client certificate
 */
static const char clientcredentialCLIENT_PRIVATE_KEY_PEM[] =
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEAqtah0I99GP6i+XXSg91kvszJuP/G/4DScPF/n8HEpunRMSj1\n"
"M2UO7uRGzFQeO9T4zTP0NixvqfZiXSgmqpwbwhJ6VbEZUc0O/0cUUJjfmAroTnBo\n"
"5LDL8l6CZpMxuXcsxb5RTbGzGgj983LHUfdVKVlofDW6KCSfdZ7S4W50yDzqxNML\n"
"R968Veo01NSwtEukX6pyHiXeK9DoNaAnCEBcJFGXCwiArLuhZvsiWCd026VdfWhC\n"
"2Ix4nNy+0TY2FNrJUjNhVorF9N7BsflgrqTQaYHrcV8a44YYdCpq0m3AoyVSHV+n\n"
"BPS0AJxE8XohFyRTlwWphKbvJKFRK3TeZcFZxQIDAQABAoIBAGrLEQxXJlRhmW9n\n"
"/9j+pOPBELjc64D8/pAr5yPu9QeCUorwl5cxLUyY1skULHIqh/1+5MKYtJUCuCqT\n"
"1tgADjobYu3+b117WkyqGkDnz54z81RVYLzU6Cjye4GzsfnGTe6vMhPnaZZ02Is+\n"
"zOqoOkxG39IFpAFEI4pbZmgVrFse88SsYab+1FWrIiAFai81+iMveDsjeFf9nrmP\n"
"jgJ/dkYlZEQxJgjm16cGCE+WabomX+Ne94eiY5Lv7aMD5+59oqqwnFzwDpdrn/r2\n"
"q1tmFEheBcnK/dAlMxTUbUIf545xiY0PcHoKrJmV4k4yPG60/cY+WF4dAgC42Pnx\n"
"boNUbQECgYEA2xlH+aWVOu9NdQbAhGsUj6vvBBdXmTHnnVDEzl/VBfKQZ3c+P0U9\n"
"olKxS3Sy+XjchbeOAYrolClgIOtS8CXNrZho+ZRXYU7AKFUBrEvhiWshjytRpiip\n"
"e0gofApPD+8gSCwal0XVOBJ1iWCULoINhtVesitHxDbUvixTc0L8tW0CgYEAx5yL\n"
"lEmNK+Ow8lNOP3QOR8jq/8glFKn9WL8TMCcpC3lRV7WmTHB1vprtNi9MVOPPB2cU\n"
"BpLQkEjF8um6NnjKHTkx/bHTijHg3WifrqyKFymnTuCeFIdz9td6JW4a7jOWgq16\n"
"EQl9H9JXWU9IXKfgYeg4C+rPS4lmoi0bRFBKdrkCgYEAvy4mMd6cgtqBOhZ0MpfF\n"
"T0B1xZogo9p6AjmSIYpZtWDdRIs2U7s1dsi1T+Q6r5kYw81RKmNtYqtf6BmvO0Gh\n"
"YVLWdsFMJeqznf3fAdsMp/5FQEpKarScfqiOOv470umTv6ZMZadX0B+7U+5kHtj4\n"
"uKjCgrFlY/98T50aDD9th1kCgYAOw04zbmkpeNKKaYzc9olj18FBn5zWdWgOWzth\n"
"EVRgy90vPnkJKDwdF7o5iq/7i0mxg3cgsVUJhshVGeIyyyYvmR5QZAmALAY0edtt\n"
"gMdJxUPheo1WeVojRHZ9NDJ7sYcNLSVdAzWk19qr+Ughyiy3MQSMJRalwIi9r9ZX\n"
"bBTFyQKBgQC+wGlaiHLnydPDfwfBo9kZBv/opiiJMkhUX2WrOYdSx/4Qz2zsnU6m\n"
"Nf303wS0vmYK0WMukcCuWe/twTZXc3zErFo/0iCGihA8sesAudTXiKcb+sEqFvOx\n"
"0E2yyBo7qFe2F+k7OcTHDSgAvVCSmBrE/9QbXjwdNBVWgezmLuLoIg==\n"
"-----END RSA PRIVATE KEY-----\n";
#elif (USE_MQTT_BROKER == MQTT_BROKER_MOSQUITTO)
/*
 * Client certificate signed by the root CA certificate
 */
static const char clientcredentialCLIENT_CERTIFICATE_PEM[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIFODCCBCCgAwIBAgIJANUhUNn92KzIMA0GCSqGSIb3DQEBDQUAMGoxFzAVBgNV\n"
"BAMMDkFuIE1RVFQgYnJva2VyMRYwFAYDVQQKDA1Pd25UcmFja3Mub3JnMRQwEgYD\n"
"VQQLDAtnZW5lcmF0ZS1DQTEhMB8GCSqGSIb3DQEJARYSbm9ib2R5QGV4YW1wbGUu\n"
"bmV0MB4XDTE4MDYxMDAyMzgwMVoXDTMyMDYwNjAyMzgwMVowZzEUMBIGA1UEAwwL\n"
"cmljaG1vbmQtUEMxFjAUBgNVBAoMDU93blRyYWNrcy5vcmcxFDASBgNVBAsMC2dl\n"
"bmVyYXRlLUNBMSEwHwYJKoZIhvcNAQkBFhJub2JvZHlAZXhhbXBsZS5uZXQwggEi\n"
"MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDAUIGfwUvdr88z/qTokzL/ulYp\n"
"+QrwuXVgion5D5qyx7DTFSIRSJSaMWYcQpvxhn9XQdJi4OXfqpb/2l9gzc8Thh2o\n"
"mx/5SVs7jW7e5Ne0DOWUc6y2OiOHsJymZ+uMapQk3cPLcRPZjf28csxMNxolD2BE\n"
"r2comhfCPqzaOqpsIp87oHmrUMbmXezq38jK+wxsxhPX2M0qc4+f/fvLEjWeQNqL\n"
"PU15zwqgmc8Wil2HRyX9js7X4tmlqPad7+sxI9xxjF6TfGcY/+YinYTj9OA1iEL0\n"
"ohLrdBvpJ6QjGi0/Z0o0KpeyDuyBmF5i4QxyjaoLwyU659owRmyZ4ptfpgVRAgMB\n"
"AAGjggHiMIIB3jAMBgNVHRMBAf8EAjAAMBEGCWCGSAGG+EIBAQQEAwIGQDALBgNV\n"
"HQ8EBAMCBeAwIQYJYIZIAYb4QgENBBQWEkJyb2tlciBDZXJ0aWZpY2F0ZTAdBgNV\n"
"HQ4EFgQUKFnssUm7X8xEdkzCixfDUwS+CwIwgZwGA1UdIwSBlDCBkYAUjdmu8NvW\n"
"XoIfWViDTGH6scG4gZKhbqRsMGoxFzAVBgNVBAMMDkFuIE1RVFQgYnJva2VyMRYw\n"
"FAYDVQQKDA1Pd25UcmFja3Mub3JnMRQwEgYDVQQLDAtnZW5lcmF0ZS1DQTEhMB8G\n"
"CSqGSIb3DQEJARYSbm9ib2R5QGV4YW1wbGUubmV0ggkAswNDmMFTNEwwRAYDVR0R\n"
"BD0wO4cEwKgAhYcQ/oAAAAAAAADji9cNFx/dF4cEfwAAAYcQAAAAAAAAAAAAAAAA\n"
"AAAAAYIJbG9jYWxob3N0MIGGBgNVHSAEfzB9MHsGAysFCDB0MBwGCCsGAQUFBwIB\n"
"FhBodHRwOi8vbG9jYWxob3N0MFQGCCsGAQUFBwICMEgwEBYJT3duVHJhY2tzMAMC\n"
"AQEaNFRoaXMgQ0EgaXMgZm9yIGEgbG9jYWwgTVFUVCBicm9rZXIgaW5zdGFsbGF0\n"
"aW9uIG9ubHkwDQYJKoZIhvcNAQENBQADggEBAHTdKaRp/oM7A7qlav4WVgOhw/Z7\n"
"DrsUUwaM73Am/BcVHLjEP71tV+iDlh1EsvctS2eTsTaBcCrR82nL3/imVV1HiKgg\n"
"nYkJWQPHNUJGOcUklXf5Ge1Mm3TpYF+6JQLJ0LI+fXlgFLmh3eDTPxTuIvetG9ym\n"
"S7oB/JjKEkTL9uMKyVjB/X4NY1MEPR5B0dYSuF7cr2bg5aTTsQNcZJ7ZSbpsP5UV\n"
"l0Km6MD622q1epn9I1EmI6U1YVY1lQADhC603w55BDe//xEEo2z9bV/+z3qvs5j+\n"
"paQvyFq1DF0Y1VtRJYkG4e1jH2yHR7ild9YqQzWSqdW+RUFS0UXXcijSZEE=\n"
"-----END CERTIFICATE-----\n";

/*
 * Private key associated with the client certificate
 */
static const char clientcredentialCLIENT_PRIVATE_KEY_PEM[] =
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpQIBAAKCAQEAwFCBn8FL3a/PM/6k6JMy/7pWKfkK8Ll1YIqJ+Q+assew0xUi\n"
"EUiUmjFmHEKb8YZ/V0HSYuDl36qW/9pfYM3PE4YdqJsf+UlbO41u3uTXtAzllHOs\n"
"tjojh7CcpmfrjGqUJN3Dy3ET2Y39vHLMTDcaJQ9gRK9nKJoXwj6s2jqqbCKfO6B5\n"
"q1DG5l3s6t/IyvsMbMYT19jNKnOPn/37yxI1nkDaiz1Nec8KoJnPFopdh0cl/Y7O\n"
"1+LZpaj2ne/rMSPccYxek3xnGP/mIp2E4/TgNYhC9KIS63Qb6SekIxotP2dKNCqX\n"
"sg7sgZheYuEMco2qC8MlOufaMEZsmeKbX6YFUQIDAQABAoIBAQCmVrvXXwxsob0Z\n"
"Xt5qH6+TyyrtwO+6iN7CcvGlz95hViFWYcgGpP2LZMDKHr7dKBUKdu8vAREy5U0X\n"
"3Vxz0W+TloQeqpX8O3vCxbEo70y2+hS001yGfUDwA01rDSWKXUDnpjzMwXCG0q6u\n"
"Cak1Hs/pUCRXJFOYpbd5FlbRpr5zKfqqtlPgvGcx+qDrkbLTDPhgl7YPUany6nyb\n"
"XxfMDBGWppLxzJBkgu3f5dIpCrbBZEfZle3Rw+2YA/WdP1Hfxw6558nR6ay4NHwW\n"
"3v4MbAc23SaSCxsKCCy7mWGU7ZP2rIqojp7KQW6pAIjJoBFdhstrQ1GhPjh07oXx\n"
"GPOFfgWlAoGBAOn0g03Fp+OPdy8MnK9h3b7IdjaPBME9YCkF07ODtUmJkxFlVxPV\n"
"TVIeB6DtZbapQaBof4uM0lUvvLhbLFAB2D5LpOwP91OzGQQXIoD2rZVvlH4/ONH0\n"
"qp1v99LpCQl0CEuQt8XmFAnrGh+P/+NXDMcSNYDW1yi/Al+QJOBt3bTLAoGBANJv\n"
"iKm+rws+JPdUzf9O5PiRYl1AH1I3x9MXNe7vNk95aWGCSN3o1KUsKglsEthta1oH\n"
"XYNTOM4AoWXrxzNVqSL8wTpjUAs6k8Lhrafbp+5awR82Tfdy7Cp1tXR2hRfoHBvE\n"
"degymIXgrNXYIxQwtxuypn2d7DjGFsNgUkhqn8bTAoGBAJa8TJEiRX6Po9mzhYxW\n"
"QNm4LLoQQZ8DnV4w++pQdNBRjGkL4yPLLYs3//BGpF8hBECGs47FB4uNO0mIrK9L\n"
"0PiqIjNaQh8yaG7DPR7cUJDUmFcTAtf7jKXtSz4fmQv3L5UIQ++Ewup1CJrHW7Yt\n"
"EvV8HO7K/UuqbawGokvbXZorAoGBANF1pTFYlQF61O6/IeBb1ju48pDhL4v169eo\n"
"hc2Tm5qVvhgJx5/iji7ue9UvFr8Igs90/1alcqhSJlS90GS/ggBKV8dXbSgQIV3c\n"
"pq6rTEdLXGlF82s3n+1PhtMq2aWHKttUvQvnuLuhfEfKwHfrWYefVms40xVbzvKg\n"
"IKbmQq1zAoGANclT+zYi6x8Vt8TjpQ+xKEv+oypyATONTovi7S5PJ/GPM6Vv2guR\n"
"yUSdP24srgz0IRI7BC6XFjs9YAqLhEDgbuQVmm1Pw28BBv7Rj37FfRwtpUokJZPt\n"
"BT+tjCPdR5TKgfMj61MlXerse8QYlQ7Ht5UfynBEV4Qrx8ixXC2ojb4=\n"
"-----END RSA PRIVATE KEY-----\n";
#endif
#endif // USE_TLS

#endif // __AWS_CLIENTCREDENTIAL__H__


