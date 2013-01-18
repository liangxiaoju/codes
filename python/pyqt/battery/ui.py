#!/usr/bin/env python
"""A battery tool
"""

import os
import sys
from PyQt4 import QtGui, QtCore
import power
try:
    import images_rc
except:
    os.system("pyrcc4 -o images_rc.py images.qrc")
    try:
        import images_rc
    except:
        print "Please generate images_rc.py first,"
        print "run 'pyrcc4 -o images_rc.py images.qrc'."

class MainWin(QtGui.QDialog):
    def __init__(self, parent=None):
        super(MainWin, self).__init__(None)

        self.power = power.Power(self.onEvent)

        self.timer = QtCore.QTimer()
        self.timer.setInterval(500)
        self.timer.setSingleShot(False)
        self.connect(self.timer, QtCore.SIGNAL("timeout()"), self.iconFlicker)

        # define ourself signals
        self.connect(self, QtCore.SIGNAL("startWarning_"), self.startWarning)
        self.connect(self, QtCore.SIGNAL("stopWarning_"), self.stopWarning)

        self.createActions()
        self.createTrayIcon()
        self.createTopTabWidget()

        mainLayout = QtGui.QGridLayout()
        mainLayout.addWidget(self.topTabWidget, 0, 0)
        self.setLayout(mainLayout)
        self.setWindowTitle("PyBattery")
        self.changeStyle("Cleanlooks")

        self.trayIcon.show()

        QtGui.qApp.lastWindowClosed.connect(self.__exiting)

        self.power.start()

    def iconFlicker(self):
        if not self.trayIcon.icon().isNull():
            self.trayIcon.setIcon(QtGui.QIcon())
        else:
            self.trayIcon.setIcon(self.trayIconPic)

    def stopWarning(self, *args, **kwargs):
        print "stop warning"
        self.timer.stop()
        self.trayIcon.setIcon(self.trayIconPic)

    def startWarning(self, *args, **kwargs):
        print "start warning"
        if self.trayIcon.supportsMessages():
            self.trayIcon.showMessage(args[0], args[1])
        self.timer.start()

    # not UI thread, we cannt do any UI operation here
    def onEvent(self):
        capacity = self.power.getBattCap()
        self.progressBar.setValue(capacity)
        if capacity <= self.warnValue:
            if not self.timer.isActive():
                self.emit(QtCore.SIGNAL("startWarning_"), "Warning", "Battery Capacity too low !")
        else:
            if self.timer.isActive():
                self.emit(QtCore.SIGNAL("stopWarning_"))

    def trayIconActivated(self, reason):
        if reason in (QtGui.QSystemTrayIcon.Trigger, QtGui.QSystemTrayIcon.DoubleClick):
            if self.isHidden():
                self.show()
            else:
                self.hide()

    def setWarning(self, value):
        self.warnValue = value
        self.power.setWarning(value)
        self.onEvent()

    def changeStyle(self, styleName):
        QtGui.QApplication.setStyle(QtGui.QStyleFactory.create(styleName))

    def __suspendToMode(self, mode):
        try:
            with open("/sys/power/state", "r+") as f:
                f.write(mode)
        except IOError:
            print "Failed to suspend to", mode

    def systemSleep(self):
        self.__suspendToMode("mem")

    def systemHibernate(self):
        self.__suspendToMode("disk")

    def systemShutdown(self):
        os.system("poweroff -h -i")

    def systemReboot(self):
        os.system("reboot")

    def createActions(self):
        self.sleepAction = QtGui.QAction(QtGui.QIcon(":/images/sleep.png"),
                                            "Sleep", self,
                                            triggered=self.systemSleep)

        self.hibernateAction = QtGui.QAction(QtGui.QIcon(":/images/hibernate.png"),
                                           "Hibernate", self,
                                           triggered=self.systemHibernate)

        self.shutdownAction = QtGui.QAction(QtGui.QIcon(":/images/shutdown.png"),
                                          "Shutdown", self,
                                          triggered=self.systemShutdown)

        self.rebootAction = QtGui.QAction(QtGui.QIcon(":/images/reboot.png"),
                                          "Reboot", self,
                                          triggered=self.systemReboot)

        self.quitAction = QtGui.QAction(QtGui.QIcon(":/images/exit.png"),
                                        "&Quit", self, triggered=self.__exiting)

    def createTrayIcon(self):
        self.trayIconMenu = QtGui.QMenu(self)
        self.trayIconMenu.addAction(self.sleepAction)
        self.trayIconMenu.addAction(self.hibernateAction)
        self.trayIconMenu.addAction(self.shutdownAction)
        self.trayIconMenu.addAction(self.rebootAction)
        self.trayIconMenu.addSeparator()
        self.trayIconMenu.addAction(self.quitAction)

        self.trayIconPic = QtGui.QIcon(":/images/battery.png")
        self.trayIcon = QtGui.QSystemTrayIcon(self.trayIconPic, self)
        self.trayIcon.setContextMenu(self.trayIconMenu)
        self.trayIcon.activated.connect(self.trayIconActivated)

    def createTopTabWidget(self):
        self.topTabWidget = QtGui.QTabWidget()
        self.topTabWidget.setSizePolicy(QtGui.QSizePolicy.Preferred,
                                        QtGui.QSizePolicy.Preferred)
        #self.topTabWidget.setTabPosition(QtGui.QTabWidget.West)

        tab1 = QtGui.QWidget()
        self.labelWidget = QtGui.QLabel("capacity:")
        self.progressBar = QtGui.QProgressBar()
        self.progressBar.setRange(0, 100)
        self.progressBar.setValue(self.power.getBattCap())
        tab1hbox = QtGui.QHBoxLayout()
        tab1hbox.setMargin(5)
        tab1hbox.addWidget(self.labelWidget)
        tab1hbox.addWidget(self.progressBar)
        tab1.setLayout(tab1hbox)

        tab2 = QtGui.QWidget()
        lable = QtGui.QLabel("Warning:")
        self.spinBox = QtGui.QSpinBox()
        self.spinBox.setRange(0, 100)
        self.spinBox.valueChanged.connect(self.setWarning)
        self.spinBox.setValue(5)
        tab2hbox = QtGui.QHBoxLayout()
        tab2hbox.setMargin(5)
        tab2hbox.addWidget(lable)
        tab2hbox.addWidget(self.spinBox)
        tab2.setLayout(tab2hbox)

        self.topTabWidget.addTab(tab1, "info")
        self.topTabWidget.addTab(tab2, "setting")

    def __exiting(self):
        self.power.stop()
        QtGui.qApp.quit()

def main():
    app = QtGui.QApplication(sys.argv)
    ex = MainWin()
    ex.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
