#!/bin/bash

objcopy="arm-boeye-linux-gnueabi-objcopy"

input=
output=

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

$objcopy --rename-section "___ksymtab+rockchip_wifi_exit_module"="___ksymtab+rtl8188eus_wifi_exit_module" $input "obj-rename"

$objcopy --rename-section "___ksymtab+rockchip_wifi_init_module"="___ksymtab+rtl8188eus_wifi_init_module" "obj-rename" $output

rm obj-rename
