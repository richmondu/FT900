# ESP32 Alexa Client


This MicroPython-based application simulates FT900 Alexa Client but uses <b>ESP32 MCU</b> instead of FT900 MCU.

An FT900 MCU or ESP32 MCU can connect to the RPI Alexa Gateway to access Alexa.


<img src="https://github.com/richmondu/FT900/blob/master/Alexa/Amazon%20Alexa%20Client/docs/images/system_diagram.jpg" width="623"/>

<img src="https://github.com/richmondu/FT900/blob/master/Alexa/Amazon%20Alexa%20Client/docs/images/system_diagram2.jpg" width="623"/>

<img src="https://github.com/richmondu/FT900/blob/master/Alexa/Amazon%20Alexa%20Client/docs/images/alexa_steps.jpg" width="623"/>

<img src="https://github.com/richmondu/FT900/blob/master/Alexa/Amazon%20Alexa%20Client/docs/images/alexa_audio.jpg" width="623"/>



Refer to the following links:

- [Alexa Gateway](https://github.com/richmondu/FT900/tree/master/Alexa/Amazon%20Alexa%20Gateway)

- [FT900 Alexa Client](https://github.com/richmondu/FT900/tree/master/Alexa/Amazon%20Alexa%20Client) - main documentation is here

- [FT900 Alexa Client Simulator](https://github.com/richmondu/FT900/tree/master/Alexa/Amazon%20Alexa%20Client%20Simulator)



# MicroPython on ESP32

ESP32 MCU is a new platform to be used for 2nd-generation PanL displays.
The first generation PanL displays uses the in-house FT900 MCU.

ESP32 MCU supports C and MicroPython programming languages.
 
 

# Setting up MicroPython on ESP32

Below are instructions to setup ESP32 to use MicroPython

        1. pip install esptool

        2. Get esptool.py from https://github.com/espressif/esptool

        3. Get esp32-20190125-v1.10.bin from https://micropython.org/download#esp32
            [FYI: esp32-20190125-v1.10.bin has TCP RECV issue; replace with esp32-20190507-v1.10-326-g089c9b71d.bin]

        4. Run esptool.py to program 
            python esptool.py --port COM40 erase_flash
            python esptool.py --chip esp32 --port COM40 --baud 115200 write_flash -z 0x1000 esp32-20190507-v1.10-326-g089c9b71d.bin



# Creating a simple MicroPython application on ESP32

Below are instructions to run a simple Hello World application

        1. Create boot.py and main.py files
           Create boot.py with print("boot.py - Hello World!")
           Create main.py with print("main.py - Hello World!")

        2. Copy boot.py and main.py files to ESP32 using RSHELL.
            pip install rshell
            rshell
            connect serial COM40 115200
            cp boot.py /pyboard
            cp main.py /pyboard
            repl [or open teraterm COM40 115200]
            reset ESP32



# Running custom MicroPython applications on ESP32

Below are demo applications I have created to experiment and learn MicroPython on ESP32.

### ESP32 IoT demo

This demonstrates secure IoT connectivity with AWS IoT, GCP IoT and Azure IoT cloud platforms using MQTT over TLS.

        1. Copy iot folder to ESP32 /pyboard/iot
        2. Enable test_iot_demo() in main.py
        
A similar application for FT900 MCU exists in [here](https://github.com/richmondu/FT900/tree/master/IoT/ft90x_iot_aws_gcp_azure).

<img src="https://github.com/richmondu/FT900/blob/master/Alexa/Amazon%20Alexa%20Client/docs/images/esp32_usage_iot_demo.jpg" width="623"/>


### ESP32 Alexa demo

This demonstrates accessing Alexa Voice Services by connecting to an Alexa ‘edge’ gateway that enables multiple connected FT900/ESP32 clients to simultaneously access Alexa.

        1. Copy alexa folder to ESP32 /pyboard/alexa
        2. Enable test_alexa_demo() in main.py
        
A similar application for FT900 MCU exists in [here](https://github.com/richmondu/FT900/tree/master/Alexa/Amazon%20Alexa%20Client).

<img src="https://github.com/richmondu/FT900/blob/master/Alexa/Amazon%20Alexa%20Client/docs/images/esp32_usage_alexa_demo.jpg" width="623"/>



# Building MicroPython port for ESP32

MicroPython port for ESP32 should be customized to allocate more memory for PanL fraemwork and application.

Instructions to compile/build MicroPython port for ESP32 is specified in https://github.com/micropython/micropython/tree/master/ports/esp32 but it does not work as is.

To fill-in the gaps, below are complementary details to make the process straight-forward.

        1. Checkout specific version of ESP IoT Development Framework (ESP-IDF)
           git clone https://github.com/espressif/esp-idf.git
           git checkout 5c88c5996dbde6208e3bec05abc21ff6cd822d26
           git submodule update --init --recursive
        
        2. Checkout MicroPython
           git clone https://github.com/micropython/micropython
        
        3. Compile MicroPython cross-compiler
           cd micropython
           make -C mpy-cross PYTHON=python2
           
           Expected result:
           GEN build/genhdr/mpversion.h
           GEN build/genhdr/qstrdefs.collected.h
           QSTR not updated
           CC main.c
           LINK mpy-cross
              text    data     bss     dec     hex filename
            300085    2628     704  303417   4a139 mpy-cross
           make: Leaving directory '/home/richmond/esp/micropython/mpy-cross'
                      
        4. Create micropython/ports/esp32/GNUmakefile
           ESPIDF = $(HOME)/esp/esp-idf
           #ESPIDF="F:/msys32/home/richmond/esp/esp-idf"
           PORT = COM40
           FLASH_MODE = dio
           FLASH_SIZE = 4MB
           CROSS_COMPILE = xtensa-esp32-elf-
           SDKCONFIG = boards/sdkconfig
           include Makefile     
           
        5. Modify micropython/ports/esp32/Makefile
           MICROPY_PY_BTREE = 0
           #MICROPY_PY_BTREE = 1
           ESPCOMP_KCONFIGS = $(shell find "F:/msys32/home/richmond/esp/esp-idf/components" -name Kconfig)
           ESPCOMP_KCONFIGS_PROJBUILD = $(shell find "F:/msys32/home/richmond/esp/esp-idf/components" -name Kconfig.projbuild)         
           #ESPCOMP_KCONFIGS = $(shell find $(ESPCOMP) -name Kconfig)
           #ESPCOMP_KCONFIGS_PROJBUILD = $(shell find $(ESPCOMP) -name Kconfig.projbuild)
           
        6. Update files in micropython/ports/esp32/modules/ 
           dht.py
           ds18x20.py
           ntptime.py
           onewire.py
           upip.py
           upip_utarfile,py
           upysh.py
           urequests.py
           webrepl.py
           webrepl_setup.py
           websocket_hepler.py
           umqtt/simple.py
           umqtt/robust.py

        7. Update micropython/py/objmodule.c
           #if 0//MICROPY_PY_BTREE
           #endif

        8. Compile MicroPython port for ESP32
           cd ports/esp32
           make PYTHON=python2
           
           Expected result:
           LINK build/application.elf
           text    data     bss     dec     hex filename
           839176  233948   28180 1101304  10cdf8 build/application.elf
           Create build/firmware.bin
           bootloader     20624
           partitions      3072
           application  1073264
           total        1138800    

Libraries that will not be used should be removed to allocate more memory for PanL usage.

This customized MicroPython build has been tested to be working using the current ESP32 Alexa demo. 
