#!/usr/bin/env python

import os
from optparse import OptionParser


def suspend(mode):
	path = "/sys/power/state"
	try:
		with open(path, "r+") as f:
			s = f.readline()
		if mode in s:
			f.write(mode)
	except IOError:
		print "Failed to operate '%s'" % path

def poweroff():
	cmd = ["poweroff", "-h", "-i"]
	os.execvp(cmd[0], cmd)


parser = OptionParser()
parser.add_option("-m", "--mode", dest="mode",
		help="set computer to a mode", default="mem")

(options, args) = parser.parse_args()


mode = options.mode

if mode == "poweroff":
	poweroff()
else:
	suspend(mode)

