#from umqtt.simple import MQTTClient
from iot.umqtt.simple import MQTTClient
import _thread
import time
import urandom
import utime
import sys
from iot import base64, hmac
#from iot import hashlib
import uhashlib as hashlib
import ubinascii


# select CONFIG_SERVER
CONFIG_SERVER_AWS_IOT       = ('amasgua12bmkv-ats.iot.us-east-1.amazonaws.com', 8883)
CONFIG_SERVER_GCP_IOT       = ('mqtt.googleapis.com', 8883)
CONFIG_SERVER_AZURE_IOT     = ('FT900IoTHub.azure-devices.net', 8883)
CONFIG_SERVER_MOSQUITTO     = ('test.mosquitto.org', 1883)
CONFIG_SERVER               = CONFIG_SERVER_MOSQUITTO



# amazon web services
CONFIG_AWS_CERTIFICATE_FILE = "iot/certs/ft900device1_cert.pem"
CONFIG_AWS_CERTIFICATE_PKEY = "iot/certs/ft900device1_pkey.pem"
CONFIG_AWS_CERTIFICATE_RTCA = "iot/certs/rootca_aws_ats.pem"
CONFIG_AWS_CREDENTIAL_ID    = "ft900device1"
CONFIG_AWS_CREDENTIAL_USER  = None
CONFIG_AWS_CREDENTIAL_PASS  = None

# google cloud
CONFIG_GCP_CERTIFICATE_FILE = "iot/certs/ft900device1_cert.pem"
CONFIG_GCP_CERTIFICATE_PKEY = "iot/certs/ft900device1_pkey.pem"
CONFIG_GCP_CERTIFICATE_RTCA = None
CONFIG_GCP_PROJECT_ID       = "ft900iotproject"
CONFIG_GCP_LOCATION_ID      = "us-central1"
CONFIG_GCP_REGISTRY_ID      = "ft900registryid"
CONFIG_GCP_DEVICE_ID        = "ft900device1"
CONFIG_GCP_CREDENTIAL_ID    = "projects/" + CONFIG_GCP_PROJECT_ID + "/locations/" + CONFIG_GCP_LOCATION_ID + "/registries/" + CONFIG_GCP_REGISTRY_ID + "/devices/" + CONFIG_GCP_DEVICE_ID
CONFIG_GCP_CREDENTIAL_USER  = ""
CONFIG_GCP_CREDENTIAL_PASS  = None

# microsoft azure
CONFIG_AZU_CERTIFICATE_FILE = "iot/certs/ft900device2_cert.pem"
CONFIG_AZU_CERTIFICATE_PKEY = "iot/certs/ft900device2_pkey.pem"
CONFIG_AZU_CERTIFICATE_RTCA = "iot/certs/rootca_azure.pem"
CONFIG_AZU_HUB_NAME         = "FT900IoTHub"
CONFIG_AZU_CREDENTIAL_ID    = "ft900device2"
CONFIG_AZU_CREDENTIAL_USER  = CONFIG_AZU_HUB_NAME + ".azure-devices.net/" + CONFIG_AZU_CREDENTIAL_ID + "/api-version=2016-11-14"
CONFIG_AZU_CREDENTIAL_PASS  = ""

# eclipse mosquitto
CONFIG_MOS_CERTIFICATE_FILE = None
CONFIG_MOS_CERTIFICATE_PKEY = None
CONFIG_MOS_CERTIFICATE_RTCA = None
CONFIG_MOS_CREDENTIAL_ID    = 'ft900device1'
CONFIG_MOS_CREDENTIAL_USER  = None
CONFIG_MOS_CREDENTIAL_PASS  = None



############################################################################################
# mqtt manager class
############################################################################################
class mqtt_manager():

    def __init__(self):
        self.quits = False
        self.exiting = False
        self.server = CONFIG_SERVER

    ############################################################################################
    # thread_publisher
    ############################################################################################
    def thread_publisher(self, client_handle):

        # publish request continuously
        devices = ["hopper", "knuth", "turing"]
        while not self.quits:
            for j in range(3):

                # construct topic and payload
                sensor_value = str(urandom.getrandbits(32) % 10 + 30)
                charge_value = str(urandom.getrandbits(32) % 30 - 10)
                discharge_value = str(urandom.getrandbits(32) % 5)
                payload = '{\r\n "deviceId":' + '"' + devices[j] + '"' + ',\r\n "sensorReading":' + sensor_value + ',\r\n "batteryCharge":' + charge_value + ',\r\n "batteryDischargeRate":' + discharge_value + '\r\n}'
                
                if self.server == CONFIG_SERVER_AWS_IOT:
                    topic = "device/" + devices[j] + "/devicePayload"
                elif self.server == CONFIG_SERVER_AZURE_IOT:
                    topic = "devices/" + CONFIG_AZU_CREDENTIAL_ID + "/messages/events/"
                elif self.server == CONFIG_SERVER_GCP_IOT:
                    topic = "/devices/" + CONFIG_GCP_CREDENTIAL_ID + "/events"
                else:
                    topic = "device/" + devices[j] + "/devicePayload"

                # display topic and payload
                print(topic + " [" + str(len(payload)) + "]")
                print(payload)

                if self.quits:
                    break

                # publish packet
                try:
                    client_handle.publish(topic, payload)
                    print("")
                    time.sleep(1)
                except:
                    _thread.exit()
                    return

        _thread.exit()
        return

    ############################################################################################
    # generate_jwt
    ############################################################################################
    def generate_jwt(self, project_id, certificate_pkey, minutes):

        # json web token consist of a header, payload and signature
        # <base64-encoded header>.<base64-encoded payload>.<base64-encoded signature>
        
        # header
        hdr = '{"typ":"JWT","alg":"RS256"}'
        print(hdr)
        hdr = base64.b64encode(bytes(hdr, 'utf-8')) 
        
        # body
        iat = utime.time()
        exp = iat + minutes
        body = '{"iat":' + str(iat) + ',"exp":' + str(exp) + ',"aud":' + project_id + '}'
        print(body)
        body = base64.b64encode(bytes(body, 'utf-8'))
        
        # construct the jwt for signing
        jwt = hdr + "." + body
        print("jwt: {}".format(jwt))
        
        # jwt signature
        with open(certificate_pkey, 'rb') as f:
            pkey = f.read()
        #print(pkey)
        
        #print("0")
        #hashobj = hashlib.sha256(jwt.decode('utf-8'))
        #print("1")
        #hashobj.update(pkey.decode('utf-8'))
        #print("2")
        #signature = ubinascii.hexlify(hashobj.digest())
        #print("3")

        print("0")
        try:
            hashobj = hmac.new(pkey, msg=jwt, digestmod=hashlib.sha256)
        except Exception as e:
            sys.print_exception(e)
            #print("Oops!",sys.exc_info()[0],"occured.")
            return None
        print("1")
        try:
            signature = ubinascii.hexlify(hashobj.digest())
        except Exception as e:
            sys.print_exception(e)
            #print("Oops!",sys.exc_info()[0],"occured.")
            print("hmac.digest")
            return None
        print("2")

        print("signature: {}".format(signature))
        signature = base64.b64encode(signature)
        print("signature: {}".format(signature))

        jwt = str(jwt, 'utf-8')
        print("4")
        jwt += "." + signature.decode('utf-8')
        print("5")
        #jwt = str(jwt, 'utf-8')
        print(jwt)
        return jwt

    def setup(self):
        if self.server == CONFIG_SERVER_AWS_IOT:
            certificate_file = CONFIG_AWS_CERTIFICATE_FILE
            certificate_pkey = CONFIG_AWS_CERTIFICATE_PKEY
            certificate_ca   = CONFIG_AWS_CERTIFICATE_RTCA
            credentials_id   = CONFIG_AWS_CREDENTIAL_ID
            credentials_user = CONFIG_AWS_CREDENTIAL_USER
            credentials_pass = CONFIG_AWS_CREDENTIAL_PASS
        elif self.server == CONFIG_SERVER_GCP_IOT:
            certificate_file = CONFIG_GCP_CERTIFICATE_FILE
            certificate_pkey = CONFIG_GCP_CERTIFICATE_PKEY
            certificate_ca   = CONFIG_GCP_CERTIFICATE_RTCA
            credentials_id   = CONFIG_GCP_CREDENTIAL_ID
            credentials_user = CONFIG_GCP_CREDENTIAL_USER
            try:
                credentials_pass = self.generate_jwt(CONFIG_GCP_PROJECT_ID, certificate_pkey, 3600*12)
            except:
                print("generate_jwt exception")
                self.exiting = True
                _thread.exit()
                return
        elif self.server == CONFIG_SERVER_AZURE_IOT:
            certificate_file = CONFIG_AZU_CERTIFICATE_FILE
            certificate_pkey = CONFIG_AZU_CERTIFICATE_PKEY
            certificate_ca   = CONFIG_AZU_CERTIFICATE_RTCA
            credentials_id   = CONFIG_AZU_CREDENTIAL_ID
            credentials_user = CONFIG_AZU_CREDENTIAL_USER
            credentials_pass = CONFIG_AZU_CREDENTIAL_PASS
        elif self.server == CONFIG_SERVER_MOSQUITTO:
            certificate_file = CONFIG_MOS_CERTIFICATE_FILE
            certificate_pkey = CONFIG_MOS_CERTIFICATE_PKEY
            certificate_ca   = CONFIG_MOS_CERTIFICATE_RTCA
            credentials_id   = CONFIG_MOS_CREDENTIAL_ID
            credentials_user = CONFIG_MOS_CREDENTIAL_USER
            credentials_pass = CONFIG_MOS_CREDENTIAL_PASS

        if self.server[1] == 1883:
            client = MQTTClient(credentials_id, self.server[0], port=self.server[1], user=credentials_user, password=credentials_pass)
        else:
            with open(certificate_file, 'rb') as f:
                cert1 = f.read()
            with open(certificate_pkey, 'rb') as f:
                key1 = f.read()
            client_ssl  = { "cert":cert1.decode("utf-8"), "key":key1.decode("utf-8") }
            client = MQTTClient(credentials_id, self.server[0], port=self.server[1], user=credentials_user, password=credentials_pass, ssl=True, ssl_params=client_ssl)
        return client

    def connect(self, client):
        print("Connecting to {}:{}...".format(self.server[0], self.server[1]))
        client.connect()
        print("Connected!\r\n")

    ############################################################################################
    # thread_manager
    ############################################################################################
    def thread_manager(self):
        print("\r\n=======================================================================")
        print("ESP32 AWS/GCP/Azure IoT Demo")
        print("=======================================================================\r\n")

        client = self.setup()
        try:
            self.connect(client)
        except Exception as e:
            print("Connect exception!\r\n")
            sys.print_exception(e)
            self.exiting = True
            _thread.exit()
            return
        _thread.start_new_thread(self.thread_publisher, (client, ))

        # wait for user input to start the commander thread
        while not self.quits:
            key = input("").lower()
            if key == 'q':
                if client is not None:
                    client.disconnect()
                    client = None
                break
            elif key == 'h':
                self.usage()
            elif key=='d': # Disconnect from server
                if client is not None:
                    client.disconnect()
                    client = None
                time.sleep(1)
                self.usage()
            elif key=='c': # Connect to same server
                if client == None:
                    client = self.setup()
                    self.connect(client)
                    _thread.start_new_thread(self.thread_publisher, (client, ))
            elif key=='a': # Amazon Web Services
                if client == None:
                    self.server = CONFIG_SERVER_AWS_IOT
                    client = self.setup()
                    self.connect(client)
                    _thread.start_new_thread(self.thread_publisher, (client, ))
            elif key=='g': # Google Cloud
                if client == None:
                    self.server = CONFIG_SERVER_GCP_IOT
                    client = self.setup()
                    self.connect(client)
                    _thread.start_new_thread(self.thread_publisher, (client, ))
            elif key=='m': # Microsoft Azure
                if client == None:
                    self.server = CONFIG_SERVER_AZURE_IOT
                    client = self.setup()
                    self.connect(client)
                    _thread.start_new_thread(self.thread_publisher, (client, ))
            elif key=='e': # Eclipse Mosquitto
                if client == None:
                    self.server = CONFIG_SERVER_MOSQUITTO
                    client = self.setup()
                    self.connect(client)
                    _thread.start_new_thread(self.thread_publisher, (client, ))

        # display some message
        time.sleep(1)
        print("\r\n=======================================================================")
        print("ESP32 AWS/GCP/Azure IoT Demo exits...")
        print("=======================================================================\r\n\r\n")

        # set exiting status for wait function
        self.exiting = True

        _thread.exit()
        return

    def usage(self):
        time.sleep(1)
        print("\r\n=======================================================================")
        print("[MAIN] Usage:")
        print("[MAIN]   Press 'q' key to quit...")
        print("[MAIN]   Press 'd' key to disconnect...")
        print("[MAIN]   Press 'c' key to connect and publish...")
        print("[MAIN]   Press 'a' key to connect & publish to Amazon AWS IoT...")
        print("[MAIN]   Press 'g' key to connect & publish to Google Cloud IoT...")
        print("[MAIN]   Press 'm' key to connect & publish to Microsoft Azure IoT...")
        print("[MAIN]   Press 'e' key to connect & publish to Eclipse Mosquitto...")
        print("=======================================================================\r\n")

    def run(self):
        _thread.start_new_thread(self.thread_manager, ())
        return

    def wait(self):
        while not self.exiting:
            pass
        return

    def quit(self):
        self.quits = True

