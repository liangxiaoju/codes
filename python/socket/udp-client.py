#!/usr/bin/env python

import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_address = ('localhost', 60000)

message = "message"

try:
    print "sending '%s'" % message
    sock.sendto(message, server_address)

    print "waiting for response"
    data, address = sock.recvfrom(16)
    print "received '%s' from %s" % (data, address)

finally:
    print "close"
    sock.close()

