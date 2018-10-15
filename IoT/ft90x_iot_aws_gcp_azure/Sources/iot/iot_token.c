#include <string.h>
#include "ft900.h"

/* tinyprintf Header. */
#include "tinyprintf.h"       // For tfp_printf, tfp_snprintf

/* FreeRTOS Headers. */
#include "FreeRTOS.h"         // For pvPortMalloc

/* mbedTLS Headers. */
#include "mbedtls/pk.h"       // For mbedtls_pk_xxx

/* LWIP Headers. */
#include "lwipopts.h"         // For ALTCP_MBEDTLS_ENTROPY_xxx

/* LWIP Headers. */
#include "iot.h"              // For ALTCP_MBEDTLS_ENTROPY_xxx



//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define DEBUG_PRINTF(...)
#endif




char* urlEncode(const char* data, int len)
{
    char* out = NULL;
    int count = 0;

    for (int i=0; i<len; i++) {
        if (data[i]=='/' || data[i]=='+' || data[i]=='=')
            count++;
    }

    int size = len + 1 + count*2;
    out = pvPortMalloc(size);
    memset(out, 0, size);

    for (int i=0,j=0; i<len; i++) {
        if (data[i]=='/') {
            strcat(out, "%2F");
            j+=3;
        }
        else if (data[i]=='+') {
            strcat(out, "%2B");
            j+=3;
        }
        else if (data[i]=='=') {
            strcat(out, "%3D");
            j+=3;
        }
        else {
            out[j++] = data[i];
        }
    }

    return out;
}

static const char base64en[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '-', '_',
};

static int base64url_encode(const unsigned char *in, unsigned int inlen, char *out)
{
	unsigned int i, j;

	for (i = j = 0; i < inlen; i++) {
		int s = i % 3;
		switch (s) {
			case 0:
				out[j++] = base64en[(in[i] >> 2) & 0x3F];
				continue;
			case 1:
				out[j++] = base64en[((in[i-1] & 0x3) << 4) + ((in[i] >> 4) & 0xF)];
				continue;
			case 2:
				out[j++] = base64en[((in[i-1] & 0xF) << 2) + ((in[i] >> 6) & 0x3)];
				out[j++] = base64en[in[i] & 0x3F];
		}
	}

	i -= 1;
	if ((i % 3) == 0) {
		out[j++] = base64en[(in[i] & 0x3) << 4];
		//out[j++] = BASE64_PAD;
		//out[j++] = BASE64_PAD;
	} else if ((i % 3) == 1) {
		out[j++] = base64en[(in[i] & 0xF) << 2];
		//out[j++] = BASE64_PAD;
	}
	out[j++] = 0;

	return 0;
}

//
// Generate Shared Access Signature (SAS) used for Microsoft Azure IoT Hub
//
char* token_create_sas(const char* resourceUri, const char* sharedAccessKey, uint32_t timeNow)
{
    char pcTemp[128] = {0};
    size_t olen = 0;
    int ret = 0;
    char* token = NULL;
    char* decodedSharedAccessKey = NULL;
    char* encodedResourceUri = NULL;
    char* encodedHashedRequest = NULL;
    char* dataToSign = NULL;


    if (resourceUri == NULL || sharedAccessKey == NULL || timeNow == 0) {
        DEBUG_PRINTF("token_create_sas failed! Invalid parameters!\r\n");
        return NULL;
    }

    //
    // Decode the sharedAccessKey
    //
    memset(pcTemp, 0, sizeof(pcTemp));
    ret = mbedtls_base64_decode((unsigned char*)pcTemp, sizeof(pcTemp), &olen, (const unsigned char*)sharedAccessKey, strlen(sharedAccessKey));
    if (ret != 0) {
        DEBUG_PRINTF("token_create_sas failed! mbedtls_base64_decode: %d (-0x%x)\r\n", ret, -ret);
        goto cleanup;
    }
    int decodedSharedAccessKeyLen = olen;
    decodedSharedAccessKey = pvPortMalloc(decodedSharedAccessKeyLen+1);
    if (decodedSharedAccessKey == NULL) {
        DEBUG_PRINTF("token_create_sas failed! pvPortMalloc decodedSharedAccessKey\r\n");
        goto cleanup;
    }
    memset(decodedSharedAccessKey, 0, decodedSharedAccessKeyLen+1);
    memcpy(decodedSharedAccessKey, pcTemp, decodedSharedAccessKeyLen);
    DEBUG_PRINTF("sharedAccessKey: %s [%d]\r\n", sharedAccessKey, strlen(sharedAccessKey));
    DEBUG_PRINTF("decoded SharedAccessKey: [%d][%d]\r\n", decodedSharedAccessKeyLen, olen);


    //
    // Encode the resourceUri
    //
    encodedResourceUri = urlEncode(resourceUri, strlen(resourceUri));
    if (encodedResourceUri == NULL) {
        DEBUG_PRINTF("token_create_sas failed! urlEncode\r\n");
        goto cleanup;
    }
    int encodedResourceUriLen = strlen(encodedResourceUri);
    DEBUG_PRINTF("resourceUri: %s [%d]\r\n", resourceUri, strlen(resourceUri));
    DEBUG_PRINTF("encoded ResourceUri: [%s][%d]\r\n", encodedResourceUri, encodedResourceUriLen);


    //
    // Generate data to sign
    //
    uint32_t expiry = timeNow + 3600*12; // Set the expiry time after 12 hours.
    int dataToSignLen = encodedResourceUriLen + 1 + strlen("\n") + 12;
    dataToSign = pvPortMalloc(dataToSignLen);
    if (dataToSign == NULL) {
        DEBUG_PRINTF("token_create_sas failed! pvPortMalloc dataToSign\r\n");
        goto cleanup;
    }
    memset(dataToSign, 0, dataToSignLen);
    dataToSignLen = tfp_snprintf(dataToSign, dataToSignLen, "%s\n%u", encodedResourceUri, (unsigned int)expiry);
    DEBUG_PRINTF("dataToSign: %s [%d]\r\n", dataToSign, dataToSignLen);


    uint8_t hashedRequest[64+1] = {0};
    {
        mbedtls_md_context_t md_ctx;
        mbedtls_md_init(&md_ctx);

        if ((ret = mbedtls_md_setup(&md_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1)) != 0) {
            DEBUG_PRINTF("HMAC setup failed! returned %d (-0x%04x)\r\n", ret, -ret);
            mbedtls_md_free(&md_ctx);
            goto cleanup;
        }

        if ((ret = mbedtls_md_hmac_starts(&md_ctx, (unsigned char*)decodedSharedAccessKey, decodedSharedAccessKeyLen)) != 0) {
            DEBUG_PRINTF("HMAC starts failed! returned %d (-0x%04x)\r\n", ret, -ret);
            mbedtls_md_free(&md_ctx);
            goto cleanup;
        }

        if ((ret = mbedtls_md_hmac_update(&md_ctx, (unsigned char*)dataToSign, dataToSignLen)) != 0) {
            DEBUG_PRINTF("HMAC update failed! returned %d (-0x%04x)\r\n", ret, -ret);
            mbedtls_md_free(&md_ctx);
            goto cleanup;
        }
        DEBUG_PRINTF("mbedtls_md_get_size: %d\r\n", mbedtls_md_get_size(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256)));

        if ((ret = mbedtls_md_hmac_finish(&md_ctx, hashedRequest)) != 0) {
            DEBUG_PRINTF("HMAC update failed! returned %d (-0x%04x)\r\n", ret, -ret);
            mbedtls_md_free(&md_ctx);
            goto cleanup;
        }
        mbedtls_md_free(&md_ctx);
    }

    memset(pcTemp, 0, sizeof(pcTemp));
    if ((ret = mbedtls_base64_encode((unsigned char *)pcTemp, sizeof(pcTemp), &olen, hashedRequest, 32)) != 0) {
        DEBUG_PRINTF("Encoding device key failed! returned %d (-0x%04x)\r\n", ret, -ret);
        goto cleanup;
    }
    DEBUG_PRINTF("hashedRequest: %s [%d]\r\n", hashedRequest, sizeof(hashedRequest));
    encodedHashedRequest = urlEncode(pcTemp, strlen(pcTemp));
    if (encodedHashedRequest == NULL) {
        DEBUG_PRINTF("token_create_sas failed! urlEncode\r\n");
        goto cleanup;
    }
    int encodedHashedRequestLen = strlen(encodedHashedRequest);
    DEBUG_PRINTF("Encoded hashedRequest %s [%d] [%d] \r\n\r\n", pcTemp, strlen(pcTemp), olen);


    //
    // Construct the token
    //
    olen = 34 + encodedResourceUriLen + encodedHashedRequestLen + 10 + 1;
    token = pvPortMalloc(olen);
    if (token == NULL)
    {
        DEBUG_PRINTF("pvPortMalloc failed!\r\n");
        goto cleanup;
    }
    memset(token, 0, olen);
    ret = tfp_snprintf(token, olen, "SharedAccessSignature sr=%s&sig=%s&se=%u", encodedResourceUri, encodedHashedRequest, (unsigned int)expiry);
    DEBUG_PRINTF("data %s [%d] [%d] [%d] \r\n\r\n", token, strlen(token), ret, olen);


cleanup:
    vPortFree(encodedHashedRequest);
    vPortFree(decodedSharedAccessKey);
    vPortFree(encodedResourceUri);
    vPortFree(dataToSign);
    return token;
}
//
// Generate JSON Web Token (JWT) used for Google Cloud IoT Core
//
char* token_create_jwt(const char* projectId, const uint8_t* privateKey, size_t privateKeySize, uint32_t timeNow)
{
    int len = 512;//464; // TODO
    char pcTemp[352] = {0};
    char* pcJWT = NULL;
    size_t olen = 0;


    if (privateKey == NULL || privateKeySize == 0 || timeNow == 0) {
        DEBUG_PRINTF("token_create_jwt failed! Invalid parameters!\r\n");
        return NULL;
    }

    pcJWT = (char*)pvPortMalloc(len);
    if (pcJWT == NULL) {
        DEBUG_PRINTF("token_create_jwt failed! malloc FAILED!\r\n");
        return NULL;
    }
    memset(pcJWT, 0, len);


    //
    // JWT Header
    //

    // Create header
    const char pcHdr[] = "{\"typ\":\"JWT\",\"alg\":\"RS256\"}";
    DEBUG_PRINTF("pcHdr %s %d\r\n", pcHdr, strlen(pcHdr));

    // Encode header

    base64url_encode((unsigned char *)pcHdr, strlen(pcHdr), pcTemp);
    DEBUG_PRINTF("Encoded pcHdr %s %d\r\n\r\n", pcTemp, strlen(pcTemp));

    // Build JWT packet
    strcat(pcJWT, pcTemp);
    strcat(pcJWT, ".");


    //
    // JWT Body
    //

    // python time.mktime(datetime.datetime.now().timetuple())
    uint32_t iat = timeNow;    // Set the time.
    uint32_t exp = iat + 3600*12; // Set the expiry time after 1 hour.
    char pcBody[64] = {0};
    memset(pcBody, 0, sizeof(pcBody));
    tfp_snprintf(pcBody, sizeof(pcBody), "{\"iat\":%u,\"exp\":%u,\"aud\":\"%s\"}", (unsigned int)iat, (unsigned int)exp, projectId);
    DEBUG_PRINTF("pcBody %s %d\r\n", pcBody, strlen(pcBody));

    // Encode body
    memset(pcTemp, 0, sizeof(pcTemp));
    base64url_encode((unsigned char *)pcBody, strlen(pcBody), pcTemp);
    DEBUG_PRINTF("Encoded pcBody %s %d\r\n\r\n", pcTemp, strlen(pcTemp));

    // Build JWT packet
    strcat(pcJWT, pcTemp);


    //
    // JWT Signature
    //

    uint8_t pcSignature[256+1] = {0};
    mbedtls_pk_context pk_context;
    size_t retSize;


    mbedtls_pk_init(&pk_context);
    int rc = mbedtls_pk_parse_key(&pk_context, privateKey, privateKeySize, NULL, 0);
    if (rc != 0) {
        DEBUG_PRINTF("token_create_jwt failed! mbedtls_pk_parse_key: %d (-0x%x)\r\n", rc, -rc);
        mbedtls_pk_free(&pk_context);
        return NULL;
    }

    memset(pcTemp, 0, sizeof(pcTemp));
    rc = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), (unsigned char*)pcJWT, strlen((char*)pcJWT), (unsigned char*)pcTemp);
    if (rc != 0) {
        DEBUG_PRINTF("token_create_jwt failed! mbedtls_md: %d (-0x%x)\r\n", rc, -rc);
        mbedtls_pk_free(&pk_context);
        return NULL;
    }

    // Create signature
    rc = mbedtls_pk_sign(&pk_context, MBEDTLS_MD_SHA256, (unsigned char*)pcTemp, 32, pcSignature, &retSize, NULL, NULL);
    if (rc != 0) {
        DEBUG_PRINTF("token_create_jwt failed! mbedtls_pk_sign: %d (-0x%x)\r\n", rc, -rc);
        mbedtls_pk_free(&pk_context);
        return NULL;
    }
    //DEBUG_PRINTF("pcSignature %d %d\r\n", strlen(pcSignature), retSize);
    mbedtls_pk_free(&pk_context);

    // Encode signature
    base64url_encode((unsigned char *)pcSignature, retSize, pcTemp);
    //DEBUG_PRINTF("Encoded pcSignature %s %d\r\n\r\n", pcTemp, strlen(pcTemp));

    // Build JWT packet
    strcat(pcJWT, ".");
    strcat(pcJWT, pcTemp);
    DEBUG_PRINTF("%s %d\r\n\r\n", pcJWT, strlen(pcJWT));

    return pcJWT;
}


void token_free(char** token)
{
    if (*token)
    {
        vPortFree(*token);
        *token = NULL;
    }
}

