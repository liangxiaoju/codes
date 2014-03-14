#!/usr/bin/env python2.7

from SocketServer import *

class Handler(DatagramRequestHandler):

    def handle(self):
        data, socket = self.request
        print "Got %s from %s" % (data, self.client_address)
        self.wfile.write("Hello!")


server = ThreadingUDPServer(("", 60000), Handler)
server.serve_forever()
