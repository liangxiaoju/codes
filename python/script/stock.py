#!/usr/bin/python

import sys
from time import sleep
import ystockquote

def main(args):
    if len(args) == 0:
        print("Usage: %s stock_name" % sys.argv[0])
        return

    symbol = args[0]

    while True:
        try:
            data = ystockquote.get_all(symbol)
        except KeyboardInterrupt:
            print("KeyboardInterrupt")
            break
        except:
            #print("Exception")
            pass
        else:
            print("p(%.0f/100)v(%s)c(%s)" %
                    ((float(data["price"])*100), data["volume"], data["change"]))
            print("")

        sleep(10)

if __name__ == "__main__":
    main(sys.argv[1:])
