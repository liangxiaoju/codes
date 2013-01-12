#!/usr/bin/env python

import socket

# create a tcp/ip socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server_address = ('localhost', 60000)

print "connecting to", server_address

#connect the socket to the port where the server is listening
sock.connect(server_address)

try:
    # send data
    message = "message " * 5
    print "sending '%s'" % message
    sock.sendall(message)

    # look for response
    amount_received = 0
    amount_expected = len(message)

    while amount_received < amount_expected:
        data = sock.recv(16)
        amount_received += len(data)
        print "received '%s'" % data

finally:
    print "close"
    sock.close()

