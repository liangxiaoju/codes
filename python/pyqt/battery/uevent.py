"""netlink to communicate with kernel
"""

from threading import Thread
import socket
import select

class UEvent():
    def __init__(self, path, onUEvent, args=[], kwargs={}):
        self.path = path
        self.onUEvent = onUEvent
        self.args = args
        self.kwargs = kwargs

        # NETLINK_KOBJECT_UEVENT is 15
        self.sock = socket.socket(socket.AF_NETLINK, socket.SOCK_RAW, 15)
        self.sock.setblocking(False)
        # AF_NETLINK sockets are represented as pairs (pid, groups)
        # set pid=0 to let kernel assign it
        # pid is the unicast address of netlink socket
        # groups is a bit mask with every bit representing a netlink group number
        addr = (0, 0xffff)
        self.sock.bind(addr)

        self.sockpair = socket.socketpair(socket.AF_UNIX, socket.SOCK_STREAM, 0)

        self.finished = False

    def __filter(self, msg, address):
        msg_match = str(msg).find(self.path) >= 0
        addr_match = address[0] == 0
        if msg_match and addr_match:
            return True
        else:
            return False

    def __start(self):
        while not self.finished:
            (rlist, wlist, xlist) = select.select([self.sock, self.sockpair[0]], [], [])
            if self.sock in rlist:
                try:
                    (msg, address) = self.sock.recvfrom(1024)
                    if msg and self.__filter(msg, address):
                        #print "received '%s' from %s" % (msg, address)
                        if callable(self.onUEvent):
                            self.onUEvent(*self.args, **self.kwargs)
                except KeyboardInterrupt:
                    self.finished = True
                except:
                    #print "no data"
                    continue
            elif self.sockpair[0] in rlist:
                #print "stop uevent"
                pass
            else:
                #print "unknown event"
                pass
        else:
            self.sock.close()

    def start(self):
        self.thread = Thread(target=self.__start)
        self.thread.start()

    def stop(self):
        self.finished = True
        self.sockpair[1].send("anything")

def print_uevent():
    print "uevent"

def test():
    e = UEvent("/", print_uevent)
    e.start()
    while raw_input() != "q":
        pass
    e.stop()

if __name__ == "__main__":
    test()
