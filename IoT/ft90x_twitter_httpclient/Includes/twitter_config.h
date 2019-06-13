#ifndef TWITTER_CONFIG_H
#define TWITTER_CONFIG_H



//###############################################################################

#define CONFIG_TWITTER_CONSUMER_API_KEY    ""                                                   // update me
#define CONFIG_TWITTER_CONSUMER_SECRET_KEY ""                                                   // update me
#define CONFIG_TWITTER_ACCESS_TOKEN        ""                                                   // update me
#define CONFIG_TWITTER_ACCESS_SECRET       ""                                                   // update me

//###############################################################################

#define CONFIG_HTTP_METHOD                 "POST"
#define CONFIG_HTTP_API                    "/1.1/statuses/update.json"
#define CONFIG_HTTP_VERSION                "HTTP/1.1"
#define CONFIG_HTTP_ACCEPT                 "*/*"
#define CONFIG_HTTP_CONNECTION             "close"
#define CONFIG_HTTP_CONTENT_TYPE           "application/x-www-form-urlencoded"
#define CONFIG_HTTP_AUTHORIZATION          "OAuth"
#define CONFIG_HTTP_OAUTH_ALGORITHM        "HMAC-SHA1"
#define CONFIG_HTTP_OAUTH_VERSION          "1.0"
#define CONFIG_HOST                        "api.twitter.com"
#define CONFIG_PORT                        443
//#define CONFIG_MAX_RECV_SIZE               512
//#define CONFIG_TLS_CACERTIFICATE           "twitter_ca_cert.pem"

//###############################################################################



#endif // TWITTER_CONFIG_H
