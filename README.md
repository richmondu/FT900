# FT900 IoT Security and Cloud Connectivity

I handle IoT security and cloud connectivity for FT900 series of microcontrollers.

- IoT Security: IoT framework including mbedTLS integration, SSL certificate authentication, JWT/SAS security token generation, ciphersuite-memory tradeoffs, LWIP open-source contributions, and IoT Project Templates (for FT900 Eclipse IDE)
- IoT Connectivity: Secure authentication and communication with IoT cloud platforms: Amazon Web Services, Google Cloud Platform and Microsoft Azure.
- IoT Solutions: Backend of PoC IoT solutions for Electronica/Embedded World summit and client demos leveraging AWS (Greengrass, IoT Core, Lambda, DynamoDB), GCP (IoT Core, Pub/Sub, Dataflow, BigQuery) and Azure (IoT Hub, Stream Analytics, CosmosDB). Frontend NodeJS scripts that accesses Google Cloud SDK and Microsoft Azure SDK to demonstrate authenticating and querying of Big Query and CosmosDB databases.    


# FT900 IoT/SmartHome Demo applications

This contains the IoT framework including IoT demo applications for FTDI/Bridgetek's FT900 series of memory-constrained microcontrollers.

#### FT900 AWS IoT Demo
- This demonstrates secure MQTT connectivity to Amazon AWS IoT Core and AWS Greengrass using MQTT library from Amazon FreeRTOS aka AWS IoT SDK.
- It contains end-to-end demo including lambda functions for backend cloud and local gateway (Greengrass). 
- This was made after I successfully integrated mbedTLS open-source library with optimal settings for memory footprint.
- This was presented in Electronica 2018 https://brtchip.com/Brochures/FT900%20AWS%20IoT%20Demosheet.pdf

#### FT900 IoT Cloud Demo
- This demonstrates secure MQTT connectivity to IoT cloud services: Amazon AWS IoT Core (and Amazon AWS Greengrass), Google Cloud IoT Core and Microsoft Azure IoT Hub.
- This will be presented in Embedded World 2019 https://www.ftdichip.com/Embedded%20World%202019.htm

#### FT900 Alexa Demo
- This demonstrates using FT900 microcontroller as an Amazon Echo Dot device, where users can directly talk to Alexa.
- This will be integrated to PanL Smart Home Automation (PanL Hub and PanL Display).
  https://github.com/richmondu/FT900/tree/master/Alexa/Amazon%20Alexa%20Client
  https://github.com/richmondu/FT900/tree/master/Alexa/Amazon%20Alexa%20Gateway
