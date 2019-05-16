# ESP32 Alexa Client


This MicroPython-based application simulates FT900 Alexa Client but uses ESP32 MCU instead of FT900 MCU.

An FT900 MCU or ESP32 MCU can connect to the RPI Alexa Gateway to access Alexa.


<img src="https://github.com/richmondu/FT900/blob/master/Alexa/Amazon%20Alexa%20Client/docs/images/system_diagram.jpg" width="623"/>

<img src="https://github.com/richmondu/FT900/blob/master/Alexa/Amazon%20Alexa%20Client/docs/images/system_diagram2.jpg" width="623"/>

<img src="https://github.com/richmondu/FT900/blob/master/Alexa/Amazon%20Alexa%20Client/docs/images/alexa_steps.jpg" width="623"/>

<img src="https://github.com/richmondu/FT900/blob/master/Alexa/Amazon%20Alexa%20Client/docs/images/alexa_audio.jpg" width="623"/>



Refer to the following links:

- [Alexa Gateway](https://github.com/richmondu/FT900/tree/master/Alexa/Amazon%20Alexa%20Gateway)

- [FT900 Alexa Client](https://github.com/richmondu/FT900/tree/master/Alexa/Amazon%20Alexa%20Client) - main documentation is here

- [FT900 Alexa Client Simulator](https://github.com/richmondu/FT900/tree/master/Alexa/Amazon%20Alexa%20Client%20Simulator)



## Creating a simple MicroPython application on ESP32

Below are instructions to run basic application

        1. pip install esptool

        2. Get esptool.py from https://github.com/espressif/esptool

        3. Get esp32-20190125-v1.10.bin from https://micropython.org/download#esp32
            [FYI: esp32-20190125-v1.10.bin has TCP RECV issue; replace with esp32-20190507-v1.10-326-g089c9b71d.bin]

        4. Run esptool.py to program 
            python esptool.py --port COM40 erase_flash
            python esptool.py --chip esp32 --port COM40 --baud 115200 write_flash -z 0x1000 esp32-20190507-v1.10-326-g089c9b71d.bin

        5. Create boot.py and main.py files
           Create boot.py with print("boot.py - Hello World!")
           Create main.py with print("main.py - Hello World!")

        5. Copy boot.py and main.py files to ESP32 using RSHELL.
            pip install rshell
            rshell
            connect serial COM40 115200
            cp boot.py /pyboard
            cp main.py /pyboard
            repl [or open teraterm COM40 115200]
            reset ESP32


## Building MicroPython port for ESP32


Note that the instructions specified in https://github.com/micropython/micropython/tree/master/ports/esp32 does not work as is.

Below are instructions to compile/build MicroPython for ESP32.

        1.
        2.
        3.

The resulting binary sizes is as follows:

        mpy-cross is the cross-compiler which is used to turn scripts into precompiled bytecode.
        LINK mpy-cross
        text    data     bss     dec     hex filename
        300085    2628     704  303417   4a139 mpy-cross

        LINK build/application.elf
        text    data     bss     dec     hex filename
        839176  233948   28180 1101304  10cdf8 build/application.elf

        Create build/firmware.bin
        bootloader     20624
        partitions      3072
        application  1073264
        total        1138800    

Libraries that will not be used should be removed to allocate more memory for PanL framework and applications.

This has been tested to be working using the current ESP32 Alexa demo. 
