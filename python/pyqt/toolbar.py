#!/usr/bin/env python

'''
toolbar
'''

import sys
from PyQt4 import QtGui

class ToolBar(QtGui.QMainWindow):
	def __init__(self):
		super(ToolBar, self).__init__()
		self.initUI()

	def initUI(self):
		exitAction = QtGui.QAction(QtGui.QIcon('exit.png'), 'Exit', self)
		exitAction.setShortcut('Ctrl+Q')
		exitAction.triggered.connect(self.exit)

		saveAction = QtGui.QAction(QtGui.QIcon('save.png'), 'Save', self)
		saveAction.triggered.connect(self.save)

		self.toolbar = self.addToolBar('Exit')
		self.toolbar.addAction(exitAction)

		self.toolbar2 = self.addToolBar('Save')
		self.toolbar2.addAction(saveAction)

		self.setGeometry(300, 300, 300, 200)
		self.setWindowTitle('Toolbar')
		self.show()

	def save(self):
		pass

	def exit(self):
		sys.exit()

def main():
	app = QtGui.QApplication(sys.argv)
	ex = ToolBar()
	sys.exit(app.exec_())

if __name__ == '__main__':
	main()
