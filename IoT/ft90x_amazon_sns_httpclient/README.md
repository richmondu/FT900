# FT900 Amazon SNS client


This project demonstrates FT900 HTTPS connectivity to Amazon SNS by implementing SigV4 Signature authentication on HTTP POST request packets sent over secure TLS connection.


FT900 can now send text/email messages by sending HTTPS POST request (signed with SigV4 Signature authentication) to Amazon SNS endpoint.
Amazon SNS provides 2 options:

        1. Send via sms/text [by specifying phone number]
        2. Send via multiple subscribers via topic subscriptions
            topic subscriptions => you can set multiple subscribers to the topic
            can set email, sms, lambda, http subscribers


Instructions:

        1. Update amazon_sns_config.h to include your Amazon security credentials and SNS configurations
            CONFIG_AWS_ACCESS_KEY, CONFIG_AWS_SECRET_KEY
            CONFIG_AWS_SNS_TOPIC_ARN, CONFIG_AWS_SNS_PHONE_NUMBER
            CONFIG_AWS_REGION, CONFIG_AWS_HOST
        2. Update SNS_IS_TEXT in main.c
            1 if send sms/text, 
            0 if send to topic arn [multiple subscribers: SMS/email/lambda/etc]
            Note that these uses CONFIG_AWS_SNS_PHONE_NUMBER and CONFIG_AWS_SNS_TOPIC_ARN
        3. Compile, run and verify if text or email has been received


Sample request packets:

        POST / HTTP/1.1
        Host:sns.ap-southeast-1.amazonaws.com
        Content-Type:application/x-www-form-urlencoded
        X-Amz-Date:20190611T054832Z
        Authorization:AWS4-HMAC-SHA256 Credential=AKIAJIMSC33TIO5IVGLQ/20190611/ap-southeast-1/sns/aws4_request,SignedHeaders=content-type;host;x-amz-date,Signature=0fd240db776171fbb15aa1e8ea911a69059038e2651887b4c6501dc81e1811d6
        Content-Length:95

        Action=Publish&Version=2010-03-31&PhoneNumber=%2B639175900612&Message=Hello+World+from+FT900%21


        POST / HTTP/1.1
        Host:sns.ap-southeast-1.amazonaws.com
        Content-Type:application/x-www-form-urlencoded
        X-Amz-Date:20190611T055208Z
        Authorization:AWS4-HMAC-SHA256 Credential=AKIAJIMSC33TIO5IVGLQ/20190611/ap-southeast-1/sns/aws4_request,SignedHeaders=content-type;host;x-amz-date,Signature=59d9fa63e23ebc7dc79115ee163973a05f17eab19882f79583903833576cb6b8
        Content-Length:140

        Action=Publish&Version=2010-03-31&TopicArn=arn%3Aaws%3Asns%3Aap-southeast-1%3A773510983687%3AFT900SNStopic&Message=Hello+World+from+FT900%21


Notes:

Determining the HTTPS POST request required some reverse-engineering using AWS SDK Boto3 Python library. 
Boto3 abstracts low-level implementation of Amazon SNS connectivity 
but Boto3 provides access to BotoCore which allows debugging/printing of the request packet details. 

Using this information, I implemented https://github.com/richmondu/libpyawsbarebone 
which is a barebone Python implementation of Amazon SNS connectivity using plain sockets and encryption. 
This made me familiarize with the step by step procedures on generating the SigV4 Signature.
Once I ported the SigV4 Signature using mbedTLS, the rest was straightforward. 
