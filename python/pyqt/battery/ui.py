#!/usr/bin/env python2.7
"""A battery tool
"""

import os
import sys
import logging
from PyQt4 import QtGui, QtCore
import power
try:
    import images_rc
except:
    print "Please generate images_rc.py first,"
    print "run 'pyrcc4 -o images_rc.py images.qrc'."
    sys.exit(1)

logger = logging.getLogger("pybattery")
handler = logging.FileHandler("/tmp/pybattery.log")
formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
logger.setLevel(logging.DEBUG)
handler.setLevel(logging.DEBUG)
handler.setFormatter(formatter)
logger.addHandler(handler)
debug = logger.debug

class MainWin(QtGui.QDialog):
    def __init__(self, parent=None):
        super(MainWin, self).__init__(None)

        self.power = power.Power(self.onEvent)

        self.timer = QtCore.QTimer()
        self.timer.setInterval(500)
        self.timer.setSingleShot(False)
        self.connect(self.timer, QtCore.SIGNAL("timeout()"), self.iconFlickerSlot)

        # define ourself signals
        self.connect(self, QtCore.SIGNAL("startWarning_"), self.startWarningSlot)
        self.connect(self, QtCore.SIGNAL("stopWarning_"), self.stopWarningSlot)
        self.connect(self, QtCore.SIGNAL("onEvent_"), self.onEventSlot)

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
        debug("Start UI")

    def iconFlickerSlot(self):
        if not self.trayIcon.icon().isNull():
            self.trayIcon.setIcon(QtGui.QIcon())
        else:
            self.trayIcon.setIcon(QtGui.QIcon(self.currTrayIconPic))

    def stopWarningSlot(self, *args, **kwargs):
        debug("Stop warning")
        self.timer.stop()
        self.trayIcon.setIcon(QtGui.QIcon(self.currTrayIconPic))

    def startWarningSlot(self, *args, **kwargs):
        debug("Start warning")
        if self.trayIcon.supportsMessages():
            self.trayIcon.showMessage(args[0], args[1])
        self.timer.start()

    def onEventSlot(self, *args, **kwargs):
        capacity = self.power.getBattCap()
        charging = self.power.isCharging()
        self.progressBar.setValue(capacity)
        debug("onEvent: cap=%d charging=%d" % (capacity, charging))

        if charging:
            self.currTrayIconPic = self.trayIconPicCharge[str((capacity+19)/20*20)]
        else:
            self.currTrayIconPic = self.trayIconPicDisCharge[str((capacity+19)/20*20)]
        self.trayIcon.setIcon(QtGui.QIcon(self.currTrayIconPic))

        if capacity <= self.warnValue and not charging:
            if not self.timer.isActive():
                self.emit(QtCore.SIGNAL("startWarning_"), "Warning", "Battery Capacity too low !")
        else:
            if self.timer.isActive():
                self.emit(QtCore.SIGNAL("stopWarning_"))

    # not UI thread, we cannt do any UI operation here
    def onEvent(self):
        self.emit(QtCore.SIGNAL("onEvent_"))

    def trayIconActivated(self, reason):
        if reason in (QtGui.QSystemTrayIcon.Trigger, QtGui.QSystemTrayIcon.DoubleClick):
            if self.isHidden():
                self.show()
            else:
                self.hide()

    def setWarning(self, value):
        self.warnValue = value
        self.onEvent()

    def changeStyle(self, styleName):
        QtGui.QApplication.setStyle(QtGui.QStyleFactory.create(styleName))

    def __suspendToMode(self, mode):
        try:
            with open("/sys/power/state", "r+") as f:
                f.write(mode)
        except IOError:
            debug("Failed to suspend to %s" % mode)

    def systemSleep(self):
        debug("Suspend to mem")
        self.__suspendToMode("mem")

    def systemHibernate(self):
        debug("Suspend to disk")
        self.__suspendToMode("disk")

    def runSubProcess(self, *args, **kwargs):
        os.execvp(args[0], args)

    def runCmd(self, cmd):
        from multiprocessing import Process
        proc = Process(target=self.runSubProcess, args=cmd)
        proc.start()

    def systemShutdown(self):
        debug("Shutdown")
        self.runCmd(("poweroff", "-h", "-i"))

    def systemReboot(self):
        debug("Reboot")
        self.runCmd(("reboot",))

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

        keys = ["%d" % i for i in range(0, 101, 20)]
        discharge_pics = [":/images/battery-%s.png" % i.zfill(3) for i in keys]
        charge_pics = [":/images/battery-charging-%s.png" % i.zfill(3) for i in keys]
        self.trayIconPicDisCharge = dict(zip(keys, discharge_pics))
        self.trayIconPicCharge = dict(zip(keys, charge_pics))
        self.currTrayIconPic = discharge_pics[-1]
        self.trayIcon = QtGui.QSystemTrayIcon(QtGui.QIcon(self.currTrayIconPic), self)
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
#    ex.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
