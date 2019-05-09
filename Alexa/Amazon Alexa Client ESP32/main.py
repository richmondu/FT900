from alexa.avs_manager import avs_manager



def main():

    # run alexa manager
    alexa_manager = avs_manager()
    alexa_manager.run()

    # trigger alexa manager to quit
    #alexa_manager.quit()

    # wait for alexa manager to finish
    alexa_manager.wait()
    return


if __name__ == '__main__':
    main()
