#!/usr/bin/env python

"""main window
"""

import sys
from PyQt4 import QtGui
from PyQt4 import QtCore

class MainWin(QtGui.QMainWindow):
	def __init__(self):
		super(MainWin, self).__init__()
		self.initAction()
		self.initMenuBar()
		self.initToolBar()
		self.initTextEdit()
		self.initStatusBar()
		self.initFinish()

	def initAction(self):
		self.openAction = QtGui.QAction('&Open', self)
		self.openAction.setStatusTip('Open file')
		self.openAction.triggered.connect(self._open)

		self.exitAction = QtGui.QAction('&Exit', self)
		self.exitAction.setShortcut('Ctrl+Q')
		self.exitAction.setStatusTip('Exit application')
		self.exitAction.triggered.connect(self._exit)

		#self.undoAction = QtGui.QAction(QtGui.QIcon('undo.png'), '&Undo', self)
		self.undoAction = QtGui.QAction('&Undo', self)
		self.undoAction.setStatusTip('Undo')
		self.undoAction.triggered.connect(self._undo)

		self.redoAction = QtGui.QAction('&Redo', self)
		self.redoAction.setStatusTip('Redo')
		self.redoAction.triggered.connect(self._redo)

		self.saveAction = QtGui.QAction('&Save', self)
		self.saveAction.setStatusTip('Save')
		self.saveAction.triggered.connect(self._save)

		self.aboutAction = QtGui.QAction('&About', self)
		self.aboutAction.setStatusTip('About')
		self.aboutAction.triggered.connect(self._about)

	def initStatusBar(self):
		self.statusBar().showMessage("Ready")

	def initMenuBar(self):
		menubar = self.menuBar()

		fileMenu = menubar.addMenu('&File')
		fileMenu.addAction(self.openAction)
		fileMenu.addAction(self.saveAction)
		fileMenu.addAction(self.exitAction)

		editMenu = menubar.addMenu('&Edit')
		editMenu.addAction(self.undoAction)
		editMenu.addAction(self.redoAction)

		helpMenu = menubar.addMenu('&Help')
		helpMenu.addAction(self.aboutAction)

	def initToolBar(self):
		toolbar = self.addToolBar('ToolBar')
		toolbar.addAction(self.saveAction)

	def initTextEdit(self):
		self.textEdit = QtGui.QTextEdit()
		self.textEdit.setTabStopWidth(35)
		self.setCentralWidget(self.textEdit)

	def initFinish(self):
		self.setGeometry(300, 300, 300, 200)
		self.setWindowTitle('MainWin')
		self.show()

	def _open(self):
		"""slot open"""

		filename = QtGui.QFileDialog.getOpenFileName(self, "open", "/tmp", "File Type (*.txt)")
		if not filename.isNull():
			f = file(filename, "r")
			context = f.read()
			self.textEdit.setText(context)
			f.close()

	def _save(self):
		"""slot save"""

		filename = QtGui.QFileDialog.getSaveFileName(self, "choose", "/tmp", "File Type (*.txt)")
		if not filename.isNull():
			f = file(filename, "w")
			f.write(self.textEdit.toPlainText())
			f.close()

	def _undo(self):
		"""slot undo"""

		self.textEdit.undo()

	def _redo(self):
		"""slot redo"""

		self.textEdit.redo()

	def _about(self):
		"""slot about
		display a QMessageBox"""

		QtGui.QMessageBox().aboutQt(self)

	def _exit(self):
		"""slot exit"""

		sys.exit()

def main():
	app = QtGui.QApplication(sys.argv)
	ex = MainWin()
	sys.exit(app.exec_())

if __name__ == '__main__':
	main()

