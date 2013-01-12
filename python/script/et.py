#!/usr/bin/env python

import sys
import os

cmd = ["emacsclient", "-a", "", "-t"]
cmd += sys.argv[1:]

os.execvp(cmd[0], cmd)
