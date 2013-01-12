#!/usr/bin/env python

import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_address = ('', 60000)
sock.bind(server_address)

print "start server", server_address

while True:
    print "waiting for message"
    (data, client_address) = sock.recvfrom(16)
    print "received '%s' from %s" % (data, client_address)

    print "sending back"
    sock.sendto(data, client_address)

