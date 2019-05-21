import esp
import gc
import ntptime
import utime
import micropython
import machine
import ubinascii
import sys

from alexa.avs_manager import avs_manager
from iot.mqtt_manager import mqtt_manager



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

def get_string(localtime):
    (year, month, day, hour, minute, second, millis, _tzinfo) = localtime
    return "%d-%02d-%02d_%02d:%02d:%02d.%03d" % (year, month, day, hour, minute, second, millis)

def get_system_info():
    print("\r\n\r\n")
    print("=======================================================================")
    print("SYSTEM INFO")
    print("=======================================================================")
    print('datetime   :{}'.format(get_string(utime.localtime())))
    print('platform   :{}'.format(sys.platform))
    print('version    :{}'.format(sys.version))
    print('system     :{}-{}'.format(sys.implementation[0], sys.implementation[1]))
    print('machine_id :0x{}'.format(ubinascii.hexlify(machine.unique_id()).decode().upper()))
    print('machine_fq :{}'.format(machine.freq()))
    print('maxsize    :{}'.format(sys.maxsize))
    print('modules    :{}'.format(sys.modules))
    micropython.mem_info()
    micropython.stack_use()
    print("=======================================================================\r\n")

def main():

    # set debug
    esp.osdebug(None)
    # activate garbage collector
    gc.collect()

    # set rtc time
    try:
        ntptime.settime()
    except:
        pass
    # display system info
    get_system_info()


    #test_alexa_demo()
    test_iot_demo()

    return

if __name__ == '__main__':
    main()
