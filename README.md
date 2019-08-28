# FT900 IoT/Smart Home

This contains the <b>IoT device framework</b>, <b>IoT cloud platform</b>, <b>AWS/GCP/Azure IoT end-to-end demo</b>,  <b>Twitter demo</b> and <b>Amazon Alexa demo</b> applications for FTDI/Bridgetek's FT900 series of memory-constrained microcontrollers (MCUs).


# FT900 Alexa Demo
- This demonstrates using the <b>FT900 microcontroller [or ESP32 microcontroller] as an Amazon Echo Dot device</b>, where users can interact with Alexa via an edge gateway hub.
- This integrates <b>Amazon Alexa Voice Service AVS SDK C++</b> on smart hub device running on Raspberry PI platform and adds <b>Alexa Virtualization feature</b> enabling multiple in-house FT900 MCUs [or ESP32 MCUs] to simultaneously access Alexa (using different Alexa instances and Amazon accounts) and receive audio content (dialogue responses, alerts/alarms/notifications, music/live news/audiobook, etc) with visual display cards.
- This will be integrated to the next iteration of <b>PanL Smart Home Automation</b> (PanL Hub and PanL Display).
- [RPI Alexa Gateway](https://github.com/richmondu/FT900/tree/master/Alexa/Amazon%20Alexa%20Gateway), [FT900 Alexa Client](https://github.com/richmondu/FT900/tree/master/Alexa/Amazon%20Alexa%20Client), [FT900 Alexa Client Simulator](https://github.com/richmondu/FT900/tree/master/Alexa/Amazon%20Alexa%20Client%20Simulator), [ESP32 Alexa Client](https://github.com/richmondu/FT900/tree/master/Alexa/Amazon%20Alexa%20Client%20ESP32)

<img src="https://github.com/richmondu/FT900/blob/master/Alexa/Amazon%20Alexa%20Client/docs/images/system_diagram.jpg" width="623"/>

<img src="https://github.com/richmondu/FT900/blob/master/Alexa/Amazon%20Alexa%20Client/docs/images/system_diagram2.jpg" width="623"/>



# FT900 AWS IoT Sensor Demo [MQTT over TLS]
- This demonstrates secure MQTT connectivity to Amazon AWS IoT Core and AWS Greengrass using MQTT library from Amazon FreeRTOS aka AWS IoT SDK.
- It contains end-to-end demo including lambda functions for backend cloud and local gateway (Greengrass). 
- This was made after I successfully integrated mbedTLS open-source library with optimal settings for memory footprint.
- This was presented in Electronica 2018 https://brtchip.com/Brochures/FT900%20AWS%20IoT%20Demosheet.pdf
  https://github.com/richmondu/FT900/tree/master/IoT/aws_demos_ft_greengrass_lwip_mbedtls

<img src="https://github.com/richmondu/FT900/blob/master/IoT/aws_demos_ft_greengrass_lwip_mbedtls/doc/FT900%20AWS%20IoT%20-%20System%20Architecture.jpeg" width="800"/>



# FT900 AWS/GCP/Azure IoT Sensor Demo [MQTT over TLS]
- This demonstrates <b>secure MQTT connectivity</b> to popular IoT cloud services:
  https://github.com/richmondu/FT900/tree/master/IoT/ft90x_iot_aws_gcp_azure

  <b>Amazon AWS IoT Core</b> [using <b>X.509 certificate authentication</b>] 
  
  <b>Google Cloud IoT Core</b> [using <b>JWT authentication</b>]
  
  <b>Microsoft Azure IoT Hub</b> [using <b>SAS authentication</b> and X.509 certificate authentication] 
  
- These were presented in Embedded World 2019 https://www.ftdichip.com/Embedded%20World%202019.htm

- The IoT library consists of AWS/GCP/Azure IoT cloud connectivity, mbedTLS SSL library integration, X.509 certificate handling/authentication, JWT/SAS access token generation, ciphersuite security-memory footprint tradeoffs, LWIP open-source bug fix contributions, and IoT Project Templates (for FT900 Eclipse IDE).
- Modify USE_MQTT_BROKER to select your chosen cloud platform or local MQTT broker



# FT900 Amazon Services Demo [HTTP over TLS]
- These demonstrate <b>secure HTTPS connectivity</b> (with <b>SigV4 authentication</b>) to access REST APIs of Amazon services.
- This consists of the following demo applications

  <b>FT900 Amazon SNS Client</b> - sending of text/SMS or email messages
  https://github.com/richmondu/FT900/tree/master/IoT/ft90x_amazon_sns_httpclient
  
  <b>FT900 Amazon Lambda Client</b> - invoking serverless cloud function
  https://github.com/richmondu/FT900/tree/master/IoT/ft90x_amazon_lambda_httpclient
  
  <b>FT900 Amazon DynamoDB Client</b> - adding items to a database table
  https://github.com/richmondu/FT900/tree/master/IoT/ft90x_amazon_dynamodb_httpclient
  
  <b>FT900 Amazon IoT Core Client</b> - publishing sensor data
  https://github.com/richmondu/FT900/tree/master/IoT/ft90x_amazon_iot_httpclient

<img src="https://github.com/richmondu/FT900/blob/master/IoT/ft90x_iot_aws_gcp_azure/doc/FT900_Amazon_HTTPS_REST_APIs.png" width="800"/>



# FT900 Twitter Demo [HTTP over TLS]
- This demonstrates <b>secure HTTPS connectivity</b> (with <b>OAuth authentication</b>) to access REST APIs of Twitter.
- The application demonstrates posting tweets to Twitter.
  https://github.com/richmondu/FT900/tree/master/IoT/ft90x_twitter_httpclient
