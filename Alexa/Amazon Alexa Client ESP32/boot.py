print('[BOOT] booting up')

def do_connect():
    import network
    import machine
    
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        print('[BOOT] connecting to network...')
        wlan.connect('Virus-infected Wifi', 'MahirapTandaan')
        while not wlan.isconnected():
            pass
    print('[BOOT] network config:', wlan.ifconfig())


do_connect()
print('[BOOT] connected to WIFI\r\n\r\n')



