#!/usr/bin/env python

from timer import Timer
from uevent import UEvent
import os

class PowerSupply:
    class Info:
        pass

    def __init__(self, path, onChange, args=[], kwargs={}):
        self.timer = Timer(10, self.__update, False, args, kwargs)
        self.uevent = UEvent(path, self.__update, args, kwargs)
        self.onChange = onChange
        self.path = path
        self.info = self.Info()

    def start_monitor(self):
        self.timer.start()
        self.uevent.start()

    def __update(self, *args, **kwargs):
        if callable(self.onChange):
            self.onChange(*args, **kwargs)

    def __read_sysfs(self, filename):
        try:
            with open(self.path + "/" + filename, "r") as f:
                s = f.read()
        except:
            s = None
        return s

    def getInfo(self):
        info = self.info
        info.type = self.__read_sysfs("type")
        info.uevent = self.__read_sysfs("uevent")
        info.capacity = self.__read_sysfs("capacity")
        return info

    def stop_monitor(self):
        self.timer.stop(True)
        self.uevent.stop()


class Power:
    def __init__(self, observer=None, args=[], kwargs={}):
        self.observer = observer
        self.__findPsy()
        self.psylist =[PowerSupply(path, self.onEvent, args, kwargs) for path in self.paths]

    def __findPsy(self):
        rootdir = "/sys/class/power_supply"
        names = os.listdir(rootdir)
        join = os.path.join
        self.paths = [join(rootdir, name) for name in names
                      if os.path.isdir(join(rootdir, name))]
        #print "There are %s PSYs." % len(self.paths)
        #for path in self.paths:
        #    print path

    def onEvent(self, *args, **kwargs):
        if callable(self.observer):
            self.observer(*args, **kwargs)

    def getPsyNum(self):
        return len(self.paths)

    def getInfoByName(self, name):
        for psy in self.psylist:
            info = psy.getInfo()
            if info.name == name:
                return info

    def getInfoByNum(self, num):
        try:
            info = self.psylist[num].getInfo()
        except:
            info = None
        return info

    def getBattCap(self):
        for psy in self.psylist:
            info = psy.getInfo()
            if info.capacity != None:
                return int(info.capacity)

    def setObserver(self, observer):
        self.observer = observer

    def setWarning(self, value):
        pass

    def start(self):
        for psy in self.psylist:
            psy.start_monitor()

    def stop(self):
        for psy in self.psylist:
            psy.stop_monitor()
