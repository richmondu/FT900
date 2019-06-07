# FT900 Amazon SNS client


This project demonstrates FT900 HTTPS connectivity to Amazon SNS by implementing SigV4 Signature authentication on HTTP POST request packets.


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