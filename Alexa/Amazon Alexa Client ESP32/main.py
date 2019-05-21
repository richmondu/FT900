from alexa.avs_manager import avs_manager
from iot.mqtt_manager import mqtt_manager
import esp
import gc
import ntptime
import utime



def test_alexa_demo():
    alexa_manager = avs_manager()
    alexa_manager.run()
    #alexa_manager.quit()
    alexa_manager.wait()

def test_iot_demo():
    iot_manager = mqtt_manager()
    iot_manager.run()
    #iot_manager.quit()
    iot_manager.wait()

def localtime_to_string(localtime) :
    (year, month, day, hour, minute, second, millis, _tzinfo) = localtime
    return "%d-%02d-%02d_%02d:%02d:%02d.%03d" % (year, month, day, hour, minute, second, millis)

def main():

    # set debug
    esp.osdebug(None)
    # activate garbage collector
    gc.collect()
    # set rtc time
    try:
        ntptime.settime()
        # print current time
        print("\r\n\r\n\r\n")
        print(localtime_to_string(utime.localtime()))
    except:
        pass


    #test_alexa_demo()
    test_iot_demo()

    return

if __name__ == '__main__':
    main()
