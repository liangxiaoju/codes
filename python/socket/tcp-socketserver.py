#!/usr/bin/env python2.7

from SocketServer import *

class Handler(StreamRequestHandler):

    def handle(self):
        addr = self.request.getpeername()
        print "connection from", addr
        self.wfile.write("Hello!")

#server = ForkingTCPServer(("", 60000), Handler)
server = ThreadingTCPServer(("", 60000), Handler)
server.serve_forever()
