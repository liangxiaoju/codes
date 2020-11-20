#!/usr/bin/env python

from optparse import OptionParser
from Crypto.Cipher import ARC4

def crypt(buf, key):
    arc4 = ARC4.new(key)
    enc = arc4.encrypt(buf)
    return enc

def main():
    parser = OptionParser()
    parser.add_option("-k", "--keyfile", dest="keyfile", help="key file")
    parser.add_option("-K", "--keystr", dest="keystr", help="key string")
    parser.add_option("-i", "--input", dest="input", help="input file")
    parser.add_option("-o", "--output", dest="output", help="output file")

    (options, args) = parser.parse_args()

    key = None
    if options.keyfile:
        with open(options.keyfile) as f:
            key = f.read()
    elif options.keystr:
        key = options.keystr

    if not key:
        raise Exception("Must provide keyfile or keystr")

    with open(options.input) as f:
        source = f.read()

    with open(options.output, "w+") as f:
        f.write(crypt(source, key))

if __name__ == "__main__":
    main()
