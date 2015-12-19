#!/usr/bin/python

import sys, os
import tty
import termios
import re
from select import select
from subprocess import Popen, PIPE


def arrayIndex(i):
    try:
        r = 10 - 1 - int(i[1])
        c = ord(i[0]) - ord("a")
        return (r, c)
    except:
        raise IndexError("Illegal index.")

def ucciIndex(i):
    r = str(10 - 1 - int(i[0]))
    c = chr(ord("a") + int(i[1]))
    return c + r

def ucciMv(s, d):
    return ucciIndex(s) + ucciIndex(d)

def arrayMv(mv):
    s = arrayIndex(mv[0:2])
    d = arrayIndex(mv[2:4])
    return s, d


class Engine:
    def __init__(self):
        dirname = os.path.dirname(sys.argv[0])
        xname = os.path.join(dirname, "eleeye", "ELEEYE.PC")
        os.chmod(xname, 0744)
        self.engine = Popen(xname, stdin=PIPE, stdout=PIPE, stderr=PIPE)
        self.write("ucci\n")
        self.waitForReply("ucciok")

    def __del__(self):
        self.engine.terminate()
        self.engine.wait()

    def write(self, word):
        self.engine.stdin.write(word)

    def read(self, timeout):
        line = ""
        r, w, x = select([self.engine.stdout], [], [], timeout/1000.0)
        if self.engine.stdout in r:
            line = self.engine.stdout.readline()
        return line

    def waitForReply(self, token, timeout=0):
        self.engine.stdout.flush()
        line = ""
        while True:
            line = self.read(timeout)
            if not line:
                break
            if re.search(token, line):
                break
        return line

    def getMove(self, fen, time=5000):
        cmd = "position fen " + fen + "\n"
        self.write(cmd)
        self.write("go time %d\n" % time)
        line = self.waitForReply("bestmove|nobestmove", timeout=(time+1000))
        #print "cmd: %s; reply: %s" % (cmd, line)

        try:
            return line.split()[1]
        except:
            return None


class Piece:
    def __init__(self, text=" ", fgcolor="white", bgcolor="black"):
        self.colorTBL = {
                "black":0,
                "red":1,
                "green":2,
                "brown":3,
                "blue":4,
                "purple":5,
                "cyan":6,
                "white":7}

        self.setFgColor(fgcolor)
        self.setBgColor(bgcolor)
        self.text = text

    def setFgColor(self, color):
        if color in self.colorTBL:
            self.fgcolor = 30 + self.colorTBL[color]

    def setBgColor(self, color):
        if color in self.colorTBL:
            self.bgcolor = 40 + self.colorTBL[color]

    def __repr__(self):
        return "\033[%d;%dm" % (self.fgcolor, self.bgcolor) + self.text + "\033[0m"
        

class Board:

    def __init__(self):
        self.rows = range(10)
        self.cols = range(9)
        self.pieces = [ [ None for c in self.cols ] for r in self.rows ]
        self.focusPos = []
        self.selectPos = []

    def __repr__(self):
        star = ((2,1), (2,7), (7,1), (7,7), (1,4), (8,4))

        output = "   A  B  C  D  E  F  G  H  I\n\n"
        for r in self.rows:
            line = "%d " % (self.rows[-1] - r)
            for c in self.cols:

                # prefix
                if (r,c) in self.focusPos:
                    line += "["
                elif c != self.cols[0]:
                    line += "-"
                else:
                    line += " "

                # text
                piece = self.pieces[r][c]
                if piece:
                    if (r,c) in self.selectPos:
                        line += "\033[7m" + repr(piece) + "\033[0m"
                    else:
                        line += repr(piece)
                else:
                    if (r,c) in star:
                        line += "*"
                    else:
                        line += "-"

                # suffix
                if (r,c) in self.focusPos:
                    line += "]"
                elif c != self.cols[-1]:
                    line += "-"
                else:
                    line += " "

            output += line + "\n"

            if  r == (len(self.rows)/2-1):
                output += "   |           X           |\n"
            elif r in (0, 7):
                output += "   |  |  |  |\ | /|  |  |  |\n"
            elif r in (1, 8):
                output += "   |  |  |  |/ | \|  |  |  |\n"
            elif r != self.rows[-1]:
                output += "   |  |  |  |  |  |  |  |  |\n"

        output += "\n   0  1  2  3  4  5  6  7  8\n"

        return output


    def setPiece(self, i, piece):
        self.pieces[i[0]][i[1]] = piece

    def focus(self, i):
        self.focusPos.append(tuple(i))

    def select(self, i):
        self.selectPos.append(tuple(i))


class Controller:

    def __init__(self):
        fd = sys.stdin.fileno()
        self.termios_mode = termios.tcgetattr(fd)
        tty.setcbreak(fd)
        # hide cursor
        print "\33[?25l"

        self.__new()

    def __del__(self):
        # show cursor and move down 30 lines
        print "\33[?25h"
        print "\33[30B"
        fd = sys.stdin.fileno()
        termios.tcsetattr(fd, termios.TCSAFLUSH, self.termios_mode)

    def __new(self):
        self.model = Model()
        self.board = Board()
        self.engine = Engine()
        self.initFromFen(self.getFen())
        self.currentFocus = [5, 4]
        self.focus(self.currentFocus)
        self.currentSelect = []
        self.exit = False
        self.word = ""
        self.userRole = "r"
        self.askForHint = False
        self.skipInfo = False
        self.thinkTime = 1000
        self.debug = False

    def initFromFen(self, fen):
        rows = fen.split()[0].split("/")
        for r,line in enumerate(rows):
            c = 0
            for ch in line:
                if ch.isdigit():
                    c += int(ch)
                else:
                    if ch.islower():
                        p = Piece(ch, fgcolor="blue")
                    else:
                        p = Piece(ch, fgcolor="red")
                    self.board.setPiece((r, c), p)
                    c += 1
        self.whoInTurn = fen.split()[1]

    def getFen(self):
        '''
        get Fen string from Model
        '''
        return self.model.getFen()

    def getFenWithMoves(self):
        return self.model.getFenWithMoves()

    def isLegalMv(self, s, d):
        mv = ucciMv(s, d)
        fen = self.getFenWithMoves()
        if fen.find("moves") >= 0:
            fen += " " + mv
        else:
            fen += " moves " + mv

        mv = self.engine.getMove(fen, time=0)
        if mv == None:
            return True

        (s, d) = arrayMv(mv)
        if self.model.pieceSide(s) == self.whoInTurn:
            return False
        else:
            return True

    def movePiece(self, s, d):
        '''
        tell Model to move Piece
        '''
        ret = self.model.move(s, d)
        if ret != None:
            self.focus(s)
            self.focus(d)

    def undo(self):
        '''
        undo model's data
        '''
        self.model.undo()

    def say(self, word):
        self.word = word

    def updateUi(self):
        output = repr(self.board)
        # clean the line and print word
        output += "\n\33[K: " + self.word
        print output
        print "\33[%dA" % (len(output.split('\n'))+1)
        # new board
        self.board = Board()

    def focus(self, i):
        self.board.focus(i)

    def select(self, i):
        self.board.select(i)

    def loop(self):

        while True:
            self.initFromFen(self.getFen())

            if self.gameOver():
                win = "Black" if self.whoInTurn == "r" else "Red"
                self.say("%s win! Press 'n' to Open new board." % win)
                self.updateUi()
                try:
                    while sys.stdin.read(1) != "n":
                        pass
                    self.__new()
                except:
                    break

            if not self.skipInfo:
                turn = "Red" if self.whoInTurn == "r" else "Black"
                self.say("It is %s's turn." % turn)

            self.skipInfo = False
            self.updateUi()

            if self.exit:
                break

            try:
                if self.whoInTurn == self.userRole:
                    if self.askForHint:
                        self.action_robot()
                        self.askForHint = False
                    else:
                        self.action_user()
                else:
                    self.action_robot()
            except:
                break

    def gameOver(self):
        mv = self.engine.getMove(self.getFenWithMoves(), time=0)
        return not mv

    def action_user(self):
        key = sys.stdin.read(1)
        self.handleKeyEvent(key)

    def action_robot(self):
        mv = self.engine.getMove(self.getFenWithMoves(), time=self.thinkTime)
        if mv:
            s, d = arrayMv(mv)
            self.movePiece(s, d)
        self.focus(self.currentFocus)

    def handleKeyEvent(self, key):
        if key == "q":
            self.exit = True

        if key == "j":
            self.currentFocus[0] += 1
        elif key == "k":
            self.currentFocus[0] -= 1
        elif key == "h":
            self.currentFocus[1] -= 1
        elif key == "l":
            self.currentFocus[1] += 1

        if self.currentFocus:
            self.currentFocus[0] %= 10
            self.currentFocus[1] %= 9
            self.focus(self.currentFocus)

        if key == " ":
            if self.currentSelect:
                if self.isLegalMv(self.currentSelect, self.currentFocus):
                    self.movePiece(self.currentSelect, self.currentFocus)
                self.currentSelect = []
            else:
                self.currentSelect = self.currentFocus[:]

        if self.currentSelect:
            self.select(self.currentSelect)

        if key == "u":
            self.undo()

        if key == "d":
            self.debug = not self.debug
            self.say("Debug: " + str(self.debug))
            self.updateUi()
            self.skipInfo = True

        if key == "s":
            self.userRole = "r" if self.userRole == "b" else "r"

        if key == "a":
            self.askForHint = True

        if key == "?":
            self.say("Move[hjkl] Undo[u] SwithRole[s] Hint[a] New[n] Difficulty[+-]")
            self.updateUi()
            self.skipInfo = True

        if key == "n":
            self.__new()

        if key in ("+", "-"):
            self.thinkTime = eval(str(self.thinkTime)+key+str(100))
            self.thinkTime = 0 if self.thinkTime < 0 else self.thinkTime
            self.say("ThinkTime %dms" % self.thinkTime)
            self.updateUi()
            self.skipInfo = True


class Model:
    def __init__(self):
        self.startpos = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r"

        self.fen = self.startpos
        self.initFromFen(self.fen)
        self.historyMvs = []
        self.historyFens = [self.fen, ]

    def initFromFen(self, fen):
        self.pieces = [ [ "" for c in range(9) ] for r in range(10) ]
        rows = fen.split()[0].split("/")
        for r,line in enumerate(rows):
            c = 0
            for ch in line:
                if ch.isdigit():
                    c += int(ch)
                else:
                    self.pieces[r][c] = ch
                    c += 1

        self.whoInTurn = fen.split()[1]

    def getFen(self):
        fen = ""
        for r,lines in enumerate(self.pieces):
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

        fen += " " + self.whoInTurn

        return fen

    def getFenWithMoves(self):
        mvs = ""
        if self.historyMvs:
            mvs += " moves "
            mvs += " ".join(self.historyMvs)
        fen = self.startpos + mvs
        return fen

    def switchTurn(self):
        self.whoInTurn = "r" if self.whoInTurn == "b" else "b"

    def pieceSide(self, i):
        return "b" if self.pieces[i[0]][i[1]].islower() else "r"

    def undo(self):
        try:
            self.historyMvs.pop()
            self.historyFens.pop()

            mv = self.historyMvs.pop()
            self.historyFens.pop()
            fen = self.historyFens[-1]

            self.initFromFen(fen)
        except:
            pass

    def move(self, s, d):
        sr,sc = s
        dr,dc = d

        if s == d or self.pieceSide(s) != self.whoInTurn:
            return None

        if self.pieces[sr][sc].isalpha():
            tmp = self.pieces[sr][sc]
            self.pieces[sr][sc] = ""
            kill = self.pieces[dr][dc]
            self.pieces[dr][dc] = tmp
            self.switchTurn()
            mv = ucciMv(s, d)
            self.historyMvs.append(mv)
            self.historyFens.append(self.getFen())
            return kill

        return None

def main():
    c = Controller()
    c.loop()

if __name__ == "__main__":
    main()

