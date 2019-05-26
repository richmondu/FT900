############################################################################################
# This is an FT900 Alexa Simulator
# It simulates the behavior of FT900 Alexa Client for the FT900 Alexa Demo
# It communicates with RPI Alexa Gateway which routes request/responses to/from Alexa cloud
# It implements 3 threads: Streamer [recv], Commander [send], Player [play]
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
import queue
import json





############################################################################################
# Configurations that can be set using application parameter
############################################################################################
CONF_SERVER_ADDR_DEFAULT             = '192.168.100.12'    # --serveraddr
CONF_SERVER_PORT_DEFAULT             = 11234               # --deviceid 11234+x-1 if --usediffacct is 1
CONF_DEVICE_ID_DEFAULT               = 2                   # --deviceid

############################################################################################
# Other configurations.
############################################################################################
CONF_FILENAME_REQUEST_TIME             = "audio/REQUEST_what_time_is_it.raw"
CONF_FILENAME_REQUEST_PERSON           = "audio/REQUEST_who_is.raw"
CONF_FILENAME_REQUEST_MUSIC            = "audio/REQUEST_play_music.raw"
CONF_FILENAME_REQUEST_NEWS             = "audio/REQUEST_play_live_news.raw"
CONF_FILENAME_REQUEST_BOOK             = "audio/REQUEST_play_audio_book.raw"
CONF_FILENAME_REQUEST_ALARM            = "audio/REQUEST_set_alarm.raw"
CONF_FILENAME_REQUEST_STOP             = "audio/REQUEST_stop.raw"
CONF_FILENAME_REQUEST_YES              = "audio/REQUEST_yes.raw"
CONF_FILENAME_REQUEST_WEATHER          = "audio/REQUEST_what_is_the_weather.raw"
CONF_FILENAME_REQUEST_ONEPLUSONE       = "audio/REQUEST_one_plus_one.RAW"
CONF_FILENAME_REQUEST_LISTTODO         = "audio/REQUEST_list_todo.RAW"
CONF_FILENAME_TIMESTAMP                = 0
CONF_TIMEOUT_SEND                      = 10
CONF_TIMEOUT_RECV                      = 10
CONF_RESET_TIMEOUT                     = 1
CONF_CHUNK_SIZE                        = 512
CONF_DISPLAYCARD_PORT_ADDITION         = 100
CONF_DISPLAYCARD_PLAYERINFOCARD_RENDER = 0
CONF_DISPLAYCARD_PLAYERINFOCARD_CLEAR  = 1
CONF_DISPLAYCARD_TEMPLATECARD_RENDER   = 2
CONF_DISPLAYCARD_TEMPLATECARD_CLEAR    = 3
CONF_DISPLAYCARD_IMAGE_PNG             = 4
CONF_DISPLAYCARD_IMAGE_JPG             = 5

############################################################################################
# Device capabilities
############################################################################################
DEVICE_CAPABILITIES_OFFSET_FORMAT    = 0
DEVICE_CAPABILITIES_OFFSET_BITDEPTH  = 4
DEVICE_CAPABILITIES_OFFSET_BITRATE   = 8
DEVICE_CAPABILITIES_OFFSET_CHANNEL   = 12
DEVICE_CAPABILITIES_MASK_FORMAT      = 0xF
DEVICE_CAPABILITIES_MASK_BITDEPTH    = 0xF
DEVICE_CAPABILITIES_MASK_BITRATE     = 0xF
DEVICE_CAPABILITIES_MASK_CHANNEL     = 0xF
DEVICE_CAPABILITIES_FORMAT_RAW       = 0
DEVICE_CAPABILITIES_FORMAT_MP3       = 1
DEVICE_CAPABILITIES_FORMAT_WAV       = 2
DEVICE_CAPABILITIES_FORMAT_AAC       = 3
DEVICE_CAPABILITIES_BITDEPTH_8       = 0
DEVICE_CAPABILITIES_BITDEPTH_16      = 1
DEVICE_CAPABILITIES_BITDEPTH_24      = 2
DEVICE_CAPABILITIES_BITDEPTH_32      = 3
DEVICE_CAPABILITIES_BITRATE_16000    = 0
DEVICE_CAPABILITIES_BITRATE_32000    = 1
DEVICE_CAPABILITIES_BITRATE_44100    = 2
DEVICE_CAPABILITIES_BITRATE_48000    = 3
DEVICE_CAPABILITIES_BITRATE_96000    = 4
DEVICE_CAPABILITIES_BITRATE_8000     = 5
DEVICE_CAPABILITIES_CHANNEL_1        = 0
DEVICE_CAPABILITIES_CHANNEL_2        = 1
CONF_AUDIO_SEND_FORMAT               = DEVICE_CAPABILITIES_FORMAT_RAW
CONF_AUDIO_SEND_BITDEPTH             = DEVICE_CAPABILITIES_BITDEPTH_16
CONF_AUDIO_SEND_BITRATE              = DEVICE_CAPABILITIES_BITRATE_16000
CONF_AUDIO_SEND_CHANNEL              = DEVICE_CAPABILITIES_CHANNEL_1
CONF_AUDIO_RECV_FORMAT               = DEVICE_CAPABILITIES_FORMAT_RAW
CONF_AUDIO_RECV_BITDEPTH             = DEVICE_CAPABILITIES_BITDEPTH_16
CONF_AUDIO_RECV_BITRATE              = DEVICE_CAPABILITIES_BITRATE_16000
CONF_AUDIO_RECV_CHANNEL              = DEVICE_CAPABILITIES_CHANNEL_1

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
# avs_set_capabilities
############################################################################################
def avs_set_capabilities(format, depth, rate, channel):
    cap = 0
    cap |= (format  & DEVICE_CAPABILITIES_MASK_FORMAT)   << DEVICE_CAPABILITIES_OFFSET_FORMAT
    cap |= (depth   & DEVICE_CAPABILITIES_MASK_BITDEPTH) << DEVICE_CAPABILITIES_OFFSET_BITDEPTH
    cap |= (rate    & DEVICE_CAPABILITIES_MASK_BITRATE)  << DEVICE_CAPABILITIES_OFFSET_BITRATE
    cap |= (channel & DEVICE_CAPABILITIES_MASK_CHANNEL)  << DEVICE_CAPABILITIES_OFFSET_CHANNEL
    return (cap & 0xFFFF)

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

        # Set device info
        buf = bytes()
        buf += struct.pack('I', g_deviceid)
        sendCap = avs_set_capabilities(CONF_AUDIO_SEND_FORMAT, CONF_AUDIO_SEND_BITDEPTH, CONF_AUDIO_SEND_BITRATE, CONF_AUDIO_SEND_CHANNEL)
        buf += struct.pack('H', sendCap)
        recvCap = avs_set_capabilities(CONF_AUDIO_RECV_FORMAT, CONF_AUDIO_RECV_BITDEPTH, CONF_AUDIO_RECV_BITRATE, CONF_AUDIO_RECV_CHANNEL)
        buf += struct.pack('H', recvCap)

        # Set timeout
        timeval = struct.pack('ll', CONF_TIMEOUT_SEND, 0)
        g_socket.setsockopt(socket.SOL_SOCKET, socket.SO_SNDTIMEO, timeval)

        # Send device info
        g_socket.sendall(buf)

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
# avs_send_audio
############################################################################################
def avs_send_audio(file_name):

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
    if CONF_AUDIO_SEND_BITDEPTH == DEVICE_CAPABILITIES_BITDEPTH_8:
        file_bytes = audioop.lin2ulaw(file_bytes, 2)
    file_size = len(file_bytes)

    # Send size of Alexa request
    timeval = struct.pack('ll', CONF_TIMEOUT_SEND, 0)
    g_socket.setsockopt(socket.SOL_SOCKET, socket.SO_SNDTIMEO, timeval)
    val = struct.pack('i', file_size)
    g_socket.sendall(val)

    # Send Alexa request
    # send in chunks instead of sending all to simulate RS485 slowness
    # g_socket.sendall(bytes(file_bytes))
    sent_data = 0
    send_size = CONF_CHUNK_SIZE
    file_bytes = bytes(file_bytes)
    #print("{}".format(file_size))
    while sent_data < file_size:
        if file_size - sent_data < send_size:
            send_size = file_size - sent_data
        g_socket.sendall(file_bytes[sent_data:sent_data+send_size])
        sent_data += send_size
        #print("{} {}".format(send_size, sent_data))

############################################################################################
# avs_recv_audio
############################################################################################
def avs_recv_audio(queue_data):

    global g_socket
    global g_quit
    
    try:
        val = g_socket.recv(4)
        if not val:
            print("recv error")
            g_quit = True
        else:
            file_size_recv = struct.unpack('I', val[:])[0]
            recv_size = CONF_CHUNK_SIZE
            recved_size = 0
            #print("streaming {} bytes".format(file_size_recv))

            while g_quit is False:
                try:
                    # recv data
                    data = g_socket.recv(recv_size)
                    if not data:
                        print("Error: recv failed! not data")
                        g_quit = True
                        break

                    # get length of data
                    len_data = len(data)
                    if len_data <= 0:
                        print("Error: recv failed! len_data <= 0")
                        print(len_data)
                        break

                    # update total bytes recvd
                    recved_size += len_data
                    if recved_size == file_size_recv:
                        break

                    # queue the data
                    queue_data.put(data)
                    # print("queue_data.put {}".format(len_data))

                    # compute bytes to recv
                    if recv_size > file_size_recv - recved_size: 
                        recv_size = file_size_recv - recved_size

                except socket.error as e:
                    print("error")
                    print(e.args[0])

    except:
        print("avs_recv_audio exception")

    return

############################################################################################
# avs_play_audio
############################################################################################
def avs_play_audio(queue_data, stream):

    # Get data from queue
    try:
        data = queue_data.get(block=True, timeout=1)
        if len(data) == 0:
            return False
    except:
        return False
    #print("queue_data.get {}".format(len(data)))

    # Decompress and play data
    if CONF_AUDIO_RECV_BITDEPTH == DEVICE_CAPABILITIES_BITDEPTH_8:
        data = audioop.ulaw2lin(data, 2)
    stream.write(data)

    queue_data.task_done()
    return True


############################################################################################
# avs_recv_and_play_audio
############################################################################################
def avs_recv_and_play_audio():

    global g_quit
    global g_socket

    # Get the rate based on configuration
    bitrate = 0
    if (CONF_AUDIO_RECV_BITRATE == DEVICE_CAPABILITIES_BITRATE_16000):
        bitrate = 16000
    elif (CONF_AUDIO_RECV_BITRATE == DEVICE_CAPABILITIES_BITRATE_32000):
        bitrate = 32000
    elif (CONF_AUDIO_RECV_BITRATE == DEVICE_CAPABILITIES_BITRATE_44100):
        bitrate = 44100
    elif (CONF_AUDIO_RECV_BITRATE == DEVICE_CAPABILITIES_BITRATE_48000):
        bitrate = 48000
    elif (CONF_AUDIO_RECV_BITRATE == DEVICE_CAPABILITIES_BITRATE_8000):
        bitrate = 8000

    # Get the channel based on configuration
    channel = 0
    if (CONF_AUDIO_RECV_CHANNEL == DEVICE_CAPABILITIES_CHANNEL_1):
        channel = 1
    elif (CONF_AUDIO_RECV_CHANNEL == DEVICE_CAPABILITIES_CHANNEL_2):
        channel = 2

    audio = pyaudio.PyAudio()
    stream = audio.open(format=pyaudio.paInt16, channels=channel, rate=bitrate, output=True)

    try:
        val = g_socket.recv(4)
        if not val:
            print("recv error")
            g_quit = True
        else:
            file_size_recv = struct.unpack('I', val[:])[0]
            recv_size = CONF_CHUNK_SIZE
            recved_size = 0
            #print("streaming {} bytes".format(file_size_recv))

            while g_quit is False:
                try:
                    # recv data
                    data = g_socket.recv(recv_size)
                    if not data:
                        print("Error: recv failed! not data")
                        g_quit = True
                        break

                    # get length of data
                    len_data = len(data)
                    if len_data <= 0:
                        print("Error: recv failed! len_data <= 0")
                        break

                    # update total bytes recvd
                    recved_size += len_data
                    if recved_size == file_size_recv:
                        break

                    # play recvd data
                    if CONF_AUDIO_RECV_BITDEPTH == DEVICE_CAPABILITIES_BITDEPTH_8:
                        data = audioop.ulaw2lin(data, 2)
                    stream.write(data)

                    # compute bytes to recv
                    if recv_size > file_size_recv - recved_size: 
                        recv_size = file_size_recv - recved_size

                except socket.error as e:
                    print("error")
                    print(e.args[0])

    except:
        print("exception")

    stream.stop_stream()
    stream.close()
    audio.terminate()


############################################################################################
# thread_player
############################################################################################
class thread_player(threading.Thread):

    def __init__(self, threadID, name, queue_data):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.queue_data = queue_data

    def run(self):
        sleep(1)
        global g_quit

        # Get the rate based on configuration
        bitrate = 0
        if (CONF_AUDIO_RECV_BITRATE == DEVICE_CAPABILITIES_BITRATE_16000):
            bitrate = 16000
        elif (CONF_AUDIO_RECV_BITRATE == DEVICE_CAPABILITIES_BITRATE_32000):
            bitrate = 32000
        elif (CONF_AUDIO_RECV_BITRATE == DEVICE_CAPABILITIES_BITRATE_44100):
            bitrate = 44100
        elif (CONF_AUDIO_RECV_BITRATE == DEVICE_CAPABILITIES_BITRATE_48000):
            bitrate = 48000
        elif (CONF_AUDIO_RECV_BITRATE == DEVICE_CAPABILITIES_BITRATE_8000):
            bitrate = 8000

        # Get the channel based on configuration
        channel = 0
        if (CONF_AUDIO_RECV_CHANNEL == DEVICE_CAPABILITIES_CHANNEL_1):
            channel = 1
        elif (CONF_AUDIO_RECV_CHANNEL == DEVICE_CAPABILITIES_CHANNEL_2):
            channel = 2

        # Initialize audio player
        audio = pyaudio.PyAudio()
        stream = audio.open(format=pyaudio.paInt16, channels=channel, rate=bitrate, output=True)

        # Dequeue and play audio
        while g_quit is False:
            if not avs_play_audio(self.queue_data, stream):
                sleep(1)

        # Uninitialize audio player
        stream.stop_stream()
        stream.close()
        audio.terminate()

        # Drain the queue
        print("\n[PLAYER] Playback exiting...")
        while not self.queue_data.empty():
            data = self.queue_data.get()
            self.queue_data.task_done()

        print("\n[PLAYER] Playback exited!\n")
        return


############################################################################################
# thread_streamer
############################################################################################
class thread_streamer(threading.Thread):

    def __init__(self, threadID, name, queue_data):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.queue_data = queue_data

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
                avs_recv_audio(self.queue_data)
                #avs_recv_and_play_audio()
            except:
                print("\navs_recv_and_play_audio exception!")
                break

        print("\n[STREAMER] Disconnecting from Alexa provider...\n")
        if g_connected is True:
            avs_disconnect()
            g_connected = False
        g_quit = True
        print("\n[STREAMER] Disconnected from Alexa provider!\n")

        return


############################################################################################
# thread_commander
############################################################################################
class thread_commander(threading.Thread):

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
            avs_send_audio(self.file_request)
            print("[{0:.0f} ms]".format((time()-start)*1000))

        return


############################################################################################
# thread_renderer
############################################################################################
class thread_renderer(threading.Thread):

    def __init__(self, threadID, name):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name

    def run(self):
        global g_quit
        global g_serverport
        global g_connected

        while not g_connected:
            sleep(1)

        if g_quit is False:
            try:
                socket_handle = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                socket_handle.bind(('', g_serverport + CONF_DISPLAYCARD_PORT_ADDITION))
                socket_handle.listen(3)
                socket_handle.settimeout(1)
            except:
                print('\n[RENDERER] Socket exception')
                return

        while g_quit is False:
            try:
                (conn, (ip, port)) = socket_handle.accept()
            except:
                continue

            # receive the header
            val = conn.recv(8)
            if not val:
                print("\n[RENDERER] recv header error")
                g_quit = True
            else:
                # size: unsigned int
                # type: unsigned char
                # reserved1: unsigned char
                # reserved2: unsigned short
                data_size = struct.unpack('I', val[0:4])[0]
                data_type = struct.unpack('B', val[4:5])[0]
                data_reserved1 = struct.unpack('B', val[5:6])[0]
                data_reserved2 = struct.unpack('H', val[6:8])[0]
                print('\n[RENDERER] Display card {}:{}:{}:{}'.format(data_size, data_type, data_reserved1, data_reserved2))
                if data_type == CONF_DISPLAYCARD_PLAYERINFOCARD_RENDER:
                    print('CONF_DISPLAYCARD_PLAYERINFOCARD_RENDER')
                elif data_type == CONF_DISPLAYCARD_PLAYERINFOCARD_CLEAR:
                    print('CONF_DISPLAYCARD_PLAYERINFOCARD_CLEAR')
                elif data_type == CONF_DISPLAYCARD_TEMPLATECARD_RENDER:
                    print('CONF_DISPLAYCARD_TEMPLATECARD_RENDER')
                elif data_type == CONF_DISPLAYCARD_TEMPLATECARD_CLEAR:
                    print('CONF_DISPLAYCARD_TEMPLATECARD_CLEAR')
                elif data_type == CONF_DISPLAYCARD_IMAGE_PNG:
                    print('CONF_DISPLAYCARD_IMAGE_PNG')
                elif data_type == CONF_DISPLAYCARD_IMAGE_JPG:
                    print('CONF_DISPLAYCARD_IMAGE_JPG')

            # receive the actual json_payload
            if data_size > 0:
                recv_size = 0
                json_payload = bytes()
                while recv_size < data_size:
                    payload = conn.recv(data_size)
                    if not payload:
                        print("\n[RENDERER] recv data error")
                        g_quit = True
                        break
                    else:
                        # print(len(payload))
                        recv_size += len(payload)
                        json_payload += payload
                # print("total={}".format(len(json_payload)))
                self.printJSON(json_payload, data_type)

            sleep(1)
            conn.close()

        socket_handle.close()
        print("\n[RENDERER] Exited!\n")
        return

    def printJSON(self, json_payload, data_type):
        try:
            json_payload = json_payload.decode("utf-8")
            json_payload = json.loads(json_payload)
            print("")
            for key,val in json_payload.items():
                print("{}: {}\r\n".format(key, val))
        except:
            print(len(json_payload))


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
    print("[MAIN]   Press 'w' key to ask Alexa what is the weather...")
    print("[MAIN]   Press 'o' key to ask Alexa one plus one...")
    print("[MAIN]   Press 'l' key to ask Alexa what's on my todo list...")
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
        queue_data = queue.Queue()
        threads = []
        t = thread_streamer(0, "streamer", queue_data)
        t.start()
        t = thread_player(0, "player", queue_data)
        t.start()
        t = thread_renderer(0, "renderer")
        t.start()
        threads.append(t)

        # Process user input
        while g_quit is False:
            if msvcrt.kbhit():
                key = msvcrt.getch().decode('utf-8').lower()
                print(key)
                if key=='q':
                    # send stop command to stop music or alarm
                    t = thread_commander (1, "commander", CONF_FILENAME_REQUEST_STOP)
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
                    t = thread_commander (2, "commander", CONF_FILENAME_REQUEST_TIME)
                    t.start()
                elif key=='p':
                    t = thread_commander (3, "commander", CONF_FILENAME_REQUEST_PERSON)
                    t.start()
                elif key=='m':
                    t = thread_commander (4, "commander", CONF_FILENAME_REQUEST_MUSIC)
                    t.start()
                elif key=='n':
                    t = thread_commander (5, "commander", CONF_FILENAME_REQUEST_NEWS)
                    t.start()
                elif key=='b':
                    t = thread_commander (6, "commander", CONF_FILENAME_REQUEST_BOOK)
                    t.start()
                elif key=='a':
                    t = thread_commander (7, "commander", CONF_FILENAME_REQUEST_ALARM)
                    t.start()
                elif key=='s':
                    t = thread_commander (8, "commander", CONF_FILENAME_REQUEST_STOP)
                    t.start()
                elif key=='y':
                    t = thread_commander (9, "commander", CONF_FILENAME_REQUEST_YES)
                    t.start()
                elif key=='w':
                    t = thread_commander (10, "commander", CONF_FILENAME_REQUEST_WEATHER)
                    t.start()
                elif key=='o':
                    t = thread_commander (11, "commander", CONF_FILENAME_REQUEST_ONEPLUSONE)
                    t.start()
                elif key=='l':
                    t = thread_commander (12, "commander", CONF_FILENAME_REQUEST_LISTTODO)
                    t.start()
                else:
                    sleep(1)

        for t in threads:
            t.join()

        if g_exit is True:
            break

        print("\nRetrying in {} seconds".format(CONF_RESET_TIMEOUT), end='', flush=True)
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

