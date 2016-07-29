#!/usr/bin/env python

from struct import unpack, pack
from optparse import OptionParser
import json
import ctypes
import sys
import sqlite3
import os

class DB(object):

    def __init__(self, filename="file.db"):
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
        CREATE TABLE IF not exists %s
        (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        pid INTEGER,
        status INTEGER DEFAULT 0,
        type INTEGER DEFAULT 0,
        progress INTEGER DEFAULT 0,
        name TEXT,
        desc TEXT,
        json TEXT
        )
        """ % tbl
        self._cursor.execute(sql)

    def insert_record(self, tbl, pid, name, desc, json):
        sql = """
        INSERT INTO
        %s (pid, name, desc, json)
        VALUES ('%d', '%s', '%s', '%s')
        """ % (tbl, pid, name, desc, json)

        try:
            self._cursor.execute(sql)
            #self._connect.commit()
        except:
            raise Exception(sql)

        sql = "SELECT id FROM %s ORDER BY id DESC limit 1" % tbl
        self._cursor.execute(sql)
        return self._cursor.fetchone()[0]


class XQFFile(object):

    def __init__(self, filename):
        self.filename = filename
        with open(filename, "rb") as f:
            self.data = f.read()

        self.check_format()
        self.parse_header()
        self.parse_decryptkey()
        self.parse_fen()
        self.read_moves()

        #print self.header

    def get_number(self, offset, len=1):
        map = {1:"B", 2:"H", 4:"I"}
        if not len in map:
            return 0
        return unpack(map[len], self.data[offset:offset+len])[0]

    def get_array(self, offset, len):
        return list(unpack("%dB" % len, self.data[offset:offset+len]))

    def get_string(self, offset):
        len = unpack("B", self.data[offset])[0]
        name = self.data[offset+1:offset+1+len]
        name = unpack("%ds" % len, name)[0]
        try:
            s = name.decode("cp936").encode("utf-8")
        except:
            try:
                s = name.decode("gb18030").encode("utf-8")
            except:
                s = name.decode("ISO-8859-2").encode("utf-8")
        return s

    def parse_header(self):
        data = self.data
        header = {}
        header["Signature"] = data[0:2]
        header["Version"] = self.get_number(2)
        header["KeyMask"] = self.get_number(3)
        header["ProductId"] = self.get_number(4, 4)
        header["KeysSum"] = self.get_number(12)
        header["KeyXY"] =  self.get_number(13)
        header["KeyXYf"] = self.get_number(14)
        header["KeyXYt"] = self.get_number(15)
        header["QiziXY"] = self.get_array(16, 32)
        header["PlayStepNo"] = self.get_number(48, 2)
        header["WhoPlay"] = "w" if self.get_number(50) == 0 else "b"
        header["CodeA"] = self.get_number(64, 2)
        header["Title"] = self.get_string(0x50)
        header["Match"] = self.get_string(0xd0)
        header["Date"] = self.get_string(0x110)
        header["Addr"] = self.get_string(0x120)
        header["Red"] = self.get_string(0x130)
        header["Black"] = self.get_string(0x140)
        header["Annotator"] = self.get_string(0x1d0)
        header["Author"] = self.get_string(0x1e0)

        self.header = header

    def parse_fen(self):
        board = [ [ "" for c in range(9) ] for r in range(10) ]
        piecePos = [ 0xff for i in range(32) ]

        if (self.header["Version"] < 12):
            for i in range(0, 32):
                piecePos[i] = ctypes.c_uint8(self.header["QiziXY"][i] - self.piece_offset).value
        else:
            for i in range(0, 32):
                index = (self.piece_offset + 1 + i) % 32
                piecePos[index] = ctypes.c_uint8(self.header["QiziXY"][i] - self.piece_offset).value

        self.header["QiziXY"] = piecePos

        symbol = "RNBAKABNRCCPPPPPrnbakabnrccppppp"
        for i,pos in enumerate(piecePos):
            if pos < 90:
                try:
                    board[9-pos%10][pos/10] = symbol[i]
                except:
                    raise Exception("pos=%d" % pos)

        fen = ""
        for r,lines in enumerate(board):
            n = 0
            for c,ch in enumerate(lines):
                if ch.isalpha():
                    if n != 0:
                        fen += str(n)
                        n = 0
                    fen += ch
                else:
                    n += 1
            if n != 0:
                fen += str(n)
            if r != 9:
                fen += "/"

        fen += " " + self.header["WhoPlay"]

        self.header["fen"] = fen

    def check_format(self):
        if self.data[0:2] != "XQ":
            raise Exception("Not a xqf file.")

    def parse_decryptkey(self):
        Square54Plus221 = lambda x: x * x *54 + 221

        self.piece_offset = 0
        self.src_offset = 0
        self.dst_offset = 0
        self.comment_offset = 0
        self.encrypt_key = [0,] * 32

        if self.header["Version"] >= 11:
            self.piece_offset = (Square54Plus221(self.get_number(0xd))
                                 * self.get_number(0xd)) & 0xff
            self.src_offset = (Square54Plus221(self.get_number(0xe))
                               * self.piece_offset) & 0xff
            self.dst_offset = (Square54Plus221(self.get_number(0xf))
                               * self.src_offset) & 0xff
            self.comment_offset = ((self.get_number(0xc) * 256
                                    + self.get_number(0xd)) % 0x7d00) + 0x2ff

            arg0 = self.get_number(3)
            args = [0,] * 4
            for i in range(0, 4):
                args[i] = self.get_number(8+i) | (self.get_number(12+i) & arg0)

            encrypt_key_mask = "[(C) Copyright Mr. Dong Shiwei.]"
            for i in range(0, 32):
                self.encrypt_key[i] = (args[i % 4] & ord(encrypt_key_mask[i])) & 0xff

            #print "piece_offset: ", self.piece_offset
            #print "src_offset: ", self.src_offset
            #print "dst_offset: ", self.dst_offset
            #print "comment_offset: ", self.comment_offset

    def read_decrypt(self, offset, len):
        buf = self.get_array(offset, len)
        for i in range(0, len):
            buf[i] -= self.encrypt_key[(offset+i)%32]
            buf[i] = ctypes.c_uint8(buf[i]).value
        return buf

    def read_decrypt_number(self, offset, len):
        buf = self.read_decrypt(offset, len)
        map = {1:"B", 2:"H", 4:"I"}
        if not len in map:
            return 0
        p = pack("%dB" % len, *buf)
        return unpack(map[len], p)[0]

    def read_decrypt_string(self, offset, len):
        buf = self.read_decrypt(offset, len)
        p = pack("%dB" % len, *buf)
        s = unpack("%ds" % len, p)[0]
        try:
            s = s.decode("cp936").encode("utf-8")
        except:
            try:
                s = s.decode("gb18030").encode("utf-8")
            except:
                s = s.decode("ISO-8859-2").encode("utf-8")
        return s

    def read_decrypt_seq(self, len):
        n = self.read_decrypt(self.offset, len)
        self.offset += len
        return n

    def read_decrypt_number_seq(self, len):
        n = self.read_decrypt_number(self.offset, len)
        self.offset += len
        return n

    def read_decrypt_string_seq(self, len):
        n = self.read_decrypt_string(self.offset, len)
        self.offset += len
        return n

    def _read_moves(self):
        moves = []
        node = {}
        buf = self.read_decrypt_seq(4)

        flags = 0
        commentLen = 0
        if self.header["Version"] <= 0xA:
            if buf[2] & 0xf0:
                flags |= 0x80
            if buf[2] & 0x0f:
                flags |= 0x40
            commentLen = self.read_decrypt_number_seq(4)
        else:
            flags = buf[2]
            flags &= 0xe0
            if flags & 0x20:
                commentLen = self.read_decrypt_number_seq(4) - self.comment_offset

        src = ctypes.c_uint8(buf[0] - 24 - self.src_offset).value
        dst = ctypes.c_uint8(buf[1] - 32 - self.dst_offset).value

        if src < 90 and dst < 90:
            map = "abcdefghi"
            src = map[src/10] + str(src%10)
            dst = map[dst/10] + str(dst%10)
        else:
            src = dst = ""
        node["src"] = src
        node["dst"] = dst

        comment = ""
        if commentLen > 0:
            comment = self.read_decrypt_string_seq(commentLen)
            comment = comment.replace("'", "")
            node["comment"] = comment

        moves.append(node)

        if flags & 0x80:
            nextmoves = self._read_moves()
            moves += nextmoves

        if flags & 0x40:
            altmoves = self._read_moves()
            node["sub"] = [altmoves, ]
            if altmoves[0].has_key("sub"):
                node["sub"] += altmoves[0].pop("sub")

        return moves

    def read_moves(self):
        self.offset = 1024
        moves = self._read_moves()
        self.subno = 1
        self.submovelist = {}
        moves = self.parse_moves(moves)
        self.movelist = moves

    def parse_moves(self, moves):
        for move in moves:
            if move.has_key("sub"):
                keys = []
                for sub in move["sub"]:
                    self.parse_moves(sub)
                    key = str(self.subno)
                    self.submovelist[key] = sub
                    self.subno += 1
                    keys.append(key)
                move["sub"] = keys
        return moves

    def jsonify(self):
        d = {}
        d["id"] = "0"
        d["SubTitle"] = self.header["Match"] or self.header["Title"]
        d["SubTitle"] = d["SubTitle"].replace("'", "")
        d["fen"] = self.header["fen"]
        d["pay"] = "0"
        d["prompt"] = "0"
        d["sort"] = "0"
        d["move"] = {
            "submovelist":self.submovelist,
            "movelist":self.movelist
        }
        #return json.dumps(d, ensure_ascii=False)
        return json.dumps(d)


def main():
    parser = OptionParser()
    parser.add_option("-x", "--xqf", dest="xqffile",
                      help=".xqf file", default=None)
    parser.add_option("-d", "--dir", dest="dir",
                      help="file directory", default=None)
    parser.add_option("-o", "--output", dest="output",
                      help=".db file path", default="file.db")

    (options, args) = parser.parse_args()

    xqffile = options.xqffile
    root_dir = options.dir
    output = options.output

    if xqffile == None and dir == None:
        raise Exception("no xqf file or directory")

    if xqffile:
        xqf = XQFFile(xqffile)
        json = xqf.jsonify()
        print json
    elif dir:
        db = DB(output)
        db.create_table_if_null("Category")
        db.create_table_if_null("Node")
        root_id = db.insert_record("Category", 0, "ROOT", "", "")

        def listdir(pdir, pid):
            for f in os.listdir(pdir):
                p = os.path.join(pdir, f)
                if os.path.isdir(p):
                    id = db.insert_record("Category", pid, f, "", "")
                    listdir(p, id)
                elif os.path.isfile(p):
                    if p.endswith(".XQF") or p.endswith(".xqf"):
                        try:
                            json = XQFFile(p).jsonify()
                            id = db.insert_record("Node", pid, f, "", json)
                        except:
                            print "Skip %s" % p

        if not os.path.isdir(root_dir):
            raise Exception("not direcotry")
        listdir(root_dir, root_id)


if __name__ == "__main__":
    main()
