import sqlite3
import datetime


def AddPlant(name, joiningDate):
    try:
        conn = sqlite3.connect('Splash.db')
        c = conn.cursor()
        print("Connected to SQLite")

        # Check if the table exists
        c.execute('''SELECT count(name) FROM sqlite_master WHERE type='table' AND name='Plants';''')
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


AddPlant('Janeous', datetime.datetime.now())
