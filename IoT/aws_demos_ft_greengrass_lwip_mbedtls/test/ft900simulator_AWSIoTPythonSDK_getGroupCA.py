########################################################################################################
# This application retrieves the Greengrass group CA certificate from AWS IoT cloud
# and saves it to the local directory for access of the AWS IoT demo
########################################################################################################

import os
import sys
import time
import logging
import argparse
from AWSIoTPythonSDK.core.greengrass.discovery.providers import DiscoveryInfoProvider



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
    parser.add_argument("-p", "--filePath", action="store", dest="groupCAFilePath", default="certificates/", help="Group CA file path")
    parser.add_argument("-f", "--fileName", action="store", dest="groupCAFileName", default="group-ca-cert.pem", help="Group CA file name")
    
    return parser
    
    
def initGGDiscoveryProvider(host, rootCAPath, certificatePath, privateKeyPath, timeout):
   
    discoveryInfoProvider = DiscoveryInfoProvider()
    discoveryInfoProvider.configureEndpoint(host)
    discoveryInfoProvider.configureCredentials(rootCAPath, certificatePath, privateKeyPath)
    discoveryInfoProvider.configureTimeout(timeout)
    
    return discoveryInfoProvider
    
 
def getGroupCA(discoveryInfoProvider, thingName, filePath, fileName):

    caList = discoveryInfoProvider.discover(thingName).getAllCas()
    groupId, ca = caList[0]

    groupCA = filePath + fileName
    if not os.path.exists(filePath):
        os.makedirs(filePath)
    groupCAFile = open(groupCA, "w")
    groupCAFile.write(ca)
    groupCAFile.close()
    
    return groupCA

    
def main():

    print("\n")

    # Initialize logger
    initLogger()
    
    # Initialize input/parameter parser
    args = initParser().parse_args()
    if not args.certificatePath or not args.privateKeyPath:
        print("Missing credentials for authentication.")
        sys.exit(-1) 
    
    # Initialize Greengrass Group Discovery Provider
    discoveryInfoProvider = initGGDiscoveryProvider(args.host, args.rootCAPath, args.certificatePath, args.privateKeyPath, 10)
    
    # Get Greengrass group CA certificate using Greengrass Group Discovery Provider
    discovered = False
    retryCount = 3
    while retryCount != 0:
        try:
            groupCA = getGroupCA(discoveryInfoProvider, args.thingName, args.groupCAFilePath, args.groupCAFileName)
            discovered = True
            print("The group CA certificate has been retrieved and saved to: %s\n" % groupCA)
            break
        except BaseException as e:
            print("BaseException %s %s %d/%d" % (str(type(e)), e.message, retryCount, MAX_DISCOVERY_RETRIES) )
            retryCount -= 1

    if not discovered:
        print("Discovery failed after %d retries. Exiting...\n" % (MAX_DISCOVERY_RETRIES))
        sys.exit(-1)

        
if __name__== "__main__":
    main()