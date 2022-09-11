
import requests
import sys
import os
import glob
import socket
from time import sleep

import os

calledViaPlatformIO = False

try :
    Import("env")   
    print("Called via PlatformIO")
    calledViaPlatformIO = True
except:
    print("Called via Terminal")




from zeroconf import ServiceBrowser, Zeroconf

ipaddress = ""

class MyListener:

    def remove_service(self, zeroconf, type, name):
        print("Service %s removed" % (name,))

    def add_service(self, zeroconf, type, name):
        info = zeroconf.get_service_info(type, name)

        global ipaddress
        ipaddress = ""

        for ipBytes in info.addresses:
            ipaddress = socket.inet_ntoa(ipBytes)
            print("wibee @" + ipaddress)

        # print("Service %s added, IP address: %s" % (name, ipaddress))

def ota_update(source, target, env):

    zeroconf = Zeroconf()
    listener = MyListener()
    browser = ServiceBrowser(zeroconf, "_wibee._tcp.local.", listener)

    # python ota_update.py "D:\W\P\FMS\30_SW\wibee_fw_release\wibee_esp_firmware_2.8.652.bin"

    VERSION_FILE = 'version'

    if len(sys.argv) > 1:
        if sys.argv[1] == "" or sys.argv[1] == "-Q":
            with open(VERSION_FILE) as FILE:
                VERSION_PATCH_NUMBER = FILE.readline()
                VERSION_PREFIX = VERSION_PATCH_NUMBER[0:VERSION_PATCH_NUMBER.rindex('.') + 1]
                VERSION_PATCH_NUMBER = int(VERSION_PATCH_NUMBER[VERSION_PATCH_NUMBER.rindex('.') + 1:])
                VERSION_PATCH_NUMBER = VERSION_PATCH_NUMBER - 1

                fileName = os.getcwd() + ".\\wibee_fw_release\\wibee_esp_firmware_" + VERSION_PREFIX + str(
                    VERSION_PATCH_NUMBER) + ".bin"
        else:
            fileName = sys.argv[1]
    else:
        list_of_files = glob.glob(
            '.\\wibee_fw_release\\*.bin')  # * means all if need specific format then *.csv

        fileName = max(list_of_files, key=os.path.getctime)

    print("OTA: " + fileName)


    print("Waiting for discovery")
    sleep(1)

    if ipaddress != "":
        print("Updating Wibee @" + ipaddress + " with " + str(VERSION_PATCH_NUMBER))
        url = 'http://'+ipaddress+'/update'

        files = {'file': ('update.bin', open(fileName, 'rb'), 'text/csv')}
        r = requests.post(url, files=files, verify=False)
        r.text
        print("Wibee Update Complete !!!")
    else:
        print("Cannot discover Wibee ")
        
        # url = 'http://10.0.0.43/update'


if calledViaPlatformIO:
    env.AddPreAction("upload", ota_update)
else:
    ota_update("","","")


