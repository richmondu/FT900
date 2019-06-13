# FT900 Twitter client


This project demonstrates FT900 HTTPS connectivity to Twitter by implementing OAuth Signature authentication on HTTP POST request packets.


FT900 can now post tweets to Twitter by sending HTTPS POST request (signed with OAuth Signature authentication) to Twitter REST API endpoint.


Instructions:

        1. Update twitter_config.h to include your Twitter keys and tokens
            CONFIG_TWITTER_CONSUMER_API_KEY
            CONFIG_TWITTER_CONSUMER_SECRET_KEY
            CONFIG_TWITTER_ACCESS_TOKEN
            CONFIG_TWITTER_ACCESS_SECRET
        2. Compile, run and verify if tweet has been posted on your Twitter account


