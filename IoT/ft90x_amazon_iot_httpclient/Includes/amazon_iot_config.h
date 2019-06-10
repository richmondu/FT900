#ifndef AMAZON_IOT_CONFIG_H
#define AMAZON_IOT_CONFIG_H



// Copy your Amazon security credentials (access key id and secret access key)
// from https://console.aws.amazon.com/iam/home?#/security_credentials
#define CONFIG_AWS_ACCESS_KEY       ""                                                      // update me
#define CONFIG_AWS_SECRET_KEY       ""                                                      // update me

// Update based on your AWS IoT region
#define CONFIG_AWS_REGION           "us-east-1"                                             // update me
#define CONFIG_AWS_HOST             "data.iot.us-east-1.amazonaws.com"                      // update me

#define CONFIG_AWS_SERVICE          "iotdata"
#define CONFIG_AWS_HEADERS          "host;x-amz-date"
#define CONFIG_AWS_ALGORITHM        "AWS4-HMAC-SHA256"
#define CONFIG_AWS_SIG4_REQUEST     "aws4_request"

#define CONFIG_HTTP_METHOD          "POST"
#define CONFIG_HTTP_API             "/topics/"
#define CONFIG_HTTP_CONTENT_TYPE    "application/json"
#define CONFIG_HTTP_TLS_PORT        443



#endif // AMAZON_IOT_CONFIG_H
