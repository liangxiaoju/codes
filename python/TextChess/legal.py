#!/usr/bin/python

import os
import sys
import tty
import termios
from subprocess import Popen, PIPE

class BaseSite(object):
    def __init__(self, role=" "):
        self.role = role
        self.focused = False
        self.selected = False
        self.flashed = False
        self.bolded = False
        self.colorTBL = {
                "black":0,
                "red":1,
                "green":2,
                "brown":3,
                "blue":4,
                "purple":5,
                "cyan":6,
                "white":7}
        self.setFgColor("white")
        self.setBgColor("black")
        self.underlined = False

    def setFocus(self, focus):
        self.focused = focus

    def isFocused(self):
        return self.forcused

    def setSelect(self, select):
        self.selected = select

    def isSelected(self):
        return self.selected

    def setFlash(self, flash):
        self.flashed = flash

    def setBold(self, bold):
        self.bolded = bold

    def setFgColor(self, color):
        if color in self.colorTBL:
            self.fgcolor = 30 + self.colorTBL[color]

    def setBgColor(self, color):
        if color in self.colorTBL:
            self.bgcolor = 40 + self.colorTBL[color]

    def setUnderline(self, underline):
        self.underlined = underline

    def show(self):
        content = self.role
        prefix = "%d" % self.fgcolor
        if self.bolded:
            prefix += ";1"
        if self.flashed:
            prefix += ";5"
        if self.selected:
            prefix += ";7"
        if self.underlined:
            prefix += ";4"

        content = "\033[%sm" % prefix + content + "\033[0m"

        bgcolor = "\033[%dm" % self.bgcolor

        if self.focused:
            content = bgcolor + "[" + content + bgcolor + "]" + "\033[0m"
        else:
            content = bgcolor + " " + content + bgcolor + " " + "\033[0m"

        return content

class EmptySite(BaseSite):
    def __init__(self):
        super(EmptySite, self).__init__("-")

class PieceSite(BaseSite):

    def __init__(self, name):
        super(PieceSite, self).__init__(name)


class ChessView:

    def __init__(self, rows=10, cols=9):
        self.rows = rows
        self.cols = cols
        self.content = [ [ EmptySite() for c in range(cols) ] for r in range(rows) ]
        self.history = []
        self.selectedCoord = None
        self.focusCoord = "e4"
        self.focus(self.focusCoord)
        self.initBoard()
        self.whoInTurn = "r"
        self.update()
        self.startEngine()

    def startEngine(self):
        dirname = os.path.dirname(sys.argv[0])
        machine = os.uname()[4]
        if machine in ("x86", "x86_64"):
            enginePath = os.path.join(dirname, "eleeye", "ELEEYE.PC")
            tmpdir = "/tmp"
        else:
            enginePath = os.path.join(dirname, "eleeye", "ELEEYE.ARM")
            tmpdir = "/data/local/tmp"

        xname = os.path.join(tmpdir, "ELEEYE")

        with open(enginePath, "rb") as fsrc:
            with open(xname, "wb") as fdst:
                while True:
                    buf = fsrc.read(16*1024)
                    if not buf:
                        break
                    fdst.write(buf)

        try:
            os.chmod(xname, 0700)
        except:
            pass

        self.engine = Popen(xname, stdin=PIPE, stdout=PIPE, stderr=PIPE)

        self.engine.stdin.write("ucci\n")
        while True:
            line = self.engine.stdout.readline()
            if line.startswith("ucciok"):
                break

    def stopEngine(self):
        self.engine.terminate()
        self.engine.wait()

    def arrayIndex(self, coord):
        try:
            r = self.rows - 1 - int(coord[1])
            c = ord(coord[0]) - ord("a")
            return (r, c)
        except:
            raise IndexError("Illegal index.")

    def coordIndex(self, r, c):
        r = str(self.rows - 1 - int(r))
        c = chr(ord("a") + int(c))
        return c + r

    def __getitem__(self, coord):
        r, c = self.arrayIndex(coord)
        return self.content[r][c]

    def __setitem__(self, coord, val):
        r, c = self.arrayIndex(coord)
        self.content[r][c] = val

    def pieceSide(self, piece):
        if "a" <= piece.role <= "z":
            return "b"
        elif "A" <= piece.role <= "Z":
            return "r"
        else:
            return None

    def initBoard(self):
        d = {
                "a9":"r","b9":"n","c9":"b","d9":"a","e9":"k","f9":"a","g9":"b","h9":"n","i9":"r",
                "b7":"c","h7":"c",
                "a6":"p","c6":"p","e6":"p","g6":"p","i6":"p",

                "a0":"R","b0":"N","c0":"B","d0":"A","e0":"K","f0":"A","g0":"B","h0":"N","i0":"R",
                "b2":"C","h2":"C",
                "a3":"P","c3":"P","e3":"P","g3":"P","i3":"P",
                }

        for i in d:
            self[i] = PieceSite(d[i])

    def select(self, coord):
        if not coord:
            return

        if isinstance(self[coord], PieceSite):
            self.selectedCoord = coord 
            self[coord].setSelect(True)
            self.update()

    def unselect(self, coord):
        if not coord:
            return
        self.selectedCoord = None
        self[coord].setSelect(False)
        self.update()

    def focus(self, coord):
        self[coord].setFocus(True)
        self.update()

    def unfocus(self, coord):
        self[coord].setFocus(False)
        self.update()

    def move_focus(self, roffset, coffset):
        coord = self.focusCoord
        self.unfocus(coord)
        r, c = self.arrayIndex(coord)
        r = (r + roffset) % self.rows
        c = (c + coffset) % self.cols
        coord = self.coordIndex(r, c)
        self.focus(coord)
        self.focusCoord = coord

    def update(self):
        #print "\33[2J"
        print ""
        bold = (
                "d2", "e2", "f2", "d1", "e1", "f1", "d0", "e0", "f0",
                "d9", "e9", "f9", "d8", "e8", "f8", "d7", "e7", "f7",
                )

        river_row = (4, 5)
        cannon = ("b2", "h2", "b7", "h7")
        pawn = ("a3", "c3", "e3", "g3", "i3", "a6", "c6", "e6", "g6", "i6")

        for r in range(self.rows):
            line = ""
            for c in range(self.cols):
                if self.coordIndex(r, c) in bold:
                    self.content[r][c].setFgColor("green")

                if r in river_row:
                    self.content[r][c].setBold(True)

                if self.coordIndex(r, c) in cannon:
                    self.content[r][c].setUnderline(True)

                if self.coordIndex(r, c) in pawn:
                    self.content[r][c].setUnderline(True)

                line += self.content[r][c].show()

            print line
        print "\33[12A"

    def cleanFocus(self):
        for r in range(self.rows):
            for c in range(self.cols):
                self.content[r][c].setFocus(False)

    def isLegalMv(self, isrc, idst):
        mvs = ""
        for i in self.history:
            mvs += " " + i
        mvs += " " + isrc + idst
        cmd = "position startpos moves" + mvs + "\n"
        self.engine.stdin.write(cmd)
        self.engine.stdin.write("go depth 1\n")
        while True:
            try:
                line = self.engine.stdout.readline()
            except:
                return "a0","a0"

            if line.startswith("bestmove"):
                mv = line.split()[1]
                break

        s = mv[0:2]
        if self.pieceSide(self[s]) != self.nextTurn(self.whoInTurn):
            return False
        else:
            return True

    def getComputerMove(self):
        mvs = ""
        for i in self.history:
            mvs += " " + i
        cmd = "position startpos moves" + mvs + "\n"
        self.engine.stdin.write(cmd)
        #self.engine.stdin.write("go depth 5\n")
        self.engine.stdin.write("go time 5000\n")
        while True:
            try:
                line = self.engine.stdout.readline()
            except:
                return "a0","a0"

            if line.startswith("bestmove"):
                mv = line.split()[1]
                break

        return mv[0:2],mv[2:4]

    def nextTurn(self, inTurn):
        if inTurn == "r":
            nextTurn = "b"
        elif inTurn == "b":
            nextTurn = "r"
        return nextTurn


    def movePiece(self, isrc, idst):
        legal = False
        if isrc != idst and self.pieceSide(self[isrc]) != self.pieceSide(self[idst]):
            if isinstance(self[isrc], PieceSite) and self.pieceSide(self[isrc]) == self.whoInTurn:
                legal = self.isLegalMv(isrc, idst)
                if legal:
                    self.history.append(isrc+idst)
                    self[idst] = PieceSite(self[isrc].role)
                    self[isrc] = EmptySite()
                    self.whoInTurn = self.nextTurn(self.whoInTurn)
                self.update()
        return legal

    def waitForEvent(self):
        fd = sys.stdin.fileno()
        mode = termios.tcgetattr(fd)
        tty.setcbreak(fd)
        while True:
            try:
                key = sys.stdin.read(1)
            except:
                break

            if key == "q":
                break

            focus = self.focusCoord
            if key == "j":
                self.move_focus(1, 0)
            elif key == "k":
                self.move_focus(-1, 0)
            elif key == "h":
                self.move_focus(0, -1)
            elif key == "l":
                self.move_focus(0, 1)
            elif key == " ":
                if self.selectedCoord:
                    old = self.selectedCoord
                    new = self.focusCoord
                    if self.pieceSide(self[old]) == self.pieceSide(self[new]):
                        self.unselect(old)
                        if old != new:
                            self.select(new)
                    else:
                        legal = self.movePiece(old, new)
                        if legal:
                            self.cleanFocus()
                            self.unselect(old)
                            self.unselect(new)
                            self.focus(old)
                            self.focus(new)
                            s,d = self.getComputerMove()
                            if self.pieceSide(self[s]) != self.whoInTurn:
                                self.gameOver()
                            else:
                                self.movePiece(s, d)
                                self.focus(s)
                                self.focus(d)
                else:
                    coord = self.focusCoord
                    if self[coord].isSelected():
                        self.unselect(coord)
                    else:
                        self.select(coord)
            elif ord(key) == 27: #Esc
                self.unselect(self.selectedCoord)

        termios.tcsetattr(fd, termios.TCSAFLUSH, mode)
        self.stopEngine()

    def gameOver(self):
        if self.whoInTurn == "r": king = "k"
        else: king = "K"
        for r in range(self.rows):
            for c in range(self.cols):
                if self.content[r][c].role == king:
                    self.content[r][c].setFlash(True)
                    self.update()

    def loop(self):
        self.waitForEvent()


def main():
    view = ChessView()
    view.loop()

if __name__ == "__main__":
    main()
