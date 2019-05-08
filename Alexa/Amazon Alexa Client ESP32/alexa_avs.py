import socket
import uselect
import struct



############################################################################################
# configuration
############################################################################################
CONFIG_SERVER_IP             = '192.168.100.12'
CONFIG_SERVER_PORT           = 11234
CONFIG_RECV_CHUNK_SIZE       = 512
CONFIG_RECV_TIMEOUT_MS       = 1000
CONFIG_RECV2_TIMEOUT_MS      = 3000



############################################################################################
# alexa avs class
############################################################################################
class alexa_avs:

    def __init__(self):
        self.handle = None
        self.quits = False


    def connect(self, device_id):

        self.handle = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sockaddr = socket.getaddrinfo(CONFIG_SERVER_IP, CONFIG_SERVER_PORT)[0][-1]

        # connect to server
        try:
            self.handle.connect(sockaddr)
        except:
            self.handle.close()
            self.handle = None
            return False

        # send the device id
        val = struct.pack('I', device_id)
        self.handle.send(val)

        return True


    def disconnect(self):

        if self.handle is None:
            return

        self.handle.close()
        self.handle = None
        return


    def is_connected(self):

        if self.handle is None:
            return False

        return True


    def recv_and_play_response(self):

        # set receive timeout
        poller = uselect.poll()
        poller.register(self.handle, uselect.POLLIN)
        res = poller.poll(CONFIG_RECV_TIMEOUT_MS)
        if not res:
            return True

        # receive the size of audio segment
        val = self.handle.recv(4)
        if not val:
            print("Recv size ERROR")
            return False
        total_size = int(struct.unpack('i', val[:])[0])
        print(total_size)

        # receive the audio segment
        want_recv = CONFIG_RECV_CHUNK_SIZE
        total_recv = 0
        while total_recv < total_size:
            if total_size-total_recv < want_recv:
                want_recv = total_size-total_recv

            # set receive timeout
            poller.register(self.handle, uselect.POLLIN)
            res = poller.poll(CONFIG_RECV2_TIMEOUT_MS)
            if not res:
                return False

            # receive audio segment
            val = self.handle.recv(want_recv)
            if not val:
                print("Recv size ERROR2")
                return False
            total_recv += len(val)
            #print(total_recv)

        return True


    def send_request(self, file):
        # TODO
        return True


    def is_quit(self):
        return self.quits

    def quit(self):
        self.quits = True

    def get_server_ip(self):
        return CONFIG_SERVER_IP

    def get_server_port(self):
        return CONFIG_SERVER_PORT

