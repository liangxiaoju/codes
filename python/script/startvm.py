#!/usr/bin/env python

import os

from optparse import OptionParser

parser = OptionParser()
parser.add_option("-m", "--machine", dest="machine",
		default="winxp", help="start machine")

(options, args) = parser.parse_args()

machine = options.machine

cmd = ["VBoxManage", "startvm"] + [machine,]

os.execvp(cmd[0], cmd)

