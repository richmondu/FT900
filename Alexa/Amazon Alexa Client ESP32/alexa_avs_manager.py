from alexa_avs import alexa_avs
import _thread
import time



############################################################################################
# configuration
############################################################################################
CONFIG_DEVICE_ID               = 1
CONFIG_FILENAME_REQUEST_TIME   = "audio/REQUEST_what_time_is_it.raw"
CONFIG_FILENAME_REQUEST_PERSON = "audio/REQUEST_who_is.raw"
CONFIG_FILENAME_REQUEST_MUSIC  = "audio/REQUEST_play_music.raw"
CONFIG_FILENAME_REQUEST_NEWS   = "audio/REQUEST_play_live_news.raw"
CONFIG_FILENAME_REQUEST_BOOK   = "audio/REQUEST_play_audio_book.raw"
CONFIG_FILENAME_REQUEST_ALARM  = "audio/REQUEST_set_alarm.raw"
CONFIG_FILENAME_REQUEST_STOP   = "audio/REQUEST_stop.raw"
CONFIG_FILENAME_REQUEST_YES    = "audio/REQUEST_yes.raw"



############################################################################################
# alexa avs manager class
############################################################################################
class alexa_avs_manager():

    def __init__(self):
        self.quits = False
        self.exiting = False


    ############################################################################################
    # usage
    ############################################################################################
    def usage(self):

        print("\r\n===============================================================")
        print("[MAIN] Usage:")
        print("[MAIN]   Press 'q' key to quit...")
        print("[MAIN]   Press 't' key to ask Alexa what time is it...")
        print("[MAIN]   Press 'p' key to ask Alexa who is...")
        print("[MAIN]   Press 'm' key to ask Alexa to play music...")
        print("[MAIN]   Press 'n' key to ask Alexa to play live news...")
        print("[MAIN]   Press 'b' key to ask Alexa to play audio book...")
        print("[MAIN]   Press 'a' key to ask Alexa to set alarm...")
        print("[MAIN]   Press 's' key to tell Alexa to stop...")
        print("[MAIN]   Press 'y' key to tell Alexa yes...")
        print("===============================================================\r\n")


    ############################################################################################
    # thread_fxn_streamer
    ############################################################################################
    def thread_fxn_streamer(self, avs_handle, device_id):

        while not avs_handle.is_quit():

            # connect to server
            connect_retry = 0
            while not avs_handle.is_quit():
                print("\r\n[STREAMER] Connecting to Alexa provider... {}:{} [ID:{}] [{}]".format(avs_handle.get_server_ip(), avs_handle.get_server_port(), device_id, connect_retry))
                if not avs_handle.connect(device_id):
                    connect_retry += 1
                    time.sleep(3)
                    continue
                print("\r\n[STREAMER] Connected successfully!")
                self.usage()
                break

            # stream from server
            while not avs_handle.is_quit():
                if not avs_handle.recv_and_play_response():
                    break

            # disconnect from server
            avs_handle.disconnect()
            print("\r\n[STREAMER] Disconnected from Alexa provider...")

        print("\r\n[STREAMER] exits...")
        _thread.exit()
        return


    ############################################################################################
    # thread_fxn_commander
    ############################################################################################
    def thread_fxn_commander(self, avs_handle, file_request):

        # exit if not connected
        if not avs_handle.is_connected():
            print("[COMMANDER] Not connected...")
            _thread.exit()
            return

        # send request
        if not avs_handle.is_quit():
            print("[COMMANDER] Sending Alexa query [{}]...".format(file_request))
            avs_handle.send_request(file_request)
            print("OK")

        _thread.exit()
        return


    ############################################################################################
    # thread_fxn_manager
    ############################################################################################
    def thread_fxn_manager(self):
        print("\r\n===============================================================")
        print("ESP32 Alexa Demo")
        print("===============================================================\r\n")

        avs_handle = alexa_avs()

        # start the streamer thread
        _thread.start_new_thread(self.thread_fxn_streamer, (avs_handle, CONFIG_DEVICE_ID, ))

        # wait for user input
        while not self.quits:
            key = input("")
            if key == 'q':
                avs_handle.quit()
                break
            elif key == 'h':
                usage()
            elif key=='t':
                _thread.start_new_thread(self.thread_fxn_commander, (avs_handle, CONFIG_FILENAME_REQUEST_TIME, ))
            elif key=='p':
                _thread.start_new_thread(self.thread_fxn_commander, (avs_handle, CONFIG_FILENAME_REQUEST_PERSON, ))
            elif key=='m':
                _thread.start_new_thread(self.thread_fxn_commander, (avs_handle, CONFIG_FILENAME_REQUEST_MUSIC, ))
            elif key=='n':
                _thread.start_new_thread(self.thread_fxn_commander, (avs_handle, CONFIG_FILENAME_REQUEST_NEWS, ))
            elif key=='b':
                _thread.start_new_thread(self.thread_fxn_commander, (avs_handle, CONFIG_FILENAME_REQUEST_BOOK, ))
            elif key=='a':
                _thread.start_new_thread(self.thread_fxn_commander, (avs_handle, CONFIG_FILENAME_REQUEST_ALARM, ))
            elif key=='s':
                _thread.start_new_thread(self.thread_fxn_commander, (avs_handle, CONFIG_FILENAME_REQUEST_STOP, ))
            elif key=='y':
                _thread.start_new_thread(self.thread_fxn_commander, (avs_handle, CONFIG_FILENAME_REQUEST_YES, ))

        if not avs_handle.is_quit():
            avs_handle.quit()

        time.sleep(1)
        print("\r\n===============================================================")
        print("ESP32 Alexa Demo exits...")
        print("===============================================================\r\n\r\n")
        self.exiting = True
        _thread.exit()
        return


    def run(self):
        _thread.start_new_thread(self.thread_fxn_manager, ())
        return

    def wait(self):
        while not self.exiting:
            pass
        return

    def quit(self):
        self.quits = True

