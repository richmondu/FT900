# FT900 IoT Security and Cloud Connectivity

I handle IoT security and cloud connectivity for FT900 microcontroller.

- IoT Security: I established the IoT framework including mbedTLS integration, SSL certificate authentication, JWT/SAS security token generation, ciphersuite-memory tradeoffs and LWIP open-source contributions
- IoT Connectivity: I debugged and supported secure authentication and communication with IoT cloud platforms: Amazon Web Services, Google Cloud Platform and Microsoft Azure. I also created the IoT project templates (AWS, GCP, Azure) for Eclipse plugin of FT900 Toolchain.
- IoT Solutions: I implemented backend of PoC prototypes of end-to-end IoT solutions for Electronica/Embedded World summit and client demos leveraging AWS (Greengrass, IoT Core, Lambda, DynamoDB), GCP (IoT Core, Pub/Sub, Dataflow, BigQuery) and Azure (IoT Hub, Stream Analytics, CosmosDB). I also assisted creation of frontend by creating NodeJS scripts that uses Google Cloud SDK and Microsoft Azure SDK to demonstrate authenticating and querying of Big Query and CosmosDB databases.    


# FT900 IoT Demo applications

This contains the IoT framework including IoT demo applications for FTDI/Bridgetek's FT900 series of memory-constrained microcontrollers.

#### FT900 AWS IoT Demo
- This demonstrates secure MQTT connectivity to Amazon AWS IoT Core and AWS Greengrass using MQTT library from Amazon FreeRTOS.
- It contains end-to-end demo including lambda functions for backend cloud and local gateway (Greengrass). 
- This was made after I successfully integrated mbedTLS open-source library with optimal settings for memory footprint.
- This is no longer maintained. 

#### FT900 IoT Cloud Demo
- This demonstrates secure MQTT connectivity to IoT cloud services: Amazon AWS IoT Core (and Amazon AWS Greengrass), Google Cloud IoT Core and Microsoft Azure IoT Hub.

Third-party libraries used: mbedTLS, LWIP, FreeRTOS, tinyprintf
