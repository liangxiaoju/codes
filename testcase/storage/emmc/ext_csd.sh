#!/bin/bash

usage()
{
	echo "$0 POSITION"
	exit 1
}

[[ $# -ne 1 ]] && usage

n=$1

adb wait-for-device

kernel_version=$(adb shell cat /proc/version | awk '{ print $3 }' | awk -F "-" '{ print $1 }')

# 3.4.0 print from 511 to 0
if [ "$kernel_version" = "3.4.0" ]; then
	s=$(((511-$n)*2+1))
	e=$(($s+1))
else
	s=$(($n*2+1))
	e=$(($s+1))
fi

ext_csd=$(adb shell cat /d/mmc0/mmc0:*/ext_csd)

echo $ext_csd | cut -b "$s-$e"

