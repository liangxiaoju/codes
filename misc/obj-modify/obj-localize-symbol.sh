#!/bin/bash

readelf="arm-boeye-linux-gnueabi-readelf"
objcopy="arm-boeye-linux-gnueabi-objcopy"

input=
output=

usage ()
{
    echo "Usage: $0 -i input -o output"
    exit 2
}

while getopts i:o: option
do
    case $option in
    i)
        input="$OPTARG";;
    o)
        output="$OPTARG";;
    ?)
        usage;;
    esac
done

[[ -z "$input" || -z "$output" ]] && usage

symsfile="$(mktemp)"

$readelf -W -s $input | awk '{ if (($4=="FUNC" || $4=="OBJECT") && $5=="GLOBAL" && $7!="UND") print $8 }' > $symsfile

$objcopy --localize-symbols=$symsfile $input $output

rm $symsfile
