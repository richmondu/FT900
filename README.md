# FT900 IoT Demo applications

This contains IoT demo applications for FTDI/Bridgetek's FT900 series of memory-constrained microcontrollers. 

#### FT900 AWS IoT Demo
- This demonstrates secure MQTT connectivity to Amazon AWS IoT Core and AWS Greengrass using MQTT library from Amazon FreeRTOS.
- It contains end-to-end demo including lambda functions for backend cloud and local gateway (Greengrass). 
- This was made after I successfully integrated mbedTLS open-source library with optimal settings for memory footprint.
- This is no longer maintained. 

#### FT900 IoT Cloud Demo
- This demonstrates secure MQTT connectivity to IoT cloud services: Amazon AWS IoT Core (and Amazon AWS Greengrass), Google Cloud IoT Core, Microsoft Azure IoT Hub.

Third-party libraries used: mbedTLS, LWIP, FreeRTOS, tinyprintf
