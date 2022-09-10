

import websocket
import _thread
import time
import rel

import socket
from zeroconf import ServiceBrowser, Zeroconf
from time import sleep

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
            print("Wibee CB @" + ipaddress)

        # print("Service %s added, IP address: %s" % (name, ipaddress))


def on_message(ws, message):
    try:
        print(message)
    except:
        print("Socket Exception")

def on_error(ws, error):
    print(error)

def on_close(ws, close_status_code, close_msg):
    print("### closed ###")

def on_open(ws):
    print("Opened connection")

if __name__ == "__main__":
    zeroconf = Zeroconf()
    listener = MyListener()
    browser = ServiceBrowser(zeroconf, "_wibee._tcp.local.", listener)

    print("Waiting for discovery")
    sleep(1)

    if ipaddress != "":
        print("Updating Wibee CB @" + ipaddress)
        url = 'ws://' + ipaddress + '/ws'

        websocket.enableTrace(False)

        ws = websocket.WebSocketApp(url,
                                    on_open=on_open,
                                    on_message=on_message,
                                    on_error=on_error,
                                    on_close=on_close)

        ws.run_forever(dispatcher=rel, skip_utf8_validation=False)  # Set dispatcher to automatic reconnection
        rel.signal(2, rel.abort)  # Keyboard Interrupt
        rel.dispatch()


    else:
        print("Cannot discover Wibee ")


