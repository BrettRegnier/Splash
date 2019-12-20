import socket
import threading
import os
import queue
import datetime
import sqlite3
from sqlite3 import Error


class Server:
    _connections = []
    _sock = None
    _queue = None
    _running = False

    def __init__(self):
        self._sock = socket.socket()
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
        dbThread = threading.Thread(target=self.DatabaseHandler)
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
                    print("responding")
                    client.send(bytes("responding...", "utf-8"))
                else:
                    print(str(addr[0]) + ":", str(addr[1]), "disconnected")
                    self._connections.remove(client)
                    client.close()
                    return
            except:
                print(str(addr[0]) + ":", str(addr[1]), "disconnected")
                self._connections.remove(client)
                client.close()
                return

    def DatabaseHandler(self):
        # TODO make this sql part
        while self._running:
            item = self._queue.get()
            if item is not None:
                name = item[0]
                detectors = int(item[1])
                isWatered = int(item[2])

                lLevels = []
                for x in range(detectors):
                    lLevels.append([])
                    lLevels[x].append(item[3 + x])  # premoisture
                    lLevels[x].append(item[3 * detectors + x])  # postmoisture
                for xid in range(detectors):
                    premoist = int(item[3 + xid])
                    postmoist = int(item[3 * detectors + xid])
                    self.InsertMoisture(name, xid, isWatered, premoist,
                                        postmoist, datetime.datetime.now())

    def Console(self):
        while self._running:
            con = input()
            if con == "exit":
                self._running = False

    def InsertMoisture(self, name, detectorId, isWatered, preMoistureLevel,
                       postMoistureLevel, time):
        conn = None
        try:
            conn = sqlite3.connect('Splash.db')
            c = conn.cursor()
            print("Connected to database")

            # Check if the table exists
            c.execute('''SELECT count(name) FROM sqlite_master 
                WHERE type='table' AND name='Moistures';''')
            if c.fetchone()[0] == 0:
                sql_create_table = '''CREATE TABLE Moistures (
                                        name TEXT NOT NULL,
                                        detectorId INTEGER NOT NULL,
                                        wasWatered INTEGER NOT NULL,
                                        preMoistureLevel INTEGER NOT NULL,
                                        postMoistureLevel INTEGER NOT NULL,
                                        time timestamp,                                        
                                        PRIMARY KEY("name", "detectorId")
                                        );
                                        '''
                c.execute(sql_create_table)

            # TODO Remove this later, its only here so I can get moving forward faster.
            # Check if the table exists
            c.execute('''SELECT count(name) FROM sqlite_master 
                WHERE type='table' AND name='Plants';''')
            if c.fetchone()[0] == 0:
                sql_create_table = '''CREATE TABLE Plants (
                                            name TEXT NOT NULL PRIMARY KEY,
                                            joinDate timestamp
                                            );
                                            '''
                c.execute(sql_create_table)
            # Check if specific plant exists in plants table.
            sql_check_plant = '''SELECT name FROM Plants WHERE name=?'''
            c.execute(sql_check_plant, (name, ))
            if (len(c.fetchall()) == 0):
                self.InsertPlant(name, time)

            # Insert a moisture level
            sql_insert_moisture = '''INSERT INTO 'Moistures'
                ('name', 'detectorId', 'wasWatered', 'preMoistureLevel', 'postMoistureLevel', 'time')
                VALUES (?, ?, ?, ?, ?, ?, ?)'''
            data = (name, detectorId, isWatered, preMoistureLevel,
                    postMoistureLevel, time)
            c.execute(sql_insert_moisture, data)
            conn.commit()
            print("Moisture level inserted", time)

            c.close()

        except Error as e:
            print(e)
        finally:
            if conn:
                conn.close()

    def InsertPlant(self, name, joiningDate):
        try:
            conn = sqlite3.connect('Splash.db')
            c = conn.cursor()
            print("Connected to SQLite")

            # Check if the table exists
            c.execute('''SELECT count(name) FROM sqlite_master 
                WHERE type='table' AND name='Plants';''')
            if c.fetchone()[0] == 0:
                sql_create_table = '''CREATE TABLE Plants (
                                            name TEXT NOT NULL PRIMARY KEY,
                                            joinDate timestamp
                                            );
                                            '''
                c.execute(sql_create_table)

            # Insert a plant
            sql_insert_plant = """INSERT INTO 'Plants'
                            ('name', 'joinDate') 
                            VALUES (?, ?);"""

            data = (name, joiningDate)
            c.execute(sql_insert_plant, data)
            conn.commit()
            print("Plant added successfully \n")

            # # get developer detail
            # sql_select_plant = """SELECT name, joinDate from Plants"""
            # c.execute(sql_select_plant)
            # records = c.fetchall()

            # for row in records:
            #     plant = row[0]
            #     joinDate = row[1]
            #     print(plant, "added on", joinDate)

            c.close()

        except sqlite3.Error as error:
            print("Error while working with SQLite", error)
        finally:
            if (conn):
                conn.close()
                print("sqlite connection is closed")


if __name__ == "__main__":
    try:
        server = Server()
        server.Run()
    except (KeyboardInterrupt, SystemExit):
        os._exit(0)
