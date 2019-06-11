# FT900 Amazon DynamoDB client


This project demonstrates FT900 HTTPS connectivity to Amazon DynamoDB by implementing SigV4 Signature authentication on HTTP POST request packets sent over secure TLS connection.


FT900 can now add items to AWS DynamoDB database table by sending HTTPS POST request (signed with SigV4 Signature authentication) to Amazon Lambda endpoint.


Instructions:

        1. Update amazon_dynamodb_config.h to include your Amazon security credentials and DynamoDB configurations
            CONFIG_AWS_ACCESS_KEY, CONFIG_AWS_SECRET_KEY
            CONFIG_AWS_DYNAMODB_TABLE
            CONFIG_AWS_REGION, CONFIG_AWS_HOST
        2. Compile, run and verify if text or email has been received


Sample request packet:

        POST / HTTP/1.1
        Host:dynamodb.us-east-1.amazonaws.com
        X-Amz-Target:DynamoDB_20120810.PutItem
        Content-Type:application/x-amz-json-1.0
        X-Amz-Date:20190611T060614Z
        Authorization:AWS4-HMAC-SHA256 Credential=AKIAJIMSC33TIO5IVGLQ/20190611/us-east-1/dynamodb/aws4_request,SignedHeaders=content-type;host;x-amz-date;x-amz-target,Signature=9c44740ac1eae7f0ce05c16adc08ec225cc699fd9a1fcd97539d6f7c60fe4dfa
        Content-Length:214

        {"TableName": "GreengrassDashboard-IoTGSDynamoDeviceStatusTable-1JGCAR33OAYSP", "Item": {"deviceId": {"S": "hopper"}, "sensorReading": {"N": "39"}, "batteryCharge": {"N": "-8"}, "batteryDischargeRate": {"N": "2"}}}


Notes:

Determining the HTTPS POST request required some reverse-engineering using AWS SDK Boto3 Python library. 
Boto3 abstracts low-level implementation of Amazon SNS connectivity 
but Boto3 provides access to BotoCore which allows debugging/printing of the request packet details. 

Using this information, I implemented https://github.com/richmondu/libpyawsbarebone 
which is a barebone Python implementation of Amazon SNS connectivity using plain sockets and encryption. 
This made me familiarize with the step by step procedures on generating the SigV4 Signature.
Once I ported the SigV4 Signature using mbedTLS, the rest was straightforward. 
