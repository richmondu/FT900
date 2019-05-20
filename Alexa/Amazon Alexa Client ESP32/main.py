#from alexa.avs_manager import avs_manager
from iot.mqtt_manager import mqtt_manager
import esp
import gc
import ntptime



def main():

    # set debug
    esp.osdebug(None)
    # activate garbage collector
    gc.collect()
    # set rtc time
    ntptime.settime()


    # run managers
    iot_manager = mqtt_manager()
    iot_manager.run()
    #alexa_manager = avs_manager()
    #alexa_manager.run()

    # trigger managers to quit
    #alexa_manager.quit()
    #iot_manager.quit()

    # wait for managers to quit
    #alexa_manager.wait()
    iot_manager.wait()
    return

if __name__ == '__main__':
    main()
