#!/bin/bash

usage ()
{
	echo "head.sh line keyword file"
	exit 1
}

[[ $# -eq 3 ]] && line=$1 && keyword=$2 && file=$3 || usage

awk 'NR<='${line}' && /'${keyword}'/ { printf "%d %s\n", NR, $0 }' ${file}

exit 0
