############################################################################################
# This is an FT900 Alexa Simulator
# It simulates the behavior of FT900 Alexa Client for the FT900 Alexa Demo
# It communicates with RPI Alexa Gateway which routes request/responses to/from Alexa cloud
############################################################################################
import sys
import socket
import time
from time import sleep, time
from datetime import datetime
import os
import struct
import pyaudio
import audioop



############################################################################################
# Configurations.
############################################################################################
CONF_SERVER_ADDR        = '192.168.22.5'
CONF_SERVER_PORT        = 11234
CONF_FILENAME_REQUEST   = "REQUEST.raw"
CONF_FILENAME_RESPONSE  = "RESPONSE.raw"
CONF_FILENAME_TIMESTAMP = 0
CONF_TIMEOUT_SEND       = 10
CONF_TIMEOUT_RECV       = 10

############################################################################################
# Sampling rates.
############################################################################################
SAMPLING_RATE_44100HZ   = 1
SAMPLING_RATE_48KHZ     = 2
SAMPLING_RATE_32KHZ     = 3
SAMPLING_RATE_16KHZ     = 4
SAMPLING_RATE_8KHZ      = 5
CONF_SAMPLING_RATE      = SAMPLING_RATE_16KHZ

############################################################################################
# Global variable
############################################################################################
g_socket = None



############################################################################################
# avs_connect
############################################################################################
def avs_connect():

    global g_socket
    
    g_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        g_socket.connect((CONF_SERVER_ADDR, CONF_SERVER_PORT))
    except:
        g_socket.close()
        return 0

    return 1

############################################################################################
# avs_disconnect
############################################################################################
def avs_disconnect():

    global g_socket
    g_socket.close()

############################################################################################
# avs_send_request
############################################################################################
def avs_send_request(file_name):

    global g_socket
    
    # Get file size
    file_size = 0
    if os.path.isfile(file_name):
        file_info = os.stat(file_name)
        file_size = file_info.st_size

    # Get file bytes
    file = open(file_name, "rb")
    file_bytes = file.read(file_size)
    file.close()

    # Compress file bytes from 16-bit to 8-bit
    file_bytes = audioop.lin2ulaw(file_bytes, 2)
    file_size = len(file_bytes)

    # Send size of Alexa request
    timeval = struct.pack('ll', CONF_TIMEOUT_SEND, 0)
    g_socket.setsockopt(socket.SOL_SOCKET, socket.SO_SNDTIMEO, timeval)
    val = struct.pack('i', file_size)
    g_socket.sendall(val)
    
    # Send sample rate to be used
    timeval = struct.pack('ll', CONF_TIMEOUT_SEND, 0)
    g_socket.setsockopt(socket.SOL_SOCKET, socket.SO_SNDTIMEO, timeval)
    val = struct.pack('i', CONF_SAMPLING_RATE)
    g_socket.sendall(val)
    
    # Send Alexa request
    g_socket.sendall(bytes(file_bytes))

############################################################################################
# avs_recv_response
############################################################################################
def avs_recv_response(file_name):

    global g_socket
    
    # Receive size of Alexa response
    #timeval = struct.pack('ll', CONF_TIMEOUT_RECV, 0)
    #g_socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, timeval)
    val = g_socket.recv(4)
    file_size_recv = struct.unpack('i', val[:])[0]
    
    # Receive Alexa response
    #timeval = struct.pack('ll', CONF_TIMEOUT_RECV, 0)
    #g_socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, timeval)
    file_size_recv = int(file_size_recv)
    data = g_socket.recv(file_size_recv)
    file_size_recv = file_size_recv - len(data)
    while (file_size_recv):
        data2 = g_socket.recv(file_size_recv)
        file_size_recv = file_size_recv - len(data2)
        data = data + data2

    # Expanding Alexa response from 8-bit to 16-bit
    data = audioop.ulaw2lin(data, 2)

    # Get filename with timestamp
    file_name_recv = file_name
    if CONF_FILENAME_TIMESTAMP:
        timestamp = datetime.now().strftime("%Y%m%d%H%M%S")
        file_name_recv = file_name[0:file_name.lower().find(".raw")] + "_" + str(timestamp) + ".raw"
    
    # Save Alexa response to file
    file_recv = open(file_name_recv, "wb")
    file_recv.write(data)
    file_recv.close()
    return file_name_recv

############################################################################################
# avs_play_response
############################################################################################
def avs_play_response(file_name):

    # Get the rate based on sampling rate
    rate = 0
    if (CONF_SAMPLING_RATE == SAMPLING_RATE_16KHZ):
        rate = 16000
    elif (CONF_SAMPLING_RATE == SAMPLING_RATE_44100HZ):
        rate = 44100
    elif (CONF_SAMPLING_RATE == SAMPLING_RATE_48KHZ):
        rate = 48000
    elif (CONF_SAMPLING_RATE == SAMPLING_RATE_32KHZ):
        rate = 32000
    elif (CONF_SAMPLING_RATE == SAMPLING_RATE_8KHZ):
        rate = 8000
        
    # Play alexa response from file
    p = pyaudio.PyAudio()
    stream = p.open(format=pyaudio.paInt16, channels=1, rate=rate, output=True)
    file_name_read = file_name
    file_read = open(file_name_read, "rb")
    stream.write(file_read.read())
    stream.stop_stream()
    stream.close()
    p.terminate()



############################################################################################
# Main loop
############################################################################################
def main(args, argc):

    if sys.version_info < (3, 0):
        print("Error: Tested using Python 3 only!")
        return
    
    if argc == 2:
        file_name_request = args[0]
        file_name_response = args[1]
    else:
        file_name_request = CONF_FILENAME_REQUEST
        file_name_response = CONF_FILENAME_RESPONSE


    while True:
    
        print("\nConnecting to Alexa provider... {}:{}\n".format(CONF_SERVER_ADDR, CONF_SERVER_PORT))
        if avs_connect() == 0:
            sleep(3)
            continue


        try:
            print("Sending Alexa query...", end='', flush=True)
            start = time()
            avs_send_request(file_name_request)
            print("[{0:.0f} ms]".format((time()-start)*1000))
            
            print("Receiving Alexa response...", end='', flush=True)
            start2 = time()
            file_name = avs_recv_response(file_name_response)
            print("[{0:.0f} ms]".format((time()-start2)*1000))

            print("Playing Alexa response...", end='', flush=True)
            start3 = time()
            avs_play_response(file_name)
            print("[{0:.0f} ms]".format((time()-start3)*1000))

            print("Performance: {0:.0f} ms".format((time()-start)*1000))
        except:
            print("Error!")


        print("\nClosing TCP connection...\n")		
        avs_disconnect()

        sleep(10)


if __name__ == '__main__':
    main(sys.argv[1:], len(sys.argv)-1)
