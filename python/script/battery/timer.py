"""A sample Timer
"""

from threading import Thread, Event

class Timer(Thread):
    def __init__(self, interval, function, oneshot=True, args=[], kwargs={}):
        super(Timer, self).__init__()
        self.interval = interval
        self.function = function
        self.args = args
        self.kwargs = kwargs
        self.onshot = oneshot
        self.finished = Event()

    def start(self):
        super(Timer, self).start()

    def run(self):
        while True:
            self.finished.wait(self.interval)
            if not self.finished.is_set():
                if callable(self.function):
                    self.function(*self.args, **self.kwargs)
            else:
                break
            if self.onshot:
                break
        self.finished.set()

    def stop(self, sync=0):
        self.finished.set()
        if sync and self.is_alive():
            self.join()

def timer_print(args):
    print args

def test():
    t = Timer(2, timer_print, True, ["OK",])
    t.start()
    Event().wait(3)
    t.stop()

if __name__ == "__main__":
    test()
