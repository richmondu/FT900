#ifndef AMAZON_DYNAMODB_CONFIG_H
#define AMAZON_DYNAMODB_CONFIG_H



// Copy your Amazon security credentials (access key id and secret access key)
// from https://console.aws.amazon.com/iam/home?#/security_credentials
#define CONFIG_AWS_ACCESS_KEY       ""                                                      // update me
#define CONFIG_AWS_SECRET_KEY       ""                                                      // update me

// Create your database table on your AWS DynamoDB console account
#define CONFIG_AWS_DYNAMODB_TABLE   ""                                                      // update me

// Update based on your AWS DynamoDB region
#define CONFIG_AWS_REGION           "us-east-1"                                             // update me
#define CONFIG_AWS_HOST             "dynamodb.us-east-1.amazonaws.com"                      // update me

#define CONFIG_AWS_SERVICE          "dynamodb"
#define CONFIG_AWS_HEADERS          "content-type;host;x-amz-date;x-amz-target"
#define CONFIG_AWS_ALGORITHM        "AWS4-HMAC-SHA256"
#define CONFIG_AWS_SIG4_REQUEST     "aws4_request"

#define CONFIG_HTTP_METHOD          "POST"
#define CONFIG_HTTP_API             "/"
#define CONFIG_HTTP_CONTENT_TYPE    "application/x-amz-json-1.0"
#define CONFIG_HTTP_TLS_PORT        443
#define CONFIG_HTTP_TARGET          "DynamoDB_20120810.PutItem"



#endif // AMAZON_DYNAMODB_CONFIG_H
