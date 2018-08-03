########################################################################################################
# This test application simulates the FT900 device for the FT900 AWS IoT demo
# It can be used to test and troubleshoot the backend cloud service
# This Python code can be tested using AWSIoTPythonSDK
# AWSIoTPythonSDK details can be found at https://github.com/aws/aws-iot-device-sdk-python
########################################################################################################

import os
import sys
import time
import uuid
import json
import logging
import argparse
import random
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient



AllowedActions = ['both', 'publish', 'subscribe', 'demo']

def initLogger():

    logger = logging.getLogger("AWSIoTPythonSDK.core")
    logger.setLevel(logging.ERROR) #DEBUG)
    streamHandler = logging.StreamHandler()
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    streamHandler.setFormatter(formatter)
    logger.addHandler(streamHandler)


def initParser():

    parser = argparse.ArgumentParser()
    parser.add_argument("-e", "--endpoint", action="store", required=True, dest="host", help="Your AWS IoT custom endpoint")
    parser.add_argument("-r", "--rootCA", action="store", required=True, dest="rootCAPath", help="Root CA file path")
    parser.add_argument("-c", "--cert", action="store", dest="certificatePath", help="Certificate file path")
    parser.add_argument("-k", "--key", action="store", dest="privateKeyPath", help="Private key file path")
    parser.add_argument("-n", "--thingName", action="store", dest="thingName", default="Bot", help="Targeted thing name")
    parser.add_argument("-t", "--topic", action="store", dest="topic", default="sdk/test/Python", help="Targeted topic")
    parser.add_argument("-m", "--mode", action="store", dest="mode", default="demo", help="Operation modes: %s"%str(AllowedActions))
    parser.add_argument("-M", "--message", action="store", dest="message", default="Hello World!", help="Message to publish")
    
    return parser


def customOnMessage(message):

    # General message notification callback
    print('Received message on topic %s: %s\n' % (message.topic, message.payload))


def initMQTTClient(host, port, rootCAPath, certificatePath, privateKeyPath, thingName):

    client = AWSIoTMQTTClient(thingName)
    client.configureCredentials(rootCAPath, privateKeyPath, certificatePath)
    client.onMessage = customOnMessage
    client.configureEndpoint(host, port)
    
    return client


def connectMQTTClient(client):

    ret = False
    try:
        ret = client.connect()
    except BaseException as e:
        return False
    
    return ret


def generateMessage(deviceId):

    message = {}
    message['deviceId'] = deviceId
    message['sensorReading'] = float("%.2f" % random.uniform(30, 40))
    message['batteryCharge'] = float("%.2f" % random.uniform(-10, 20))
    message['batteryDischargeRate'] = float("%.2f" % random.uniform(0, 5))
    
    return json.dumps(message)


def main():

    print("\n")

    # Configure logging
    initLogger()
    
    # Initialize input/parameter parser
    args = initParser().parse_args()
    if args.mode not in AllowedActions:
        print("Unknown --mode option %s. Must be one of %s" % (args.mode, str(AllowedActions)))
        sys.exit(-1)
    if not args.certificatePath or not args.privateKeyPath:
        print("Missing credentials for authentication.")
        sys.exit(-1)

    # Initialize and connect to the Greengrass core using AWS IoT MQTT Client
    # Use actual groupCA certificate
    # We skipped the GGC discovery which includes querying of Greengrass groupCA certificate, IP address and port number
    # For production code, GGC must be discovered by connecting to AWS IoT cloud first
    # But if devices does not have internet connection, then GGC discovery will not work anyway.
    # By hardcoding the groupCA certificate, we can connect directly to Greengrass without connecting to AWS IoT
    MQTTclient = initMQTTClient(args.host, 8883, args.rootCAPath, args.certificatePath, args.privateKeyPath, args.thingName)
    if connectMQTTClient(MQTTclient) != True:
        print("Cannot connect to host %s" % args.host)
        sys.exit(-1)
        
    # Send simulated sensor data for each device in JSON format
    if args.mode == 'demo':
        while True:
            devices = ['knuth', 'hopper', 'turing']
            for deviceId in devices:
                topic = "device/" + deviceId + "/devicePayload"
                message = generateMessage(deviceId)
                if MQTTclient.publish(topic, message, 0) != True:
                    print("Cannot publish to host %s" % args.host)
                    sys.exit(-1)
                print('Published topic %s:\n%s\n\n' % (topic, message))
                time.sleep(1)
                
    # support publish and subscribe modes for generic MQTT testing
    else:
        if args.mode == 'subscribe' or args.mode == 'both':
            MQTTclient.subscribe(args.topic, 0, None)
            time.sleep(2)        
        if args.mode == 'publish' or args.mode == 'both':
            loopCount = 0
            while True:
                message = {}
                message['message'] = args.message
                message['sequence'] = loopCount
                messageJson = json.dumps(message)
                MQTTclient.publish(args.topic, messageJson, 0)
                print('Published topic %s: %s\n' % (args.topic, messageJson))
                loopCount += 1
                time.sleep(1)  
    
    
if __name__== "__main__":
    main()