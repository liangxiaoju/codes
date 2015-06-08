#!/usr/bin/env python2.7

import os,sys
import re

cmd="adb"
for arg in sys.argv[1:]:
    cmd += " " + arg

err = os.system(cmd)

if err == 0:
    sys.exit(0)
elif len(sys.argv) == 1:
    sys.exit(err)

with os.popen("adb devices") as f:
    strings = f.read()

dlists = []
for sub in re.finditer(r".*\t.*", strings):
    dlists.append(sub.group().split("\t"))

for i,d in enumerate(dlists):
    print "#%d: %s" % (i, d[0])

serialno = None
if len(dlists) > 1:
    try:
        prompt = "Choice:(0-%d) " % (len(dlists)-1)
        choice = input(prompt)
    except:
        sys.exit(1)
    serialno = dlists[choice][0]

if serialno:
    cmd = ["adb", "-s", serialno]
    cmd += sys.argv[1:]
    os.execvp(cmd[0], cmd)
