#!/usr/bin/env python

import os
from optparse import OptionParser

table = (
        (0, 0.03, 0),
        (1500, 0.10, 105),
        (4500, 0.20, 555),
        (9000, 0.25, 1005),
        (35000, 0.30, 2755),
        (55000, 0.35, 5505),
        (80000, 0.45, 13505),
        )

#tax = (total_income - social_security - housing_fund - tax_free) * ratio - base

def main():
    parser = OptionParser()
    parser.add_option("-t", "--total", dest="total", type="float",
            help="total income")
    parser.add_option("-s", "--social", dest="social", type="float",
            help="social security fee")
    parser.add_option("-u", "--housing", dest="housing", type="float",
            help="housing fund fee")
    parser.add_option("-n", "--noTax", dest="noTax", type="float",
            default=3500, help="tax start point")
    (option, args) = parser.parse_args()

    value = option.total - option.social - option.housing - option.noTax

    for i, item in enumerate(table):
        if value > item[0]:
            continue
        else:
            param = table[i-1]
            break

    tax = value * param[1] - param[2]

    print "+++"
    print "Total:", option.total
    print "Tax:", tax
    print "Real:", option.total - option.social - option.housing - tax
    print "---"

if __name__ == "__main__":
    main()
