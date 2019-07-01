# FT900 client for [libpyiotcloud](https://github.com/richmondu/libpyiotcloud)

This project is an FT900 MQTT client for [libpyiotcloud](https://github.com/richmondu/libpyiotcloud).
libpyiotcloud platform demonstrates a custom IoT platform for remote access and control of an MCU-based smart device 
from a web client application via REST APIs (HTTP over TLS) 
with back-end AMQP (over TLS) connectivity and device-side MQTT/AMQP (over TLS) connectivity.


### Features

    1. User Registration and Device Registration
       A. Using MongoDB NoSQL database
       B. Unique ca-signed certificate + privatekey generated for registered devices
    2. Device Access/Control
       A. get/set GPIOs
       B. get/set RTC
       C. get MAC address
       D. get IP/Subnet/Gateway addresses
       E. reset device
       F. write UART
    3. HTTPS/AMQPS/MQTTS Protocol Support
       [client --HTTPS--> webserver <--AMQPS--> messagebroker <--MQTTS--> device]
       A. HTTP over TLS: client app accessing REST APIs from webserver
       B. AMQP over TLS: webserver and messagebroker communication
       C. MQTT over TLS: messagebroker and device communication
    4. Device examples
       A. FT900 MCU device (LWIP-MQTT client)
       B. Python Paho-MQTT client device simulator
       C. Python Pika-AMQP client device simulator
       D. NodeJS MQTT client device simulator


### Architecture

Instead of using 'serverless' IoT solutions like AWS IoT Core, GCP IoT Core or Azure IoT Hub, 
we can create our own 'server-based' IoT solutions using Flask webserver, RabbitMQ message broker, Pika AMQP client, Paho MQTT client and MongoDB NoSQL database.
This server-based IoT solution architecture can be deployed in local PC or in the cloud - AWS EC2, Linode, CloudAMQP, Heroku, Rackspace, DigitalOcean or etc.


<img src="https://github.com/richmondu/libpyiotcloud/blob/master/images/architecture.png" width="800"/>

<img src="https://github.com/richmondu/libpyiotcloud/blob/master/images/sequence.png" width="800"/>

<img src="https://github.com/richmondu/libpyiotcloud/blob/master/images/sequence2.png" width="800"/>


Notes:

    1. This is a simple design and will not likely scale to millions of devices.
    2. RabbitMQ supports AMQP and MQTT.
    3. For MQTT to work, MQTT plugin must be installed in RabbitMQ.
    4. Login API will return a secret key that will be used for succeeduing API calls.
    5. Register device API will return deviceid, rootca, device certificate and device private key.
    6. Device shall use deviceid as MQTT client id and use the rootca, device certificate and device private key.



### Design

User Registration APIs

    1. signup
       - requires username, password
    2. login
       - requires username, password
       - returns secret key

Device Registration APIs

    1. register_device
       - requires username, secret, devicename
       - returns deviceid, rootca, devicecert, devicepkey
    2. unregister_device
       - requires username, secret, devicename
    3. get_device_list
       - requires username, secret, devicename
       - returns deviceid, rootca, devicecert, devicepkey for all devices registered by user

Device Control APIs

    1. get_gpio/set_GPIO
    2. get_rtc/set_rtc
    3. get_mac
    4. get_ip/get_subnet/get_gateway
    5. reset_device
    6. write_uart

Device MQTT/AMQP Connectivity

    1. HOST: ip address of RabbitMQ broker
    2. PORT: 8883 (MQTT) or 5671 (AMQP)
    3. USERNAME: guest
    4. PASSWORD: guest
    5. CLIENTID: deviceid (returned by register_device API)
    6. TLS ROOTCA: returned by register_device API
    7. TLS CERT: returned by register_device API
    8. TLS PKEY: returned by register_device API
    9. MQTT SUBSCRIBE: deviceid/#
    10. MQTT PUBLISH: server/deviceid/api
    11. AMQP SUBSCRIBE: mqtt-subscription-deviceidqos1
    12. AMQP PUBLISH: server.deviceid.api



### Instructions

    1. Setup and run RabbitMQ broker

        // Installation
        A. Install Erlang http://www.erlang.org/downloads]
        B. Install RabbitMQ [https://www.rabbitmq.com/install-windows.html]
        C. Install RabbitMQ MQTT plugin [https://www.rabbitmq.com/mqtt.html]
           >> Open RabbitMQ Command Prompt
           >> rabbitmq-plugins enable rabbitmq_mqtt

        // Configuration
        D. Add environment variable RABBITMQ_CONFIG_FILE %APPDATA%\RabbitMQ\rabbitmq.config
        E. Create configuration file %APPDATA%\RabbitMQ\rabbitmq.config based on rabbitmq.config.example
        F. Update configuration file to enable the following
           {tcp_listeners, [5672]},
           {ssl_listeners, [5671]},
           {loopback_users, []},
           {ssl_options, [{cacertfile, "rootca.pem"},
                          {certfile,   "server_cert.pem"},
                          {keyfile,    "server_pkey.pem"},
                          {verify,     verify_peer},
                          {fail_if_no_peer_cert, false},
                          {ciphers,  ["RSA-AES128-SHA", "RSA-AES256-SHA"]} ]}
           {default_user, <<"guest">>},
           {default_pass, <<"guest">>},
           {allow_anonymous, false},
           {tcp_listeners, [1883]},
           {ssl_listeners, [8883]}
        G. Restart RabbitMQ
           >> Open RabbitMQ Command Prompt
           >> rabbitmq-service.bat stop 
           >> rabbitmq-service.bat remove
           >> rabbitmq-service.bat install
           >> rabbitmq-service.bat start
        H. Copy certificates to %APPDATA%\RabbitMQ 
           rootca.pem, server_cert.pem, server_pkey.pem

    2. Install Python and python libraries in requirements.txt

       pip install -r requirements.txt

    3. Install MongoDB database.
    
    4. Run web_server.bat
  
    5. Run device_simulator.py_mqtt_ft900device1.bat and device_simulator.py_mqtt_ft900device2.bat OR 
       run device_simulator.py_amqp_ft900device1.bat and device_simulator.py_amqp_ft900device2.bat OR 
       run device_simulator.js_mqtt_ft900device1.bat and device_simulator.js_mqtt_ft900device2.bat OR 
       run FT900 MCU with the following details:
       
       device id: ft900device1
       device ca: rootca.pem
       device cert: ft900device1_cert.pem
       device pkey: ft900device1_pkey.pem
       OR
       device id: ft900device2
       device ca: rootca.pem
       device cert: ft900device2_cert.pem
       device pkey: ft900device2_pkey.pem

    6. Run client.bat



### Performance

The total round trip time for setting or getting the MCU GPIO is 2.01 seconds from the client application. But round trip time for the web server for sending MQTT publish and receiving MQTT response to/from MCU is only 1 second.

    client <-> webserver <-> mqttbroker <-> MCU: 2.01 seconds
               webserver <-> mqttbroker <-> MCU: 1.00 second
    Note: the webserver is still on my local PC, not yet on Linode or AWS EC2.

The client call to HTTP getresponse() is causing the additional 1 second delay. https://docs.python.org/3/library/http.client.html#http.client.HTTPConnection.getresponse For mobile client application, this 1 second delay maybe less or more. This will depend on the equivalent function HTTP client getresponse() in Java for Android or Swift for iOS..

