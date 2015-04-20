#!/bin/bash

for file in modules/*.so; do
	adb push $file /tmp/$file
done
adb push win /tmp
