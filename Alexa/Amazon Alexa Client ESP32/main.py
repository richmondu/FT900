import socket
import struct
import time



CONFIG_SERVER_IP = '192.168.100.12'
CONFIG_SERVER_PORT = 11234
CONFIG_DEVICE_ID = 1
CONFIG_RECV_CHUNK_SIZE = 512
CONFIG_CONNECT_RETRY_SECS = 3



print("\r\n===============================================================")
print("ESP32 Alexa Demo")
print("===============================================================\r\n")

while True:
    # create sockets
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # connect to server
    connect_retry = 0
    sockaddr = socket.getaddrinfo(CONFIG_SERVER_IP, CONFIG_SERVER_PORT)[0][-1]
    while True:
        try:
            print("Connecting to Alexa provider... {}:{} [ID:{}] [{}]\r\n".format(CONFIG_SERVER_IP, CONFIG_SERVER_PORT, CONFIG_DEVICE_ID, connect_retry))
            s.connect(sockaddr)
            print("Connected successfully!")
            break
        except:
            connect_retry += 1
            time.sleep(CONFIG_CONNECT_RETRY_SECS)
            continue

    # send the device id
    val = struct.pack('I', CONFIG_DEVICE_ID)
    s.send(val)


    while True:
        # receive the size of audio segment
        val = s.recv(4)
        if not val:
            print("Recv size ERROR")
            break
        total_size = int(struct.unpack('i', val[:])[0])
        print(total_size)

        # receive the audio segment
        want_recv = CONFIG_RECV_CHUNK_SIZE
        total_recv = 0
        while total_recv < total_size:
            if total_size-total_recv < want_recv:
                want_recv = total_size-total_recv
            #print(want_recv)
            val = s.recv(want_recv)
            if not val:
                print("Recv size ERROR2")
                break
            total_recv += len(val)
            #print(total_recv)

    # close socket
    s.close()