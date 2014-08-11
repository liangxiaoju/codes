#!/usr/bin/env python

import urllib2
import json
import time

def get_ticker():
    response = urllib2.urlopen("http://www.okcoin.com/api/ticker.do?symbol=ltc_cny")
    content = response.read()
    response.close()
    return json.loads(content)

def get_depth():
    response = urllib2.urlopen("http://www.okcoin.com/api/depth.do?symbol=ltc_cny")
    content = response.read()
    response.close()
    return json.loads(content)

def get_trades():
    response = urllib2.urlopen(
            "http://www.okcoin.com/api/trades.do?symbol=ltc_cny&since=10")
    content = response.read()
    response.close()
    return json.loads(content)

def dump():
    try:
        ticker = get_ticker()
        #depth = get_depth()
        #trades = get_trades()
    except:
        pass
    else:
        for k in ticker["ticker"]:
            print("%s:\t%s" % (k, ticker["ticker"][k]))

        print("----------------------")


while True:
    dump()
    time.sleep(8)
