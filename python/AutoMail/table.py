#!/usr/bin/python

from pyh import *

class HtmlTable:

    def __init__(self, name, heading, percent=None):
        self.page = PyH("HtmlTable")
        self.page << h3(name, cl="center")
        self.tbl = self.page << table(border="1", width="100%", style="word-break:break-all;word-wrap:break-all;")
        head = None

        for i,h in enumerate(heading):
            if len(percent) == len(heading):
                t = th(h, width="%d%%" % percent[i])
            else:
                t = th(h)

            if head: head = head + t
            else: head = t

        self.head = head
        self.tbl << self.head
        self.content = {}

    def __store(self, content, result):
        if not result:
            return {}

        try:
            key = result[0].encode("utf-8")
        except:
            key = repr(result[0])

        if not content.has_key(key):
            content[key] = {}

        content[key] = self.__store(content[key], result[1:])
        return content

    def insert_result(self, result):
        if len(result) != len(self.head):
            print "Invalid input"
            return

        self.__store(self.content, result)

    def __sum(self, d):
        if len(d) == 0:
            return 1

        s = 0
        for k in d:
            s += self.__sum(d[k])

        return s

    def __show(self, row, kk, vv):
        keys = vv.keys()
        trs = [ tr() for i in keys[1:] ]

        row << td(kk, rowspan="%d" % self.__sum(vv), align="center")

        if keys:
            row = self.__show(row, keys[0], vv[keys[0]])

            for i,key in enumerate(keys[1:]):
                trs[i] = self.__show(trs[i], key, vv[key])

        for t in trs:
            row = row + t

        return row

    def show(self, content):
        ret = None
        for k in content:
            v = content[k]
            if not ret:
                ret = self.__show(tr(), k, v)
            else:
                ret = ret + self.__show(tr(), k, v)

        return ret

    def toHTML(self, f=""):
        c = self.show(self.content)
        if c:
            self.tbl << c
        else:
            self.page << p("<font color=red>No Result, Please check!</font>")

        if f:
            self.page.printOut(f)
        else:
            return self.page.render()

def test():
    db = Record()
    teams = db.get_tables()

    html = ""
    for team in teams:
        header = db.get_table_header(team)
        for s in ("id", "new", "attach_content"):
            header.remove(s)
        new_records = db.query_new_records(team, header, update=0)
        tbl = HtmlTable(team, header, (10, 15, 15, 10, 30, 10, 15))
        for r in new_records:
            tbl.insert_result(r)
        html += tbl.toHTML()

    print html

if __name__ == "__main__":
    from record import *
    test()

