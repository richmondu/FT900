# FT900 Amazon DynamoDB client


This project demonstrates FT900 HTTPS connectivity to Amazon DynamoDB by implementing SigV4 Signature authentication on HTTP POST request packets.


FT900 can now add items to AWS DynamoDB database table by sending HTTPS POST request (signed with SigV4 Signature authentication) to Amazon Lambda endpoint.


Instructions:

        1. Update amazon_dynamodb_config.h to include your Amazon security credentials and DynamoDB configurations
            CONFIG_AWS_ACCESS_KEY, CONFIG_AWS_SECRET_KEY
            CONFIG_AWS_DYNAMODB_TABLE
            CONFIG_AWS_REGION, CONFIG_AWS_HOST
        2. Compile, run and verify if text or email has been received


Notes:

Determining the HTTPS POST request required some reverse-engineering using AWS SDK Boto3 Python library. 
Boto3 abstracts low-level implementation of Amazon SNS connectivity 
but Boto3 provides access to BotoCore which allows debugging/printing of the request packet details. 

Using this information, I implemented https://github.com/richmondu/libpyawsbarebone 
which is a barebone Python implementation of Amazon SNS connectivity using plain sockets and encryption. 
This made me familiarize with the step by step procedures on generating the SigV4 Signature.
Once I ported the SigV4 Signature using mbedTLS, the rest was straightforward. 
