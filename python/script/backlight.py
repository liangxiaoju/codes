#!/usr/bin/env python

import os
import sys

def usage():
    print "Usage:", sys.argv[0], "brightness"

if len(sys.argv) != 2:
    usage()
    sys.exit(1)

brightness = sys.argv[1]
rootdir = "/sys/class/backlight"
names = os.listdir(rootdir)

for name in names:
    path = os.path.join(rootdir, name)
    if os.path.isdir(path):
        break

path += "/brightness"

with open(path, "r+") as f:
    f.write(brightness)
