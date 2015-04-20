#!/bin/bash

objcopy="arm-boeye-linux-gnueabi-objcopy"

input=
output=

usage ()
{
    echo "Usage: $0 -i input -o output -s sym-file"
    exit 2
}

while getopts i:o:s: option
do
    case $option in
    i)
        input="$OPTARG";;
    o)
        output="$OPTARG";;
    s)
        sym="$OPTARG";;
    ?)
        usage;;
    esac
done

[[ -z "$input" || -z "$output" || -z "$sym" ]] && usage

$objcopy --redefine-syms=$sym $input $output
