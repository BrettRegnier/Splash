import socket
import threading
import os
import queue
import datetime
import sqlite3
from sqlite3 import Error

import sys


import Database


class Server:
    _connections = []
    _sock = None
    _queue = None
    _running = False
    _db = "Splash.db"

    def __init__(self):
        self._sock = socket.socket()
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.bind(("0.0.0.0", 8777))
        self._sock.listen(1)

        self._queue = queue.Queue()
        self._running = True

    def Run(self):
        # Run thread for checking if there is an incoming connection
        conThread = threading.Thread(target=self.ConnectionHandler)
        conThread.daemon = True
        conThread.start()

        # Run thread for database
        dbThread = threading.Thread(target=self.MessageHandler)
        dbThread.daemon = True
        dbThread.start()

        # Have the master thread monitor the console.
        self.Console()

    def ConnectionHandler(self):
        while self._running:
            client, addr = self._sock.accept()
            cThread = threading.Thread(target=self.ClientHandler,
                                       args=(client, addr))
            cThread.daemon = True
            cThread.start()
            self._connections.append(client)
            print(str(addr[0]) + ":", str(addr[1]), "connected")

    def ClientHandler(self, client, addr):
        while self._running:
            try:
                msg = client.recv(1024)
                print(msg)
                if msg:
                    msg = msg.decode("utf-8")

                    data = []
                    s = len(msg)
                    idx = 0
                    end = 0

                    while (idx < s):
                        end = idx
                        while (msg[end] != ";"):
                            end += 1
                        data.append(msg[idx:end])
                        idx = end + 1

                    # put data in the queue and move on.
                    self._queue.put(data)

                    # # TODO send back server config settings.
                    # TODO only send back when the client is done sending,
                    # so maybe it should have an end bit..? thats when the send
                    # should be sent.
                    # print("responding")
                    client.send(bytes("r", "utf-8"))
                else:
                    print(str(addr[0]) + ":", str(addr[1]), "disconnected")
                    self._connections.remove(client)
                    client.close()
                    return
            except:
                print("Unexpected error:", sys.exc_info()[0])
                print(str(addr[0]) + ":", str(addr[1]), "disconnected")
                self._connections.remove(client)
                client.close()
                return

    def MessageHandler(self):
        while self._running:
            item = self._queue.get()
            if item is not None:
                m_type = int(item[0])
                m_name = item[1]

                current_time = datetime.datetime.now()

                # plant message
                if (m_type == 0):
                    m_to_water = bool(item[2])
                    m_detectors = int(item[3])
                    
                    m_levels = []
                    for x in range(m_detectors):
                        m_levels.append([])
                        m_levels[x].append(item[4 + x])  # premoisture
                        m_levels[x].append(item[4 * m_detectors + x])  # postmoisture
                    
                    print(m_type, m_name, current_time, m_to_water, m_detectors, m_levels)
                    # SplashFunctions.SaveEntryToFile(s)

                    # SplashFunctions.InsertPlant(self._db, name, detectors, time)
                    # for xid in range(detectors):
                    #     premoist = int(item[3 + xid])
                    #     postmoist = int(item[3 + detectors + xid])
                    #     SplashFunctions.InsertMoisture(
                    #         self._db, name, xid, time, isWatered, premoist, postmoist)
                
                # reservoir message
                elif (m_type == 1):
                    m_status = bool(item[2])
                    print(m_type, m_name, current_time, m_status)

    def Console(self):
        while self._running:
            con = input()
            if con == "exit":
                self._running = False


if __name__ == "__main__":
    try:
        server = Server()
        server.Run()

    except (KeyboardInterrupt, SystemExit):
        os._exit(0)
