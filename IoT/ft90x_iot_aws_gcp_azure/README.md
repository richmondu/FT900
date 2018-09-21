# FT900 IoT Cloud demo


This demo is an improvement of the FT900 AWS IoT demo. It demonstrates:

1. Connectivity to IoT cloud services: Amazon AWS, Google Cloud and Microsoft Azure
2. Use of LWIP's MQTT library with ALTCP_TLS instead of MQTT library from Amazon FreeRTOS
3. Improvement of net.c Ethernet abstraction layer for simplification of user application code
4. MQTT subscription in addition to MQTT publish

It also contains 3 set of device certificates that connects to all 3 cloud services.

