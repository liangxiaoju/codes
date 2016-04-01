#!/bin/bash

for p in $1/* ; do
	src=$p
	dst=${p%.*}.png

	echo "$src --> $dst"
	convert $src $dst
done
