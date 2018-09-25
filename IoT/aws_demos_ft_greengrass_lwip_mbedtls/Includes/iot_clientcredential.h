#ifndef __IOT_CLIENTCREDENTIAL__H__
#define __IOT_CLIENTCREDENTIAL__H__

#include "iot_config.h"



/*
 * MQTT Broker.
 * Switch for MQTT message broker to be used
 */
#define MQTT_BROKER_MOSQUITTO       0    // local Mosquitto broker
#define MQTT_BROKER_AWS_GREENGRASS  1    // local AWS Greengrass broker
#define MQTT_BROKER_AWS_IOT         2    // AWS IoT cloud
#if IOT_CONFIG_USE_TLS
    #define USE_MQTT_BROKER         MQTT_BROKER_AWS_GREENGRASS
    //#define USE_MQTT_BROKER         MQTT_BROKER_AWS_IOT
    //#define USE_MQTT_BROKER         MQTT_BROKER_MOSQUITTO
#else // IOT_CONFIG_USE_TLS
    #define USE_MQTT_BROKER         MQTT_BROKER_MOSQUITTO
#endif // IOT_CONFIG_USE_TLS


/*
 * MQTT Broker endpoint.
 */
#if IOT_CONFIG_USE_TLS
    #if (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
        static const char IOT_CLIENTCREDENTIAL_BROKER_ENDPOINT[] = "192.168.22.16"; // local Greengrass server
    #elif (USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT)
        static const char IOT_CLIENTCREDENTIAL_BROKER_ENDPOINT[] = "amasgua12bmkv.iot.us-east-1.amazonaws.com";
    #elif (USE_MQTT_BROKER == MQTT_BROKER_MOSQUITTO)
        static const char IOT_CLIENTCREDENTIAL_BROKER_ENDPOINT[] = "192.168.22.3"; // local mosquitto server
    #endif
#else // IOT_CONFIG_USE_TLS
    static const char IOT_CLIENTCREDENTIAL_BROKER_ENDPOINT[] = "192.168.22.3"; // local mosquitto server
#endif // IOT_CONFIG_USE_TLS


/*
 * MQTT Broker port number.
 */
#if IOT_CONFIG_USE_TLS
    #define IOT_CLIENTCREDENTIAL_BROKER_PORT 8883
#else // IOT_CONFIG_USE_TLS
    #define IOT_CLIENTCREDENTIAL_BROKER_PORT 1883
#endif // IOT_CONFIG_USE_TLS


/*
 * MQTT Client ID.
 * When using AWS Greengrass, this must be the thing name associated with the certificate
 * When using AWS IoT, this must also be the thing name if a thing name was associated to the certificate
 */
#define IOT_CLIENTCREDENTIAL_CLIENT_ID "HelloWorld_Publisher"


/*
 * MQTT Client CA certificate.
 * Root CA certificate for the client certificate and private key to be used
 * Note that the Root CA certificate for AWS IoT and AWS Greengrass are different.
 * The Root CA certification for AWS Greengrass is the Greegrass Group CA certificate
 * which expires 7 days by default but configurable to 30 days maximum.
 */
#if IOT_CONFIG_USE_TLS
#if IOT_CONFIG_USE_ROOTCA
#if IOT_CONFIG_USE_CERT_OPTIMIZATION
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    // CA certificate must be named ggca.crt and placed at certificates folder
    // Compiler will implicitly define this strings as markers in the object file
    #define IOT_CLIENTCREDENTIAL_CA_CERTIFICATE_NAME     "_binary____Certificates_ggca_crt_start"
    #define IOT_CLIENTCREDENTIAL_CA_CERTIFICATE_END_NAME "_binary____Certificates_ggca_crt_end"
#else
    // CA certificate must be named ca.crt and placed at certificates folder
    // Compiler will implicitly define this strings as markers in the object file
    #define IOT_CLIENTCREDENTIAL_CA_CERTIFICATE_NAME     "_binary____Certificates_ca_crt_start"
    #define IOT_CLIENTCREDENTIAL_CA_CERTIFICATE_END_NAME "_binary____Certificates_ca_crt_end"
#endif
#else // IOT_CONFIG_USE_CERT_OPTIMIZATION
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS)
    static const char IOT_CLIENTCREDENTIAL_CA_CERTIFICATE[] =
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
    static const char IOT_CLIENTCREDENTIAL_CA_CERTIFICATE[] =
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
    static const char IOT_CLIENTCREDENTIAL_CA_CERTIFICATE[] =
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
#endif // IOT_CONFIG_USE_CERT_OPTIMIZATION
#endif // IOT_CONFIG_USE_ROOTCA
#endif // IOT_CONFIG_USE_TLS


#if IOT_CONFIG_USE_TLS
#if (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS || USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT)

/*
 * MQTT Client certificate.
 * Client certificate signed by the root CA certificate
 */
#if IOT_CONFIG_USE_CERT_OPTIMIZATION
    // Certificate must be named cert.crt and placed at certificates folder
    // Compiler will implicitly define these strings as markers in the object file
    #define IOT_CLIENTCREDENTIAL_CERTIFICATE_NAME     "_binary____Certificates_cert_crt_start"
    #define IOT_CLIENTCREDENTIAL_CERTIFICATE_END_NAME "_binary____Certificates_cert_crt_end"
#else // IOT_CONFIG_USE_CERT_OPTIMIZATION
    static const char IOT_CLIENTCREDENTIAL_CERTIFICATE[] =
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
#endif // IOT_CONFIG_USE_CERT_OPTIMIZATION

/*
 * MQTT Client private key.
 * Private key associated with the client certificate
 */
#if IOT_CONFIG_USE_CERT_OPTIMIZATION
    // Certificate must be named cert.key and placed at certificates folder
    // Compiler will implicitly define these strings as markers in the object file
    #define IOT_CLIENTCREDENTIAL_PRIVATEKEY_NAME     "_binary____Certificates_cert_key_start"
    #define IOT_CLIENTCREDENTIAL_PRIVATEKEY_END_NAME "_binary____Certificates_cert_key_end"
#else // IOT_CONFIG_USE_CERT_OPTIMIZATION
    static const char IOT_CLIENTCREDENTIAL_PRIVATEKEY[] =
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
#endif // IOT_CONFIG_USE_CERT_OPTIMIZATION
#endif // (USE_MQTT_BROKER == MQTT_BROKER_AWS_GREENGRASS || USE_MQTT_BROKER == MQTT_BROKER_AWS_IOT)
#endif // IOT_CONFIG_USE_TLS


#endif // __IOT_CLIENTCREDENTIAL__H__
