import datetime
import sqlite3
from sqlite3 import Error

#https://stackoverflow.com/questions/38076220/python-mysqldb-connection-in-a-class

class Database:
    def __init__(self, db):
        self._db = db
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    @property
    def Connection(self):
        self._connection
    
    @property
    def Cursor(self):
        self._cursor

    def Connect(self):
        self._connection = sqlite3.connect(self._db)
        self._curosr = self._connection.cursor()

    def Commit(self):
        self.Connection.commit()

    def Close(self, commit=True):
        if commit:
            self.Commit()
        self.Connection.close()
    
    def Execute(self, sql, params=None):
        self.Cursor.execute(sql, params or ())

    def FetchAll(self):
        return self.Cursor.fetchall()
    
    def FetchOne(self):
        return self.cursor.fetchone()
    
    def Query(self, sql, params=None):
        self.Cursor.execute(sql, params or ())
        return self.FetchAll()

    def InsertPlant(self, name, detectors, time):
        try:
            self.Connect()
            print("Connected to database")
            print("Inserting plant")

            FetchOnePlant()
        except Error as e:
            print(e)
        finally:
            self.Close()

    def FetchOnePlant(self):
        self.Execute('''SELECT count(name) FROM sqlite_master
            WHERE type='table' AND name='Plants';''')
        if self.FetchOne()[0] == 0:
            self.Execute('''CREATE TABLE Plants (
                                        name TEXT NOT NULL PRIMARY KEY,
                                        detectors INTEGER,
                                        setupDate timestamp
                                        );
                                        ''')

    def PlantExists(self, name):
        self.Execute("""SELECT count(name) FROM Plants WHERE name=?""", (name,))


def InsertPlant(db, name, detectors, time):
    conn = None
    try:
        conn = sqlite3.connect(db)
        c = conn.cursor()
        print("Connected to database")
        print("Inserting plant")

        # Check if the table exists build if it doesn't
        CheckForPlantTable(c)
        if CheckIfPlantExists(c, name):
            return

        # Insert a plant
        sql_insert_plant = """INSERT INTO Plants
                            (name, detectors, setupDate)
                            VALUES (?, ?, ?);"""

        data = (name, detectors, time)
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
        print("Error ", error)
    finally:
        if (conn):
            conn.close()


def CheckForPlantTable(c):
    c.execute('''SELECT count(name) FROM sqlite_master
            WHERE type='table' AND name='Plants';''')
    if c.fetchone()[0] == 0:
        sql_create_table = '''CREATE TABLE Plants (
                                        name TEXT NOT NULL PRIMARY KEY,
                                        detectors INTEGER,
                                        setupDate timestamp
                                        );
                                        '''
        c.execute(sql_create_table)


def SelectAllPlants(db):
    conn = None
    try:
        conn = sqlite3.connect(db)
        c = conn.cursor()
        print("Connected to database")
        print("Select all from Plants")

        CheckForPlantTable(c)

        sql_select_all = '''SELECT * FROM Plants;'''
        c.execute(sql_select_all)

        # print(list(c.fetchall()))

        return list(c.fetchall())

        c.close()
    except Error as e:
        print(e)
    finally:
        if conn:
            conn.close()


def CheckIfPlantExists(c, name):
    c.execute("""SELECT count(name) FROM Plants WHERE name=?""", (name,))
    if c.fetchone()[0] == 0:
        return False
    return True


def SelectAllMoistures(db):
    conn = None
    try:
        conn = sqlite3.connect(db)
        c = conn.cursor()
        print("Connected to database")
        print("Select all from Moisture")

        CheckForMoistureTable(c)

        sql_select_all = '''SELECT * FROM Moistures;'''
        c.execute(sql_select_all)

        # print(list(c.fetchall()))

        return list(c.fetchall())

        c.close()
    except Error as e:
        print(e)
    finally:
        if conn:
            conn.close()


def InsertMoisture(db, name, detectorId, time, isWatered, preMoistureLevel, postMoistureLevel):
    conn = None
    try:
        conn = sqlite3.connect(db)
        c = conn.cursor()
        print("Connected to database")
        print("Inserting Moisture")

        # Check if the table exists
        CheckForMoistureTable(c)

        # Insert a moisture level
        sql_insert_moisture = '''INSERT INTO Moistures(name, detectorId, time, wasWatered, preMoistureLevel, postMoistureLevel)
                VALUES (?, ?, ?, ?, ?, ?)'''
        data = (name, detectorId, time, isWatered,
                preMoistureLevel, postMoistureLevel)
        c.execute(sql_insert_moisture, data)
        conn.commit()
        print("Moisture level inserted", time)

        c.close()

    except Error as e:
        print("Error", e)
    finally:
        if conn:
            conn.close()


def CheckForMoistureTable(c):
    c.execute('''SELECT count(name) FROM sqlite_master 
            WHERE type='table' AND name='Moistures';''')
    if c.fetchone()[0] == 0:
        sql_create_table = '''CREATE TABLE Moistures (
                                    name TEXT NOT NULL,
                                    detectorId INTEGER NOT NULL,
                                    time timestamp NOT NULL,
                                    wasWatered INTEGER NOT NULL,
                                    preMoistureLevel INTEGER NOT NULL,
                                    postMoistureLevel INTEGER NOT NULL,                                    
                                    PRIMARY KEY("name", "detectorId", "time")
                                    );
                                    '''
        c.execute(sql_create_table)

def SaveEntryToFile(entry):
    f = open("readings.txt", "w+")
    f.write(entry + "\n")
    f.close()
