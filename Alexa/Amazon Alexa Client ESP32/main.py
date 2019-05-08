from alexa_avs_manager import alexa_avs_manager



def main():

    # run alexa manager
    alexa_manager = alexa_avs_manager()
    alexa_manager.run()

    # trigger alexa manager to quit
    #alexa_manager.quit()

    # wait for alexa manager to finish
    alexa_manager.wait()
    return


if __name__ == '__main__':
    main()
