import socket
import time
import random


class Client:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def __init__(self, address):
        self.sock.connect((address, 8777))

    def SendTest(self):
        self.sock.send(bytes("hugh;2;1;100;100;200;200;", 'utf-8'))

    def SendMsg(self, msg):
        self.sock.send(bytes(msg, 'utf-8'))
        # time.sleep(5)
        time.sleep(2)



client = Client('localhost')
for i in range(10):
    msg = "Hugh;2;1;"
    for j in range(2):
        pre = random.randint(100, 300)
        post = random.randint(100, 300)
        msg += str(pre) + ";" + str(post) + ";"
    client.SendMsg(msg)

for i in range(10):
    msg = "Jeorg;1;1;"
    pre = random.randint(100, 300)
    post = random.randint(100, 300)
    msg += str(pre) + ";" + str(post) + ";"
    client.SendMsg(msg)
