#!/usr/bin/env python

from optparse import OptionParser
import json
import sys, os
import re

def findKey(filepath):
    lines = ""

    with open(filepath) as f:
        lines = f.readlines()

    key = []
    for line in lines:
        m = re.findall(r'TR\("([^"]*)"\)', line)
        key += m

    return key

def jsonify(keys):
    d = {}
    for key in keys:
        d[key] = ""

    return json.dumps(d, indent=2, sort_keys=True)

def main():
    parser = OptionParser()
    parser.add_option("-d", "--dir", dest="dir",
            help="source file directory")
    parser.add_option("-o", "--output", dest="output",
            help="output json file path")

    (options, args) = parser.parse_args()

    directory = options.dir
    output = options.output

    keys = []
    for root, dirs, files in os.walk(directory):
        for name in files:
            filepath = os.path.join(root, name)
            keys += findKey(filepath)

    json = jsonify(keys)

    with open(output, "w+") as f:
        f.write(json)

if __name__ == "__main__":
    main()
