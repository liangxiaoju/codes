#!/usr/bin/env python

from timer import Timer
from uevent import UEvent
import os

class PowerSupply:
    def __init__(self, path, onChange, args=[], kwargs={}):
        self.timer = Timer(10, self.__update, False, args, kwargs)
        self.uevent = UEvent(path, self.__update, args, kwargs)
        self.onChange = onChange
        self.path = path
        self.info = {}

    def __update(self, *args, **kwargs):
        if callable(self.onChange):
            self.onChange(*args, **kwargs)

    def __read_sysfs(self, filename):
        try:
            with open(self.path + "/" + filename, "r") as f:
                s = f.read()
        except:
             s = False
        if s[-1] == "\n":
            s = s[:-1]
        return s

    def getInfo(self):
        uevent = self.__read_sysfs("uevent")
        if not uevent:
            return {}
        uevent = uevent.splitlines()
        index1 = len("POWER_SUPPLY_")
        for i in range(0, len(uevent)):
            index2 = uevent[i].rfind("=")
            self.info[uevent[i][index1:index2].lower()] = uevent[i][index2+1:]
        self.info["type"] = self.__read_sysfs("type")
        return self.info

    def getValue(self, attr):
        return self.__read_sysfs(attr)

    def start_monitor(self):
        self.timer.start()
        self.uevent.start()

    def stop_monitor(self):
        self.timer.stop(True)
        self.uevent.stop()


class Power:
    def __init__(self, observer=None, args=[], kwargs={}):
        self.observer = observer
        rootdir = "/sys/class/power_supply"
        try:
            names = os.listdir(rootdir)
        except:
            raise Exception("Please add PowerSupply support in kernel.")
        self.names = names
        join = os.path.join
        paths = [join(rootdir, name) for name in names
                      if os.path.isdir(join(rootdir, name))]
        psylist =[PowerSupply(path, self.onEvent, args, kwargs)
                       for path in paths]
        self.psydict = dict(zip(names, psylist))

    def onEvent(self, *args, **kwargs):
        if callable(self.observer):
            self.observer(*args, **kwargs)

    def getPsyNum(self):
        return len(self.psydict)

    def getInfoByName(self, name):
        if self.psydict.has_key(name):
            return self.psydict[name].getInfo()
        return {}

    def getInfoByType(self, infoType):
        for psy in self.psydict.values():
            info = psy.getInfo()
            if info.has_key("type") and info["type"] == infoType:
                return info
        return {}

    def getBattCap(self):
        capacity = 0
        for psy in self.psydict.values():
            info = psy.getInfo()
            if info.has_key("capacity"):
                capacity = int(info["capacity"])
                break
        if capacity > 100:
            capacity = 100
        elif capacity < 0:
            capacity = 0
        return capacity

    def isCharging(self):
        for psy in self.psydict.values():
            info = psy.getInfo()
            if info.has_key("status"):
                if info["status"] == "Charging":
                    return True
        return False

    def setObserver(self, observer):
        self.observer = observer

    def start(self):
        for psy in self.psydict.values():
            psy.start_monitor()

    def stop(self):
        for psy in self.psydict.values():
            psy.stop_monitor()
