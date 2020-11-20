#!/usr/bin/python

import Tkinter
from Tkconstants import *

class Piece:
    pass

class Board(Tkinter.Frame, object):

    def __init__(self, master=None):
        super(Board, self).__init__(master)
        self.pack()
        self.createWidget()

    def createWidget(self):
        self.button = Tkinter.Button(self, text="Exit", command=self.exit)
        self.button.pack()

    def exit(self):
        print "exit"


if __name__ == "__main__":
    tk = Tkinter.Tk()
    b = Board(tk)
    tk.mainloop()

