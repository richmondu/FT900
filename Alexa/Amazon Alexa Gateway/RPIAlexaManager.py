import os
import sys
import threading
import argparse
import time



conf_alexa_dir = "/home/pi/Desktop/alexa"
conf_debug_level = "INFO"
num_accounts = 1



def create_configuration_file(account_no):
    global conf_alexa_dir
    
    # construct the configuration file path
    if account_no == 1:
        config = "AlexaClientSDKConfig.json"
    else:
        config = "AlexaClientSDKConfig" + str(account_no) + ".json"
    path = conf_alexa_dir + "/build/Integration/" + config
    
    # check if path already exist
    if os.path.exists(path):
        print("{} already exists".format(path))
        return 1

    # check that configuration file for account 1 exist
    if account_no == 1:
        print("{} does NOT exist".format(path))
        return 0;
    
    # set folder permission
    path_src = conf_alexa_dir + "/build/Integration/"
    os.system("sudo chmod 777 " + path_src)
    time.sleep(1)

    # check file path
    config_src = "AlexaClientSDKConfig.json"    
    path_src += config_src
    if os.path.exists(path_src) == False:
        print("{} does NOT exist yet".format(path))
        return 0
    
    # open/create the files
    f_src = open(path_src, "r")
    f = open(path, "w")

    # read src line by line
    # and write file line by line
    line = f_src.readline()
    while (line):
        
        # replace instance of /db/ to /dbX/
        string = "/db/"
        offset = line.find(string)
        if offset >= 0:
            newLine = line[0:offset]
            newLine += "/db" + str(account_no) + "/"
            newLine += line[offset+len(string):]
            f.write(newLine)
        else:
            string = "\"deviceSerialNumber\":"
            offset = line.find(string)
            if offset >= 0:
                newLine = line[0:offset+len(string)]
                newLine += "\"" + str(account_no) + "\","
                f.write(newLine) 
            else:
                f.write(line)   
        
        # read the next line
        line = f_src.readline()
    
    # close the files
    f.close()
    f_src.close()
    
    return 1


def create_database_dir(account_no):
    global conf_alexa_dir
    
    # construct the database directory path
    path = conf_alexa_dir + "/db"
    if account_no > 1:
        path += str(account_no)
    
    # check if path already exist
    if os.path.exists(path):
        print("{} already exists".format(path))
        return 1
    
    # create the database input directory
    try:
        os.mkdir(path, 0o777)
    except OSError:
        print("Creation of the directory {} failed".format(path))
        return 0
    else:
        print("Successfully created the directory {}".format(path))
        
    return 1
        
    
def thread_func(account_no):
    global conf_alexa_dir
    global conf_debug_level
    global num_accounts
    
    # construct the command parameters
    command_term = "lxterminal"
    command_app = conf_alexa_dir + "/build/SampleApp/src/SampleApp"
    if account_no == 1:
        command_config = "AlexaClientSDKConfig.json"
    else:
        command_config = "AlexaClientSDKConfig" + str(account_no) + ".json"
    command_configuration = conf_alexa_dir + "/build/Integration/" + command_config
    command_third_party = conf_alexa_dir + "/third-party/alexa-rpi/models"
    command_debug_level = conf_debug_level
    command_account_no = account_no

    # combine the command parameters into a string
    if num_accounts == 1:
        command = command_app + " " + command_configuration + " " + command_third_party + " " + command_debug_level + " " + str(command_account_no)
    else:
        command = command_term + " -e " + command_app + " " + command_configuration + " " + command_third_party + " " + command_debug_level + " " + str(command_account_no)        
    #print(command)
    
    # execute the command string
    os.system(command)
    return


def main(args, argc):
    global num_accounts

    # parse the input
    num_accounts = int(args.numaccounts)
    
    # run the threads
    threads = []
    for i in range(1, num_accounts+1):
        
        print("\r\nAccount {}".format(i))

        # create configuration file if does not exist
        # will create configuration file AlexaClientSDKConfigX.json
        # in the same folder as AlexaClientSDKConfig.json
        if create_configuration_file(i) == 0:
            print("failed!")
            continue
        
        # create database file if does not exist
        # will create dbX folder on the same folder as db folder
        if create_database_dir(i) == 0:
            print("failed!")
            continue
        
        # create thread for the account number
        t = threading.Thread(target=thread_func, args=(i,))
        t.start()
        threads.append(t)
    
    # wait for the threads to finish
    for t in threads:
        t.join()
        
    return


def parse_arguments(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument('--numaccounts', required=True, default=1, help='number of accounts to run')
    return parser.parse_args(argv)


if __name__ == "__main__":
    main(parse_arguments(sys.argv[1:]), len(sys.argv)-1)
    
#os.system("lxterminal -e /home/pi/Desktop/alexa/build/SampleApp/src/SampleApp /home/pi/Desktop/alexa/build/Integration/AlexaClientSDKConfig3.json /home/pi/Desktop/alexa/third-party/alexa-rpi/models INFO 3")

