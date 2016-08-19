#!/usr/bin/env python

import os
from optparse import OptionParser
import SimpleHTTPServer

parser = OptionParser()
parser.add_option("-d", "--dir", dest="dir",
        help="serve dir for http", default=os.getcwd())

(options, args) = parser.parse_args()

os.chdir(options.dir)

handler = SimpleHTTPServer.SimpleHTTPRequestHandler

httpd = SimpleHTTPServer.BaseHTTPServer.HTTPServer(("", 8000), handler)
sa = httpd.socket.getsockname()
print "Serving HTTP on", sa[0], "port", sa[1], "..."

httpd.serve_forever()

