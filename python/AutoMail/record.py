#!/usr/bin/python

from optparse import OptionParser
import sqlite3
import os

class Record:
    def __init__(self, filename="record.db"):
        filename = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)
        self._connect = sqlite3.connect(filename)
        self._cursor = self._connect.cursor()

    def execute(self, sql):
        self._cursor.execute(sql)
        self._connect.commit()
        return self._cursor.fetchall()

    def __del__(self):
        self._connect.commit()
        self._connect.close()

    def create_table_if_null(self, tbl):
        sql = """
        CREATE TABLE if not exists %s 
        (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        new TEXT DEFAULT 'yes',
        module TEXT,
        testcase TEXT,
        date TEXT,
        elapse TEXT,
        description TEXT,
        result TEXT,
        attach_name TEXT,
        attach_content BLOB
        )
        """ % tbl
        self._cursor.execute(sql)

    def get_table_header(self, tbl):
        sql = "PRAGMA table_info(%s)" % tbl
        self._cursor.execute(sql)
        result = self._cursor.fetchall()
        sec = [ r[1] for r in result ]
        return sec

    def get_tables(self):
        sql = "SELECT name FROM sqlite_master WHERE type='table'"
        self._cursor.execute(sql)
        results = self._cursor.fetchall()
        tbls = [ r[0] for r in results if r[0] != "sqlite_sequence" ]
        return tbls

    def insert_record(self, tbl, module, testcase, elapse, desc, result, attach):
        attach_name = None
        attach_content = None
        if os.path.isfile(attach):
            with open(attach, "rb") as f:
                attach_content = sqlite3.Binary(f.read())
                attach_name = os.path.basename(attach)

        sql = """
        INSERT INTO
        %s (module, testcase, elapse, description, date, result, attach_name, attach_content)
        VALUES ('%s', '%s', '%s', '%s', datetime('now', 'localtime'), '%s', ?, ?)
        """ % (tbl, module, testcase, elapse, desc, result)

        self._cursor.execute(sql, (attach_name, attach_content))
        self._connect.commit()

        sql = "SELECT id FROM %s ORDER BY id DESC limit 1" % tbl
        self._cursor.execute(sql)
        return self._cursor.fetchone()[0]

    def delete_record(self, tbl, id):
        sql = "DELETE FROM %s WHERE id=%s" % (tbl, str(id))
        self._cursor.execute(sql)
        self._connect.commit()

    def query_record(self, tbl, id, fmt):
        fmt = ",".join(fmt)
        sql = "SELECT %s FROM %s WHERE id=%s" % (fmt, tbl, str(id))
        self._cursor.execute(sql)
        return self._cursor.fetchone()

    def query_new_records(self, tbl, fmt, update=1):
        fmt = ",".join(fmt)
        sql = "SELECT %s FROM %s WHERE new='yes'" % (fmt, tbl)
        self._cursor.execute(sql)
        new = self._cursor.fetchall()
        if update:
            sql = "UPDATE %s SET new='no' WHERE new='yes'" % tbl
            self._cursor.execute(sql)
            self._connect.commit()
        return new

    def dump(self):
        print "---DUMP---"

        sql = "SELECT name FROM sqlite_master WHERE type='table'"
        self._cursor.execute(sql)
        tables = self._cursor.fetchall()
        for t in tables:
            name = t[0]
            if name == "sqlite_sequence":
                continue

            print "TABLE %s" % name

            sql = "SELECT * FROM %s" % name
            self._cursor.execute(sql)
            results = self._cursor.fetchall()
            for result in results:
                print result

        print "---DONE---"

def main():
    parser = OptionParser()
    parser.add_option("--team", dest="team",
            help="which team")
    parser.add_option("--module", dest="module",
            help="which module")
    parser.add_option("--testcase", dest="testcase",
            help="which testcase")
    parser.add_option("--elapse", dest="elapse",
            help="how long the testcase cost", default="0min")
    parser.add_option("--description", dest="description",
            help="describe the test result", default="None")
    parser.add_option("--result", dest="result",
            help="result of the testcase")
    parser.add_option("--attachment", dest="attachment",
            help="attachment of the testcase", default="")
    parser.add_option("-d", "--dump", dest="dump",
            help="dump all records", action="store_true", default=False)
    (options, args) = parser.parse_args()

    team = options.team
    module = options.module
    testcase = options.testcase
    elapse = options.elapse
    desc = options.description
    result = options.result
    attach = options.attachment
    dump = options.dump

    if (not (team and module and testcase and result)) and (not dump):
        parser.print_help()
        return

    db = Record()

    if dump:
        db.dump()
        return

    db.create_table_if_null(team)

    db.insert_record(team, module, testcase, elapse, desc, result, attach)

    del db

if __name__ == "__main__":
    main()
