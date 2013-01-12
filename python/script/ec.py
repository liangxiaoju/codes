#!/usr/bin/env python

import sys
import os

cmd = ["emacsclient", "-a", "", "-c", "-n"]
cmd += sys.argv[1:]

os.execvp(cmd[0], cmd)
