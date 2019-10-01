/*
 * ============================================================================
 * History
 * =======
 * 29 Oct 2018 : Created
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

#include "ft900.h"
#include "FreeRTOS.h"   // For pvPortMalloc
#include <iot_config.h> // For USE_MQTT_BROKER



static const inline
uint8_t* read_file(__flash__ uint8_t* data, __flash__ uint8_t* data_end, size_t* len)
{
    uint8_t *buf = NULL;
    size_t buf_len;

    buf_len = data_end - data;
    buf = pvPortMalloc(buf_len);
    if (buf == NULL)
    {
        *len = 0;
        return NULL;
    }
    memcpy_pm2dat(buf, data, buf_len-1);
    buf[buf_len-1] = '\0';
    *len = buf_len;

    //tfp_printf("len=%d %d\r\n", buf_len, strlen(buf));
    return buf;
}

const uint8_t* iot_certificate_getca(size_t* len)
{
#if USE_ROOT_CA
#if 0
    extern __flash__ uint8_t IOT_CLIENTCREDENTIAL_CA_CERTIFICATE[]      asm("_binary____Certificates_rootca_pem_start");
    extern __flash__ uint8_t IOT_CLIENTCREDENTIAL_CA_CERTIFICATE_END[]  asm("_binary____Certificates_rootca_pem_end");

    int ret = IOT_CLIENTCREDENTIAL_CA_CERTIFICATE_END - IOT_CLIENTCREDENTIAL_CA_CERTIFICATE + 1;
    char *ca_cert = pvPortMalloc(ret);// + 1);
    if (!ca_cert) {
        return NULL;
    }
    memcpy_pm2dat(ca_cert, IOT_CLIENTCREDENTIAL_CA_CERTIFICATE, ret-1);
    ca_cert[ret-1] = '\0';
    tfp_printf("ret=%d %d\r\n", ret, strlen(ca_cert));
    tfp_printf("%s\r\n", ca_cert);
    *len = ret;
    return ca_cert;
#else
    return read_file(ca_data, ca_data_end, len);
#endif
#else
    *len = 0;
    return NULL;
#endif
}

const uint8_t* iot_certificate_getcert(size_t* len)
{
#if (USE_MQTT_BROKER != MQTT_BROKER_UNKNOWN)
#if 0
    extern __flash__ uint8_t IOT_CLIENTCREDENTIAL_DEVICE_CERTIFICATE[]      asm("_binary____Certificates_ft900device1_cert_pem_start");
    extern __flash__ uint8_t IOT_CLIENTCREDENTIAL_DEVICE_CERTIFICATE_END[]  asm("_binary____Certificates_ft900device1_cert_pem_end");

    int ret = IOT_CLIENTCREDENTIAL_DEVICE_CERTIFICATE_END - IOT_CLIENTCREDENTIAL_DEVICE_CERTIFICATE + 1;
    char *cert_data = pvPortMalloc(ret);// + 1);
    if (!cert_data) {
        return NULL;
    }
    memcpy_pm2dat(cert_data, IOT_CLIENTCREDENTIAL_DEVICE_CERTIFICATE, ret-1);
    cert_data[ret-1] = '\0';
    tfp_printf("ret=%d %d\r\n", ret, strlen(cert_data));
    tfp_printf("%s\r\n", cert_data);
    *len = ret;
    return cert_data;
#else
	return read_file(cert_data, cert_data_end, len);
#endif
#else
    *len = 0;
    return NULL;
#endif
}

const uint8_t* iot_certificate_getpkey(size_t* len)
{
#if (USE_MQTT_BROKER != MQTT_BROKER_UNKNOWN)
#if 0
    extern __flash__ uint8_t IOT_CLIENTCREDENTIAL_DEVICE_PRIVATEKEY[]      asm("_binary____Certificates_ft900device1_pkey_pem_start");
    extern __flash__ uint8_t IOT_CLIENTCREDENTIAL_DEVICE_PRIVATEKEY_END[]  asm("_binary____Certificates_ft900device1_pkey_pem_end");

    int ret = IOT_CLIENTCREDENTIAL_DEVICE_PRIVATEKEY_END - IOT_CLIENTCREDENTIAL_DEVICE_PRIVATEKEY + 1;
    char *pkey_data = pvPortMalloc(ret);// + 1);
    if (!pkey_data) {
        return NULL;
    }
    memcpy_pm2dat(pkey_data, IOT_CLIENTCREDENTIAL_DEVICE_PRIVATEKEY, ret-1);
    pkey_data[ret-1] = '\0';
    tfp_printf("ret=%d %d\r\n", ret, strlen(pkey_data));
    tfp_printf("%s\r\n", pkey_data);
    *len = ret;
    return pkey_data;
#else
    return read_file(pkey_data, pkey_data_end, len);
#endif
#else
    *len = 0;
    return NULL;
#endif
}

#if (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT && MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)
const uint8_t* iot_sas_getkey(size_t* len)
{
    return read_file(sas_data, sas_data_end, len);
}
#endif // (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT && MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)

