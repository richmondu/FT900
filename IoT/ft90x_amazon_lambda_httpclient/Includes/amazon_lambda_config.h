#ifndef AMAZON_LAMBDA_CONFIG_H
#define AMAZON_LAMBDA_CONFIG_H



// Copy your Amazon security credentials (access key id and secret access key)
// from https://console.aws.amazon.com/iam/home?#/security_credentials
#define CONFIG_AWS_ACCESS_KEY       ""                                                      // update me
#define CONFIG_AWS_SECRET_KEY       ""                                                      // update me

// Create FT900LambdaToSNS on your AWS Lambda console account
// and implement the Lambda function to trigger SNS
#define CONFIG_AWS_LAMBDA_FUNCTION  "FT900LambdaToSNS"                                      // update me

// Create FT900SNStopic on your AWS SNS console account
// and create email subscription to FT900SNStopic
#define CONFIG_AWS_SNS_TOPIC_ARN    ""                                                      // update me

// Update based on your SNS region
#define CONFIG_AWS_REGION           "ap-southeast-1"                                        // update me
#define CONFIG_AWS_HOST             "lambda.ap-southeast-1.amazonaws.com"                   // update me

#define CONFIG_AWS_SERVICE          "lambda"
#define CONFIG_AWS_HEADERS          "content-type;host;x-amz-date"
#define CONFIG_AWS_ALGORITHM        "AWS4-HMAC-SHA256"
#define CONFIG_AWS_SIG4_REQUEST     "aws4_request"

#define CONFIG_HTTP_METHOD          "POST"
#define CONFIG_HTTP_API             "/2015-03-31/functions/%s/invocations"
#define CONFIG_HTTP_CONTENT_TYPE    "application/json"
#define CONFIG_HTTP_TLS_PORT        443



#endif // AMAZON_LAMBDA_CONFIG_H
