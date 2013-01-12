#!/usr/bin/env python

"""
status bar
"""

import sys
from PyQt4 import QtGui

class StatusBar(QtGui.QMainWindow):
	def __init__(self):
		super(StatusBar, self).__init__()
		self.initUI()

	def initUI(self):
		self.statusBar().showMessage("Ready")

		self.setGeometry(300, 300, 250, 150)
		self.setWindowTitle("StatusBar")
		self.show()

def main():
	app = QtGui.QApplication(sys.argv)
	ex = StatusBar()
	sys.exit(app.exec_())

if __name__ == '__main__':
	main()
