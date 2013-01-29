#!/usr/bin/python

import os
from optparse import OptionParser
from pyftpdlib import ftpserver

parser = OptionParser()
parser.add_option("-d", "--dir", dest="dir",
        help="serve dir for ftp", default=os.getcwd())

(options, args) = parser.parse_args()

authorizer = ftpserver.DummyAuthorizer()
authorizer.add_anonymous(options.dir)

ftp_handler = ftpserver.FTPHandler
ftp_handler.authorizer = authorizer

ftpd = ftpserver.FTPServer(("0.0.0.0", 21), ftpserver.FTPHandler)
ftpd.serve_forever()
