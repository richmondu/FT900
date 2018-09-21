#include "ft900.h"
#include "FreeRTOS.h" // For pvPortMalloc
#include "iot_config.h"      // For USE_MQTT_BROKER



static const
uint8_t* certificate_get(__flash__ uint8_t* data, __flash__ uint8_t* data_end, size_t* len)
{
    uint8_t *buf = NULL;
    size_t buf_len;

    buf_len = (data_end - data);
    buf = pvPortMalloc(buf_len);
    if (buf == NULL)
    {
        *len = 0;
        return NULL;
    }
    memcpy_pm2dat(buf, data, buf_len);
    buf[buf_len] = '\0';
    *len = buf_len;

    return buf;
}

const uint8_t* iot_certificate_getca(size_t* len)
{
#if USE_ROOT_CA
    return certificate_get(ca_data, ca_data_end, len);
#else
    *len = 0;
    return NULL;
#endif
}

const uint8_t* iot_certificate_getcert(size_t* len)
{
    return certificate_get(cert_data, cert_data_end, len);
}

const uint8_t* iot_certificate_getpkey(size_t* len)
{
    return certificate_get(pkey_data, pkey_data_end, len);
}

