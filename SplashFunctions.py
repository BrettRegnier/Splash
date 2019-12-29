import datetime
import sqlite3
from sqlite3 import Error


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
