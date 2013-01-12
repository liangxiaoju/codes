#!/usr/bin/env python

'''
Menu Bar
'''

import sys
from PyQt4 import QtGui

class MenuBar(QtGui.QMainWindow):
	def __init__(self):
		super(MenuBar, self).__init__()
		self.initUI()

	def initUI(self):
		exitAction = QtGui.QAction(QtGui.QIcon('exit.png'), '&Exit', self)
		exitAction.setShortcut('Ctrl+Q')
		exitAction.setStatusTip('Exit application')
		exitAction.triggered.connect(self.exit)

		undoAction = QtGui.QAction(QtGui.QIcon('undo.png'), '&Undo', self)
		undoAction.setStatusTip('Undo')
		undoAction.triggered.connect(self.undo)

		self.statusBar()

		menubar = self.menuBar()
		fileMenu = menubar.addMenu('&File')
		fileMenu.addAction(exitAction)
		editMenu = menubar.addMenu('&Edit')
		editMenu.addAction(undoAction)

		self.setGeometry(300, 300, 300, 200)
		self.setWindowTitle('Menubar')
		self.show()

	def exit(self):
		sys.exit()

	def undo(self):
		pass

def main():

	app = QtGui.QApplication(sys.argv)
	ex = MenuBar()
	sys.exit(app.exec_())

if __name__ == '__main__':
	main()
