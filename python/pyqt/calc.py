#!/usr/bin/env python

import sys
from PyQt4 import QtCore
from PyQt4 import QtGui

class Calculator(QtGui.QWidget):

	def __init__(self):
		super(Calculator, self).__init__()
		self.initUI()

	def initUI(self):
		btn_quit = QtGui.QPushButton("Quit", self)
		btn_quit.clicked.connect(QtCore.QCoreApplication.instance().quit)
		btn_quit.resize(btn_quit.sizeHint())

		btn_calc = QtGui.QPushButton("Calc", self)
		btn_calc.clicked.connect(self.calc)
		btn_calc.resize(btn_calc.sizeHint())

		self.text = QtGui.QLineEdit(self)

		self.label = QtGui.QLabel(" = ")

		grid = QtGui.QGridLayout()
		grid.addWidget(self.text, 0, 0)
		grid.addWidget(self.label, 0, 1)
		grid.addWidget(btn_calc, 1, 0)
		grid.addWidget(btn_quit, 1, 1)

		self.setLayout(grid)

		self.setGeometry(300, 300, 250, 200)
		self.setWindowTitle("Calculator")
		self.show()

	def calc(self):
		s = eval(str(self.text.text()))
		self.label.setText(" = " + str(s))

def main():
	app = QtGui.QApplication(sys.argv)
	ex = Calculator()
	sys.exit(app.exec_())

if __name__ == "__main__":
	main()
