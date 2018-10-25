#include "ft900.h"
#include "FreeRTOS.h"   // For pvPortMalloc
#include "iot_config.h" // For USE_MQTT_BROKER



static const inline
uint8_t* read_file(__flash__ uint8_t* data, __flash__ uint8_t* data_end, size_t* len)
{
    uint8_t *buf = NULL;
    size_t buf_len;

    buf_len = (data_end - data) + 1;
    buf = pvPortMalloc(buf_len);
    if (buf == NULL)
    {
        *len = 0;
        return NULL;
    }
    memcpy_pm2dat(buf, data, buf_len-1);
    buf[buf_len-1] = '\0';
    *len = buf_len;

    return buf;
}

const uint8_t* iot_certificate_getca(size_t* len)
{
#if USE_ROOT_CA
    return read_file(ca_data, ca_data_end, len);
#else
    *len = 0;
    return NULL;
#endif
}

const uint8_t* iot_certificate_getcert(size_t* len)
{
    return read_file(cert_data, cert_data_end, len);
}

const uint8_t* iot_certificate_getpkey(size_t* len)
{
    return read_file(pkey_data, pkey_data_end, len);
}

#if (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT && MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)
const uint8_t* iot_sas_getkey(size_t* len)
{
    return read_file(sas_data, sas_data_end, len);
}
#endif // (USE_MQTT_BROKER == MQTT_BROKER_MAZ_IOT && MAZ_AUTH_TYPE == AUTH_TYPE_SASTOKEN)

