# FT900 client for [libpyiotcloud](https://github.com/richmondu/libpyiotcloud)

# libpyiotcloud

libpyiotcloud is a dockerized IoT platform for secure remote access and control of an MCU-based smart device 
from a mobile or desktop web application via REST APIs (<b>HTTP over TLS</b>) 
with back-end <b>AMQP over TLS</b> connectivity and device-side <b>MQTT over TLS</b> connectivity.


# Background

Popular cloud platforms such as Amazon Web Services, Google Cloud Platform and Microsoft Azure provide their IoT platforms, namely, [AWS IoT Core](https://aws.amazon.com/iot/), [GCP IoT Core](https://cloud.google.com/iot-core/) or [Azure IoT Hub](https://azure.microsoft.com/en-us/services/iot-hub/). There are also dedicated IoT providers such as [Adafruit.IO](https://io.adafruit.com/), [Ubidots](https://ubidots.com/) and [ThingSpeak](https://thingspeak.com/). 
These IoT platforms are good (in fact, I have tested it with FT900 - refer to [ft90x_iot_aws_gcp_azure](https://github.com/richmondu/FT900/tree/master/IoT/ft90x_iot_aws_gcp_azure). 
But these IoT platforms are limited in that they are focused on sensor data dashboarding.
This requires device to frequently send sensor data in order to generate graphs. 
It lacks support for the use-case of remote access and control memory-constrained microcontrollers.
In this use-case, the device only sends the data when queried.

Comparable IoT platforms for our use-case of remote device access and control include the IoT platforms of 

    - TP-Link (with Kasa mobile application) for TPLink smart bulbs/smart plugs, and 
    - Xiaomi (YeeLight mobile app) for Xiaomi's smart smart bulbs/smart plugs.

However, these IoT platforms are tied up to their smart devices.
Our IoT platform is generic for all smart devices and IoT devices that can be build on top of any MCU, but primarily using our in-house FT9XX MCUs.


# Architecture

This IoT platform is a server-based IoT cloud platform that leverages 
Flask, GUnicorn, Nginx, RabbitMQ, MongoDB, Amazon Cognito, Amazon Pinpoint and Docker. 
It can be deployed in a local PC or in the cloud - AWS EC2, Linode, Heroku, Rackspace, DigitalOcean or etc.

- Nginx web server - https://www.nginx.com/
- GUnicorn WSGI server - https://gunicorn.org/
- Flask web framework (REST API) - http://flask.pocoo.org/
- RabbitMQ message broker (MQTT, AMQP) - https://www.rabbitmq.com/
- MongoDB NoSQL database - https://www.mongodb.com/
- OpenSSL cryptography (X509 certificates) - https://www.openssl.org/
- Amazon Cognito (user sign-up/sign-in) - https://aws.amazon.com/cognito/
- Amazon Pinpoint (email/SMS notifications) - https://aws.amazon.com/pinpoint/
- Amazon SNS (email/SMS notifications) - https://aws.amazon.com/sns/
- Docker containerization (dockerfiles, docker-compose) - https://www.docker.com/
- Ionic mobile/web frontend framework - https://ionicframework.com/
- Postman (API testing tool) - https://www.getpostman.com/

An alternative solution is using an AWS serverless solution wherein:

- AWS API Gateway+AWS Lambda will replace Flask+Gunicorn+Nginx
- AWS DynamoDB will replace MongoDB
- AmazonMQ will replace RabbitMQ


### High-level architecture diagram:
<img src="https://github.com/richmondu/libpyiotcloud/blob/master/_images/architecture.png" width="1000"/>

### UML Use case diagram:
<img src="https://github.com/richmondu/libpyiotcloud/blob/master/_images/usecase.png" width="800"/>

### UML Sequence diagram (user sign-up/sign-in):
<img src="https://github.com/richmondu/libpyiotcloud/blob/master/_images/sequence1.png" width="800"/>

### UML Sequence diagram (device registration):
<img src="https://github.com/richmondu/libpyiotcloud/blob/master/_images/sequence2.png" width="800"/>

### UML Sequence diagram (device access/control):
<img src="https://github.com/richmondu/libpyiotcloud/blob/master/_images/sequence3.png" width="800"/>

### Notes:

    1. RabbitMQ supports AMQP and MQTT.
    2. For MQTT to work, MQTT plugin must be installed in RabbitMQ.
    3. Login API will return an access token that will be used for succeeding API calls.
    4. Register device API will return deviceid, rootca, device certificate and device private key.
    5. Device shall use deviceid as MQTT client id and use the rootca, device certificate and device private key.
    6. The webserver has been tested on Linux Ubuntu 16.04 using GUnicorn and Nginx.



# Design

### Features

    1. User sign-up/sign-in, Device Registration, Email/SMS Notifications
       A. Amazon Cognito for user sign-up and sign-in
       B. MongoDB NoSQL database for storing registered device information
       C. OpenSSL for generating certificates on-demand for registered devices
       D. Email/SMS notifications using AmazonPinpoint (device-initiated, client-initiated)
    2. Device Access/Control via Flask+GUnicorn+Nginx
       - get/set GPIOs, get/set RTC, get MAC address, reset device
       - get IP/Subnet/Gateway addresses, write UART
    3. HTTPS/AMQPS/MQTTS Protocol Support
       [client --HTTPS--> webserver <--MQTTS (or AMQPS)--> msgbroker <--MQTTS (and AMQPS)--> device]
       A. HTTP over TLS: client app accessing REST APIs from webserver
       B. AMQP over TLS: webserver and messagebroker communication
       C. MQTT over TLS: messagebroker and device communication
    4. Device examples and simulators
       A. FT900 MCU device (LWIP-MQTT client)
       B. MQTT device simulators (Python Paho-MQTT and NodeJS)
       C. AMQP device simulator (Python Pika-AMQP)
    5. Deployment to AWS EC2 as microservices using Docker
       - 5 microservices/docker containers [rabbitmq, mongodb, webapp, nginx, notification]
       - with Dockerfiles and Docker-compose file


### REST APIs for User Sign-up/Sign-In

    1. sign_up
       - requires username, password, email, firstname, lastname
       - confirmation code will be sent to email
    2. confirm_sign_up
       - requires username, confirmation
    3. login
       - requires username, password
       - returns access_token
    4. logout
       - requires username, access key
    5. forgot_password
       - requires email address
       - confirmation code will be sent to email
    6. confirm_forgot_password
       - requires username, new password, confirmation code

### REST APIs for Device Registration/Management

    1. register_device
       - requires username, access_token, devicename
       - returns deviceid, rootca, devicecert, devicepkey
         which shall be used on the actual microcontroller device
    2. unregister_device
       - requires username, access_token, devicename
    3. get_device_list
       - requires username, access_token
       - returns device info [devicename, deviceid, rootca, devicecert, devicepkey for all devices registered by user]
    4. get_device_list_count
       - requires username, access_token
       - returns length of device list
    5. get_device_index
       - requires username, access_token, index
       - returns device info for device[index]

### REST APIs for Device Access/Control

    1. get_gpio
    2. set_GPIO
    3. get_rtc
    4. set_rtc
    5. get_mac
    6. get_ip
    7. get_subnet
    8. get_gateway
    9. get_status
    10. set_status (including reset device)
    11. write_uart
    12. trigger_notification


### Device settings for MQTT/AMQP Connectivity

    User must first register a device in the portal. Registering a device will return deviceid, rootca, cert and pkey.
    1. HOST: ip address of RabbitMQ broker
    2. PORT: 8883 (MQTT) or 5671 (AMQP)
    3. USERNAME: NULL
    4. PASSWORD: NULL
    5. CLIENTID: deviceid returned by register_device API
    6. TLS ROOTCA: returned by register_device API
    7. TLS CERT: returned by register_device API
    8. TLS PKEY: returned by register_device API
    9. MQTT SUBSCRIBE: deviceid/#
    10. MQTT PUBLISH: server/deviceid/api
    11. AMQP SUBSCRIBE: mqtt-subscription-deviceidqos1
    12. AMQP PUBLISH: server.deviceid.api


### Device MQTT/AMQP Processing

    Device can either use MQTT or AMQP. For FT900 MCU device, only MQTT is currently supported.
    MQTT
    1. Subscribe to deviceid/#
    2. Receive MQTT payload with topic "deviceid/api"
    3. Parse the api
    4. Process the api with the given payload
    5. Publish answer to topic "server/deviceid/api"

    AMQP
    1. Subscribe to mqtt-subscription-deviceidqos1
    2. Receive MQTT payload with topic "deviceid.api"
    3. Parse the api
    4. Process the api with the given payload
    5. Publish answer to topic "server.deviceid.api"


### Email/SMS Notifications

    1. Device can trigger Notification Manager to send email/SMS via Amazon Pinpoint
       device -> messagebroker -> notificationmanager -> pinpoint
    2. Notification manager subscribes to topic "server/deviceid/trigger_notifications"
       Once it receives a message on this topic, it will trigger Amazon Pinpoint to send the email or SMS.
    3. Web client application can also trigger device to send email/SMS notifications via the trigger_notification REST API.
       webclient -> webserver(rest api) -> messagebroker -> device -> messagebroker -> notificationmanager -> pinpoint


# Instructions (Docker)

    0. Install Docker
    1. Set AWS credentials + cognito/pinpoint IDs as environment variables
       export AWS_ACCESS_KEY_ID=""
       export AWS_SECRET_ACCESS_KEY=""
       export AWS_COGNITO_CLIENT_ID=""
       export AWS_COGNITO_USERPOOL_ID=""
       export AWS_COGNITO_USERPOOL_REGION=""     
       export AWS_PINPOINT_ID=""
       export AWS_PINPOINT_REGION=""
       export AWS_PINPOINT_EMAIL=""
    2. Build and execute Docker-compose file
       docker-compose build
       docker-compose up
    3. Test by browsing https://192.168.99.100 or https://<aws_ec2_hostname> or https://<aws_ec2_ip>
    

# Manual Instructions (non-Docker)

### Install Python 3.6.6 and Python libraries

       sudo apt-get install build-essential checkinstall
       sudo apt-get install libreadline-gplv2-dev libncursesw5-dev libssl-dev libsqlite3-dev tk-dev libgdbm-dev libc6-dev libbz2-dev
       cd /usr/src
       sudo wget https://www.python.org/ftp/python/3.6.6/Python-3.6.6.tgz
       sudo tar xzf Python-3.6.6.tgz
       cd Python-3.6.6
       sudo ./configure --enable-optimizations
       sudo make altinstall
       python3.6 -V
       
       pip install -r requirements.txt


### Install Gunicorn and Nginx

       // GUNICORN
       sudo pip install gunicorn
       copy web_server.service to /etc/systemd/system
       sudo systemctl start web_server
       sudo systemctl status web_server
       sudo nano /etc/systemd/system/web_server.service
       gunicorn --bind ip:port wsgi:app // test
       
       // NGINX
       sudo apt-get install nginx
       copy web_server to /etc/nginx/sites-available
       sudo ln -s /etc/nginx/sites-available/web_server /etc/nginx/sites-enabled
       sudo nginx -t
       sudo systemctl start nginx
       sudo systemctl status nginx
       sudo nano /etc/nginx/sites-available/web_server
       
       [https://www.digitalocean.com/community/tutorials/how-to-serve-flask-applications-with-gunicorn-and-nginx-on-ubuntu-18-04]
       [https://www.digitalocean.com/community/tutorials/how-to-install-nginx-on-ubuntu-16-04]
   

### Setup and run RabbitMQ broker

        LINUX:
        
        // Installation
        A. Install Erlang
           wget https://packages.erlang-solutions.com/erlang-solutions_1.0_all.deb
           sudo dpkg -i erlang-solutions_1.0_all.deb
           sudo apt-get update
           sudo apt-get install erlang
        B. Install RabbitMQ
           curl -s https://packagecloud.io/install/repositories/rabbitmq/rabbitmq-server/script.deb.sh | sudo bash
           sudo apt-get install rabbitmq-server=3.7.16-1
        C. Install RabbitMQ MQTT plugin
           sudo rabbitmq-plugins enable rabbitmq_mqtt
        
        // Configuration
        D. Update port firewall
           sudo ufw allow 8883
           sudo ufw allow 5671
           sudo ufw deny 1883
           sudo ufw deny 5672
        E. Configure RabbitMQ
           cd /etc/rabbitmq
           sudo chmod 777 .
           copy rabbitmq.config
           copy rootca.pem
           copy server_cert.pem
           copy server_pkey.pem
        F. Check RabbitMQ 
           sudo service rabbitmq-server start
           sudo service rabbitmq-server stop
           sudo service rabbitmq-server status
           sudo service rabbitmq-server restart


        WINDOWS:
        
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
           {tcp_listeners, []},
           {ssl_listeners, [5671]},
           {loopback_users, []},
           {ssl_options, [{cacertfile, "rootca.pem"},
                          {certfile,   "server_cert.pem"},
                          {keyfile,    "server_pkey.pem"},
                          {verify,     verify_peer},
                          {fail_if_no_peer_cert, false},
                          {ciphers,  ["RSA-AES128-SHA", "RSA-AES256-SHA"]} ]}
           {allow_anonymous, true},
           {tcp_listeners, []},
           {ssl_listeners, [8883]}
        G. Restart RabbitMQ
           >> Open RabbitMQ Command Prompt
           >> rabbitmq-service.bat stop 
           >> rabbitmq-service.bat remove
           >> rabbitmq-service.bat install
           >> rabbitmq-service.bat start
        H. Copy certificates to %APPDATA%\RabbitMQ 
           rootca.pem, server_cert.pem, server_pkey.pem


### Install MongoDB database.
       
       LINUX: [https://docs.mongodb.com/manual/tutorial/install-mongodb-on-ubuntu/]
       
       sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv 9DA31620334BD75D9DCB49F368818C72E52529D4
       echo "deb [ arch=amd64,arm64 ] https://repo.mongodb.org/apt/ubuntu xenial/mongodb-org/4.0 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-4.0.list
       sudo apt-get update
       sudo apt-get install -y mongodb-org
       sudo service mongod start
       sudo nano /var/log/mongodb/mongod.log

       WINDOWS: [https://www.mongodb.com/download-center/community?jmp=docs]
       Download and run MSI installer from the link above


### Setup Amazon Cognito.
    
       // Amazon Cognito cloud setup
       A. Click on "Manage User Pools"
       B. Click on "Create a user pool"
       C. Type Pool name and click "Step through settings"
       D. Check "family name" and "given name" and click "Next step"
       E. Click "Next step"
       F. Click "Next step"
       G. Click "Next step"
       H. Click "Next step"
       I. Click "Next step"
       J. Click "Add an app client", type App client name, uncheck "Generate client secret", 
          click "Create app client" and click "Next step"
       K. Click "Next step"
       L. Click "Create pool"
   
       // Update environment variables
       A. AWS_COGNITO_USERPOOL_REGION = Region of Cognito User Pool ex. "ap-southeast-1"
       B. AWS_COGNITO_USERPOOL_ID     = Copy from General settings/Pool Id
       C. AWS_COGNITO_CLIENT_ID       = Copy from General settings/App clients/App client id


### Setup Amazon Pinpoint.
    
       // Amazon Pinpoint cloud setup
       A. Click on "Create a project"
       B. Under Settings, click on "Email". 
          Under Identities tab, click Edit button.
          Select "Verify a new email address" and input "Email address".
          Check email and click on the link. 
          Get back on AWS and click Save.
       C. Under Settings, click on "SMS and voice". 
          Under SMS settings tab, click Edit button.
          Select "Enable the SMS channel for this project" and click "Save changes" button.
       D. Copy the Project ID and region for the environment variables.   
          
       // Update environment variables
       A. AWS_PINPOINT_REGION = Region of Cognito User Pool ex. "ap-southeast-1"
       B. AWS_PINPOINT_ID     = Copy from "All projects"
       C. AWS_PINPOINT_EMAIL  = Email registered to be used for email sender
          

### Others

       A. Run web_server.bat

       B. Run device_simulator.py_mqtt_ft900device1.bat and device_simulator.py_mqtt_ft900device2.bat OR 
          run device_simulator.py_amqp_ft900device1.bat and device_simulator.py_amqp_ft900device2.bat OR 
          run device_simulator.js_mqtt_ft900device1.bat and device_simulator.js_mqtt_ft900device2.bat OR 
          run FT900 MCU with the following details:

          device id: id for ft900device1
          device ca: rootca.pem
          device cert: ft900device1_cert.pem
          device pkey: ft900device1_pkey.pem

       C. Browse https://127.0.0.1 [or run client.bat for API testing]


### Certificates

       1. NGINX: rootca.pem rootca.pkey
       2. RabbitMQ: rootca.pem, server_cert.pem, server_pkey.pem
       3. WebApp: rootca.pem, server_cert.pem, server_pkey.pem
       4. Notification: rootca.pem, notification_manager_cert.pem, notification_manager_pkey.pem
       5. Device: rootca.pem, device_X.pem, device_X.pkey


### AWS EC2

       // AWS EC2 setup
       A. Create a t2.micro instance of Amazon Linux (or Ubuntu 16.04 if not using Docker)
       B. Dowload "Private key file for authentication" for SSH access
       C. Copy the "IPv4 Public IP" address
       D. Enable ports: 22 (SSH), 8883 (MQTTS), 5671 (AMQPS), 443 (HTTPS)

       // PUTTY setup (for SSH console access)
       A. Create PPK file from the PEM file downloaded from EC2 using PuttyGEN
       B. Using Putty, go to Category > Connection > SSH > Auth, then click Browse for "Private key file for authentication"    
       C. Set "hostname (or IP address)" to "ec2-user@IPV4_PUBLIC_IP_ADDRESS" (or "ubuntu@IPV4_PUBLIC_IP_ADDRESS" if using Ubuntu)
       
       // WINSCP setup (for SSH file transfer access)
       A. Create New Site
       B. Set "Host name:" to IPV4_PUBLIC_IP_ADDRESS
       C. Set "User name:" to ec2-user (or ubuntu if using Ubuntu)

       // Docker installation
       sudo yum update -y
       sudo yum install -y docker
       sudo usermod -aG docker ec2-user
       sudo curl -L https://github.com/docker/compose/releases/download/1.24.1/docker-compose-`uname -s`-`uname -m` -o /usr/local/bin/docker-compose
       sudo chmod +x /usr/local/bin/docker-compose
       sudo service docker restart
       
       // Set the AWS environment variables
       export AWS_ACCESS_KEY_ID=""
       export AWS_SECRET_ACCESS_KEY=""
       export AWS_COGNITO_CLIENT_ID=""
       export AWS_COGNITO_USERPOOL_ID=""
       export AWS_COGNITO_USERPOOL_REGION=""
       export AWS_PINPOINT_ID=""
       export AWS_PINPOINT_REGION=""       
       export AWS_PINPOINT_EMAIL=""       

       // Download the repository
       via WinSCP or git
       
       // Docker run
       docker-compose -f docker-compose.yml config
       docker-compose build
       docker-compose up
        

### AWS Credentials

       1. AWS_ACCESS_KEY_ID
       2. AWS_SECRET_ACCESS_KEY
       3. AWS_COGNITO_CLIENT_ID
       4. AWS_COGNITO_USERPOOL_ID
       5. AWS_COGNITO_USERPOOL_REGION       
       6. AWS_PINPOINT_ID
       7. AWS_PINPOINT_REGION
       8. AWS_PINPOINT_EMAIL

### Docker

        Overheads:
        1. Networking: 100 microseconds slower which is neglible. 
           [Not ideal for time-sensitive forex or stock trading market]
        2. Size: Dockers is lightweight.
        3. CPU/RAM: Virtually none on Linux.
        4. Learning: its easier than i thought, many documentations available 
           [Linux familiarity is the overhead]

        Advantages:
        1. Automates installation and deployment 
           [abstracts Linux knowledge requirements for installations/running]
        2. Automates developer/QA testing 
           [anyone can reproduce and on their own Windows 7 machine using Docker Toolbox]
        3. Simplifies maintenance and upgrade
           Dockerfile and Docker-compose file are basically Linux bash scripts
           But Dockerfile and Docker-compose file are very readable
           Easy to add/replace microservices in case needed
   
### Dockerfiles

1. The platform has been divided into 5 microservices: rabbitmq, mongodb, webapp, nginx, notification_manager
2. Each microservice is contained in a separate docker container
3. Each docker container has a dockerfile

        // RABBITMQ Dockerfile
        FROM rabbitmq:3.7
        RUN rabbitmq-plugins enable --offline rabbitmq_management
        RUN rabbitmq-plugins enable --offline rabbitmq_mqtt
        COPY src/ /etc/rabbitmq/
        EXPOSE 5671
        EXPOSE 8883

        // MONGODB Dockerfile
        FROM mongo:latest
        VOLUME ["/data/db"]
        WORKDIR /data
        EXPOSE 27017

        // WEBAPP Dockerfile
        FROM python:3.6.6
        RUN mkdir -p /usr/src/app/libpyiotcloud
        WORKDIR /usr/src/app/libpyiotcloud
        COPY libpyiotcloud/ /usr/src/app/libpyiotcloud/
        RUN pip install --no-cache-dir -r requirements.txt
        CMD ["gunicorn", "--workers=1", "--bind=0.0.0.0:8000", "--forwarded-allow-ips='*'", "wsgi:app"]
        EXPOSE 8000

        // NGINX Dockerfile
        FROM nginx:latest
        RUN rm /etc/nginx/conf.d/default.conf
        COPY src/ /etc/nginx/conf.d/
        EXPOSE 443

        // NOTIFICATION Dockerfile
        FROM python:3.6.6
        RUN mkdir -p /usr/src/app/notification_manager
        WORKDIR /usr/src/app/notification_manager
        COPY src/ /usr/src/app/notification_manager/
        WORKDIR /usr/src/app/notification_manager/notification_manager
        RUN pip install --no-cache-dir -r requirements.txt
        CMD ["python", "-u", "notification_manager.py", "--USE_HOST", "172.18.0.2"]

        // CREATE and RUN
        docker network create --subnet=172.18.0.0/16 mydockernet
        docker build -t rmq .
        docker run --net mydockernet --ip 172.18.0.2 -d -p 8883:8883 -p 5671:5671 -p 15672:15672 --name rmq rmq
        docker build -t mdb .
        docker run --net mydockernet --ip 172.18.0.3 -d -p 27017:27017 -v /data:/data/db --name mdb mdb
        docker build -t app .
        docker run --net mydockernet --ip 172.18.0.4 -d -p 8000:8000 --name app app
        docker build -t ngx .
        docker run --net mydockernet --ip 172.18.0.5 -d -p 443:443 --name ngx ngx
        docker build -t nmg .
        docker run --net mydockernet --ip 172.18.0.6 -d --name nmg nmg

        // STOP and REMOVE
        docker ps
        docker ps -a
        docker stop rmq
        docker stop mdb
        docker stop app
        docker stop ngx
        docker stop nmg
        docker rm rmq
        docker rm mdb
        docker rm app
        docker rm ngx
        docker rm nmg
        docker network rm mydockernet


### Dockercompose

1. Internal network created for the docker containers

        sudo docker network ls
        sudo docker network inspect mydockernet
    
2. Persistent volume for mongodb database created

        sudo docker volume ls
        sudo docker volume inspect mydockervol // get the mountpoint
        sudo ls <mountpoint>

3. AWS credentials + cognito/pinpoint IDs are environment variables [no longer hardcoded in code]

        Prerequisite: set the following environment variables
        - AWS_ACCESS_KEY_ID
        - AWS_SECRET_ACCESS_KEY
        - AWS_COGNITO_CLIENT_ID
        - AWS_COGNITO_USERPOOL_ID
        - AWS_COGNITO_USERPOOL_REGION       
        - AWS_PINPOINT_ID
        - AWS_PINPOINT_REGION
        - AWS_PINPOINT_EMAIL

4. Docker-compose file

        // docker-compose.yml
        version: '3.7'
        services:
          rabbitmq:
            build: ./rabbitmq
            restart: always
            networks:
              mydockernet:
                ipv4_address: 172.18.0.2
            ports:
              - "8883:8883"
              - "5671:5671"
            expose:
              - "8883"
              - "5671"
          mongodb:
            build: ./mongodb
            restart: always
            networks:
              mydockernet:
                ipv4_address: 172.18.0.3
            ports:
              - "27017:27017"
            volumes:
              - "mydockervol:/data/db"
          webapp:
            build: ./webapp
            restart: always
            networks:
              mydockernet:
                ipv4_address: 172.18.0.4
            ports:
              - "8000:8000"
            depends_on:
              - rabbitmq
              - mongodb
            environment:
              - AWS_ACCESS_KEY_ID
              - AWS_SECRET_ACCESS_KEY
              - AWS_COGNITO_CLIENT_ID
              - AWS_COGNITO_USERPOOL_ID
              - AWS_COGNITO_USERPOOL_REGION
          nginx:
            build: ./nginx
            restart: always
            networks:
              mydockernet:
                ipv4_address: 172.18.0.5
            ports:
              - "443:443"
            expose:
              - "443"
            depends_on:
              - webapp
          notification:
            build: ./notification
            restart: always
            networks:
              mydockernet:
                ipv4_address: 172.18.0.6
            depends_on:
              - rabbitmq
              - nginx
              - webapp
              - mongodb
            environment:
              - AWS_ACCESS_KEY_ID
              - AWS_SECRET_ACCESS_KEY
              - AWS_PINPOINT_ID
              - AWS_PINPOINT_REGION              
              - AWS_PINPOINT_EMAIL              
        networks:
          mydockernet:
            driver: bridge
            ipam:
              config:
                - subnet: 172.18.0.0/16
                  gateway: 172.18.0.1
        volumes:
          mydockervol:
            driver: local
    
        // test
        https:// 192.168.99.100
        mqtts:// 192.168.99.100:8883
        amqps:// 192.168.99.100:5671

5. Docker-compose commands

        docker-compose -f docker-compose.yml config
        docker-compose build
        docker-compose up
        docker-compose up -d // run as daemon
        docker-compose ps
        docker-compose down


# Testing and Troubleshooting

### Using FT900 (Currently tested with an FT900 RevC board-MM900EV1B only)

        1. Download the FT900 code from https://github.com/richmondu/FT900/tree/master/IoT/ft90x_iot_brtcloud
        2. Update USE_DEVICE_ID in Includes/iot_config.h to match the generated ID of the registered device in the portal
        3. Build and run FT900 code 
        4. Access/control the device via the portal

### Using device simulators

        1. Download the device simulators from https://github.com/richmondu/libpyiotcloud/tree/master/_device_simulator
           Choose from any of the 3: 
           Python-MQTT device simulator
           Python-AMQP device simulator
           Javascript-MQTT device simulator
        2. Update DEVICE_ID in the corresponding batch script to match the generated ID of the registered device in the portal
        3. Run the updated batch script
        4. Access/control the device via the portal

### Test utilities

        web_server_database_viewer.bat 
        - view registered devices (MongoDB) and registered users (Amazon Cognito)

### Troubleshooting

        Dockerized:
        - docker-compose ps
        - docker ps
        - docker network ls
        - docker volume ls

        Manual:
        - sudo service mongod status 
        - sudo systemctl status web_server
        - sudo systemctl status nginx


# Performance

### Windows 

The total round trip time for setting or getting the MCU GPIO is 2.01 seconds from the client application. But round trip time for the web server for sending MQTT publish and receiving MQTT response to/from MCU is only 1 second.

    client <-> webserver <-> messagebroker <-> MCU: 2.01 seconds
               webserver <-> messagebroker <-> MCU: 1.00 second
    Note: the webserver is still on my local PC, not yet on Linode or AWS EC2.

The client call to HTTP getresponse() is causing the additional 1 second delay. https://docs.python.org/3/library/http.client.html#http.client.HTTPConnection.getresponse For mobile client application, this 1 second delay maybe less or more. This will depend on the equivalent function HTTP client getresponse() in Java for Android or Swift for iOS..

### Linux

In Linux, the total round trip time is only 1 second.


# Action Items

1. Use Ionic front-end framework for cross-platform mobile, web, and desktop apps
2. Add message counter for free-tier subscription
3. Add manager/admin page in Web client (see all users and devices registered by each user)
4. Add logging for debugging/troubleshooting
5. Handle refreshing Cognito access key while user is still online
6. Support Kubernetes orchestration
