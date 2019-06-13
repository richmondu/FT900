# FT900 Twitter client


This project demonstrates FT900 HTTPS connectivity to Twitter by implementing OAuth Signature authentication on HTTP POST request packets.


FT900 can now post tweets to Twitter by sending HTTPS POST request (signed with OAuth Signature authentication) to Twitter REST API endpoint.


Instructions:

        1. Create a Twitter developer account at https://developer.twitter.com
        2. Create an app.
        3. Copy the keys and tokens: (Used for generating OAuth signature)
           Consumer API keys:
             API key
             API secret key
           Access token & access token secret:
             Access token
             Access token secret
        4. Update twitter_config.h
             CONFIG_TWITTER_CONSUMER_API_KEY
             CONFIG_TWITTER_CONSUMER_SECRET_KEY
             CONFIG_TWITTER_ACCESS_TOKEN
             CONFIG_TWITTER_ACCESS_SECRET
        5. Compile, run and verify if tweet has been posted on your Twitter account
        
        Note that in real-word scenario, access token & access token secret is dynamically retrieved using OAuth handshake. 
        Different access token & access token secret will be provided for different users.


Sample request packet:

        POST /1.1/statuses/update.json HTTP/1.1
        Connection:close
        Content-Type:application/x-www-form-urlencoded
        Authorization:OAuth oauth_consumer_key="AWpC4F23xG33siMsPZR2JX3Jp",oauth_nonce="
        704004537824812392248604814340",oauth_signature="uondjlS6RPlcVNlv05YGaGTn5bQ%3D"
        ,oauth_signature_method="HMAC-SHA1",oauth_timestamp="1560332973",oauth_token="46
        786209-AY4py6wN5OAAdEHYMOifVfJvGXX45K2Kkbgw143qR",oauth_version="1.0"
        Content-Length:40
        Host:api.twitter.com

        status=asdasdasd%20asdasdasd&trim_user=1


        Breakdown:
        1. Message: "asdasdasd asdasdasd"
        2. Signature base string:
        POST&https%3A%2F%2Fapi.twitter.com%2F1.1%2Fstatuses%2Fupdate.json&oauth_consumer
        _key%3DAWpC4F23xG33siMsPZR2JX3Jp%26oauth_nonce%3D704004537824812392248604814340%
        26oauth_signature_method%3DHMAC-SHA1%26oauth_timestamp%3D1560332973%26oauth_toke
        n%3D46786209-AY4py6wN5OAAdEHYMOifVfJvGXX45K2Kkbgw143qR%26oauth_version%3D1.0%26s
        tatus%3Dasdasdasd%2520asdasdasd%26trim_user%3D1
        3. HMAC-SHA1 signature: ba89dd8e54ba44f95c54d96fd396066864e7e5b4
        4. Base64 signature: uondjlS6RPlcVNlv05YGaGTn5bQ=
        5. OAuth Signature: uondjlS6RPlcVNlv05YGaGTn5bQ%3D


References:
1. OAuth, What is it and how it works?

      <a href="https://www.youtube.com/watch?v=SXDce0e3Ue4"
       target="_blank"></a>
       
2. https://developer.twitter.com/en/docs/basics/authentication/guides/authorizing-a-request
