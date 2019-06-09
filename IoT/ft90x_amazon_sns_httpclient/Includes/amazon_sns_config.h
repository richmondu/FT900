#ifndef AMAZON_SNS_CONFIG_H
#define AMAZON_SNS_CONFIG_H



// Copy your Amazon security credentials (access key id and secret access key)
// from https://console.aws.amazon.com/iam/home?#/security_credentials
#define CONFIG_AWS_ACCESS_KEY       ""                                                      // update me
#define CONFIG_AWS_SECRET_KEY       ""                                                      // update me

// Create FT900SNStopic on your AWS SNS console account
// and create email subscription to FT900SNStopic
#define CONFIG_AWS_SNS_TOPIC_ARN    ""                                                      // update me
#define CONFIG_AWS_SNS_PHONE_NUMBER "+639175900612"

// Update based on your SNS region
#define CONFIG_AWS_REGION           "ap-southeast-1"                                        // update me
#define CONFIG_AWS_HOST             "sns.ap-southeast-1.amazonaws.com"                      // update me

#define CONFIG_AWS_SERVICE          "sns"
#define CONFIG_AWS_HEADERS          "content-type;host;x-amz-date"
#define CONFIG_AWS_ALGORITHM        "AWS4-HMAC-SHA256"
#define CONFIG_AWS_SIG4_REQUEST     "aws4_request"

#define CONFIG_HTTP_METHOD          "POST"
#define CONFIG_HTTP_API             "/"
#define CONFIG_HTTP_CONTENT_TYPE    "application/x-www-form-urlencoded"
#define CONFIG_HTTP_TLS_PORT        443



#endif // AMAZON_SNS_CONFIG_H
