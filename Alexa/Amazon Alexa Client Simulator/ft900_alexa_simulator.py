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
import threading
import msvcrt
import argparse



############################################################################################
# Configurations that can be set using application parameter
############################################################################################
CONF_SERVER_ADDR_DEFAULT    = '192.168.100.12'    # --serveraddr
CONF_SERVER_PORT_DEFAULT    = 11234               # --deviceid 11234+x-1 if --usediffacct is 1
CONF_DEVICE_ID_DEFAULT      = 2                   # --deviceid

############################################################################################
# Other configurations.
############################################################################################
CONF_FILENAME_REQUEST_TIME  = "audio/REQUEST_what_time_is_it.raw"
CONF_FILENAME_REQUEST_PERSON= "audio/REQUEST_who_is.raw"
CONF_FILENAME_REQUEST_MUSIC = "audio/REQUEST_play_music.raw"
CONF_FILENAME_REQUEST_NEWS  = "audio/REQUEST_play_live_news.raw"
CONF_FILENAME_REQUEST_BOOK  = "audio/REQUEST_play_audio_book.raw"
CONF_FILENAME_REQUEST_ALARM = "audio/REQUEST_set_alarm.raw"
CONF_FILENAME_REQUEST_STOP  = "audio/REQUEST_stop.raw"
CONF_FILENAME_REQUEST_YES   = "audio/REQUEST_yes.raw"
CONF_FILENAME_TIMESTAMP     = 0
CONF_TIMEOUT_SEND           = 10
CONF_TIMEOUT_RECV           = 10
CONF_RESET_TIMEOUT          = 1

############################################################################################
# Sampling rates.
############################################################################################
SAMPLING_RATE_44100HZ       = 1
SAMPLING_RATE_48KHZ         = 2
SAMPLING_RATE_32KHZ         = 3
SAMPLING_RATE_16KHZ         = 4
SAMPLING_RATE_8KHZ          = 5
CONF_SAMPLING_RATE          = SAMPLING_RATE_16KHZ

############################################################################################
# Global variable
############################################################################################
g_usediffacct = 0
g_serveraddr = CONF_SERVER_ADDR_DEFAULT
g_serverport = CONF_SERVER_PORT_DEFAULT
g_deviceid = CONF_DEVICE_ID_DEFAULT
g_socket = None
g_quit = False
g_connected = False



############################################################################################
# avs_connect
############################################################################################
def avs_connect():

    global g_socket
    global g_serveraddr
    global g_serverport

    g_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        # Connect to server
        err = g_socket.connect_ex((g_serveraddr, g_serverport))
        if err != 0:
            g_socket.close()
            return 0

        # Send device ID
        timeval = struct.pack('ll', CONF_TIMEOUT_SEND, 0)
        g_socket.setsockopt(socket.SOL_SOCKET, socket.SO_SNDTIMEO, timeval)
        val = struct.pack('I', g_deviceid)
        g_socket.sendall(val)

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
    if not val:
        return
    file_size_recv = int(struct.unpack('i', val[:])[0])
    
    # Receive Alexa response
    #timeval = struct.pack('ll', CONF_TIMEOUT_RECV, 0)
    #g_socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, timeval)
    
#    data = g_socket.recv(1024)
#    len_data = len(data)
#    while len_data > 0:
#        data2 = g_socket.recv(1024)
#        len_data = len(data2)
#        if len_data <= 0:
#            break
#        data = data + data2
        
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
    audio = pyaudio.PyAudio()
    stream = audio.open(format=pyaudio.paInt16, channels=1, rate=rate, output=True)
    file_name_read = file_name
    file_read = open(file_name_read, "rb")
    stream.write(file_read.read())
    stream.stop_stream()
    stream.close()
    audio.terminate()

############################################################################################
# avs_recv_and_play_response
############################################################################################
def avs_recv_and_play_response():

    global g_quit

    audio = pyaudio.PyAudio()
    stream = audio.open(format=pyaudio.paInt16, channels=1, rate=16000, output=True)

    try:
        val = g_socket.recv(4)
        if val:
            file_size_recv = struct.unpack('I', val[:])[0]
            recv_size = 512
            recved_size = 0
            print("file_size_recv = {}".format(file_size_recv))
            
            while g_quit is False:
                try:
                    data = g_socket.recv(recv_size)
                    if not data:
                        print("Error: recv failed! not data")
                        g_quit = True
                        break
                    len_data = len(data)
                    if len_data <= 0:
                        print("Error: recv failed! len_data <= 0")
                        print(len_data)
                        break
                    recved_size += len_data
                    #print("len_data = {} {}".format(len_data, recved_size))
                    if recved_size == file_size_recv:
                        break
                    stream.write(audioop.ulaw2lin(data, 2))
                    if recv_size > file_size_recv - recved_size: 
                        recv_size = file_size_recv - recved_size
                    
                except socket.error as e:
                    print("error")
                    print(e.args[0])
                #sleep(0.05)
        else:
            print("recv error")
            g_quit = True
    except:
        print("exception")

    stream.stop_stream()
    stream.close()
    audio.terminate()


############################################################################################
# thread_fxn_streamer
############################################################################################
class thread_fxn_streamer(threading.Thread):

    def __init__(self, threadID, name):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name

    def run(self):
        sleep(1)
        global g_quit
        global g_connected
        global g_deviceid
        global g_serveraddr
        global g_serverport

        g_connected = False
        counter = 0
        while g_quit is False:
            print("\n[STREAMER] Connecting to Alexa provider... {}:{} [ID:{}] [{}]".format(g_serveraddr, g_serverport, g_deviceid, counter))
            if avs_connect() == 0:
                counter = counter + 1
                sleep(3)
                continue
            print("\n[STREAMER] Connected successfully!")
            break

        if g_quit is False:
            usage()
            g_connected = True

        while g_quit is False:
            try: 
                #file_name = "test.raw"
                #avs_recv_response(file_name)
                #avs_play_response(file_name)
                avs_recv_and_play_response()
            except:
                print("\navs_recv_and_play_response exception!")
                break

        print("\n[STREAMER] Disconnecting from Alexa provider...\n")
        if g_connected is True:
            avs_disconnect()
            g_connected = False
        g_quit = True

        return


############################################################################################
# thread_fxn_commander
############################################################################################
class thread_fxn_commander(threading.Thread):

    def __init__(self, threadID, name, file_request):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.file_request = file_request

    def run(self):
        global g_quit
        global g_connected

        if g_connected == False:
            print("[COMMANDER] Not connected...")
            return

        if g_quit is False:
            print("[COMMANDER] Sending Alexa query [{}]...".format(self.file_request), end='', flush=True)
            start = time()
            avs_send_request(self.file_request)
            print("[{0:.0f} ms]".format((time()-start)*1000))

        return

############################################################################################
# Usage
############################################################################################
def usage():

    print("===============================================================")
    print("[MAIN] Usage:")
    print("[MAIN]   Press 'q' key to quit (will send stop b4 quitting)...")
    print("[MAIN]   Press 'e' key to exit application...")
    print("[MAIN]   Press 't' key to ask Alexa what time is it...")
    print("[MAIN]   Press 'p' key to ask Alexa who is...")
    print("[MAIN]   Press 'm' key to ask Alexa to play music...")
    print("[MAIN]   Press 'n' key to ask Alexa to play live news...")
    print("[MAIN]   Press 'b' key to ask Alexa to play audio book...")
    print("[MAIN]   Press 'a' key to ask Alexa to set alarm...")
    print("[MAIN]   Press 's' key to tell Alexa to stop...")
    print("[MAIN]   Press 'y' key to tell Alexa yes...")
    print("===============================================================\n")

############################################################################################
# Main loop
############################################################################################
def main(args, argc):

    global g_usediffacct
    global g_deviceid
    global g_quit
    global g_connected
    global g_serveraddr
    global g_serverport

    if sys.version_info < (3, 0):
        print("ERROR: Tested using Python 3.6.6 only!")
        return


    # g_deviceid should be in the range of [1, 16]
    g_deviceid = int(args.deviceid)
    if g_deviceid < 1 or g_deviceid > 16:
        print("ERROR: Invalid device ID! Should be in the range of [1-16]")
        return

    # Device 1 connects to port CONF_SERVER_PORT_DEFAULT + 0
    # Device 2 connects to port CONF_SERVER_PORT_DEFAULT + 1
    # Device X connects to port CONF_SERVER_PORT_DEFAULT + X-1
    g_usediffacct = int(args.usediffacct)
    if g_usediffacct:
        g_serverport = CONF_SERVER_PORT_DEFAULT + g_deviceid - 1
    else:
        g_serverport = CONF_SERVER_PORT_DEFAULT
    g_serveraddr = args.serveraddr


    while True:
        print("\n===============================================================")
        print("FT900 Alexa Demo simulator")
        print("===============================================================\n")

        # Initialize the streamer thread
        g_quit = False
        g_exit = False
        threads = []
        t = thread_fxn_streamer(0, "streamer")
        t.start()
        threads.append(t)

        # Process user input
        while g_quit is False:
            if msvcrt.kbhit():
                key = msvcrt.getch().decode('utf-8').lower()
                print(key)
                if key=='q':
                    # send stop command to stop music or alarm
                    t = thread_fxn_commander (1, "commander", CONF_FILENAME_REQUEST_STOP)
                    t.start()
                    # wait for thread to finish
                    t.join()
                    sleep(1)
                    # trigger streamer thread to quit
                    g_quit = True
                    if g_connected is True:
                        g_connected = False
                        avs_disconnect()
                    break
                elif key=='e':
                    if g_connected is True:
                        g_connected = False
                        avs_disconnect()
                    g_quit = True
                    g_exit = True
                    break
                elif key=='t':
                    t = thread_fxn_commander (2, "commander", CONF_FILENAME_REQUEST_TIME)
                    t.start()
                elif key=='p':
                    t = thread_fxn_commander (3, "commander", CONF_FILENAME_REQUEST_PERSON)
                    t.start()
                elif key=='m':
                    t = thread_fxn_commander (4, "commander", CONF_FILENAME_REQUEST_MUSIC)
                    t.start()
                elif key=='n':
                    t = thread_fxn_commander (5, "commander", CONF_FILENAME_REQUEST_NEWS)
                    t.start()
                elif key=='b':
                    t = thread_fxn_commander (6, "commander", CONF_FILENAME_REQUEST_BOOK)
                    t.start()
                elif key=='a':
                    t = thread_fxn_commander (7, "commander", CONF_FILENAME_REQUEST_ALARM)
                    t.start()
                elif key=='s':
                    t = thread_fxn_commander (8, "commander", CONF_FILENAME_REQUEST_STOP)
                    t.start()
                elif key=='y':
                    t = thread_fxn_commander (9, "commander", CONF_FILENAME_REQUEST_YES)
                    t.start()
                else:
                    sleep(1)

        for t in threads:
            t.join()

        if g_exit is True:
            break

        print("Retrying in {} seconds".format(CONF_RESET_TIMEOUT), end='', flush=True)
        for i in range(CONF_RESET_TIMEOUT):
            sleep(1)
            print(".", end='', flush=True)

    return


def parse_arguments(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument('--serveraddr', required=True, default=CONF_SERVER_ADDR_DEFAULT,
        help='Server IP address to use.')
    parser.add_argument('--deviceid', required=True, default=CONF_DEVICE_ID_DEFAULT,
        help='DeviceID to use.')
    parser.add_argument('--usediffacct', required=True, default=0,
        help='Flag to specify if using different account.')
    return parser.parse_args(argv)


if __name__ == '__main__':
    main(parse_arguments(sys.argv[1:]), len(sys.argv)-1)

