#!/usr/bin/python

import socket

# create tcp/ip socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# associate the socket with the server address
server_address = ('localhost', 60000)
sock.bind((server_address))

print "server ", server_address

# listen for incoming connections
# listern() puts the socket into server mode
sock.listen(1)

# accept() wait for a connection
while True:
    print "waiting for connection"
    connection, client_address = sock.accept()
    try:
        print "connection from", client_address

        # recv data
        while True:
            data = connection.recv(8)
            print "received '%s'" % data
            if data:
                print "sending back"
                connection.sendall(data)
            else:
                print "no more data from", client_address
                break

    finally:
        connection.close()

