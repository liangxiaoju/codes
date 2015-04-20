#!/bin/bash

./obj-localize-symbol.sh -i $1 -o "obj1"
./obj-globalize-symbol.sh -i "obj1" -o "obj2" -s ./globalize-syms
./obj-redefine-symbol.sh -i "obj2" -o $2 -s ./redefine-syms

rm obj[1-2]
