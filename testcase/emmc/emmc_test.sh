#!/bin/sh

#set -x
#set -e

read_failed_count=0
write_failed_count=0
differ_count=0
total=10

read_failed () {
    echo "[Read Failed] $@"
    read_failed_count=$(($read_failed_count+1))
}

write_failed () {
    echo "[Write Failed] $@"
    write_failed_count=$(($write_failed_count+1))
}

differ () {
    echo "[Differ] $@"
    differ_count=$(($differ_count+1))
}

summary () {
    echo "## summary ##"
    echo "total: $total"
    echo "read failed: $read_failed_count"
    echo "write failed: $write_failed_count"
    echo "differ: $differ_count"
}

count=1
while [ $count -le $total ]; do

    for mtd_dev in $(cat /proc/mtd | awk -F ":" '/mtd/ { print $1 }'); do

        info_path="/sys/devices/virtual/mtd/${mtd_dev}"
        dev_path="/dev/${mtd_dev}"

        name=$(cat "${info_path}/name")
        size=$(($(cat "${info_path}/size")))
        erasesize=$(($(cat "${info_path}/erasesize")))

        echo "$mtd_dev"
        num=256
        dd if=/dev/urandom of=/tmp/idat bs=$erasesize count=$num 2&>/dev/null

        for i in $(seq 0 $(($size/($erasesize*$num)-1))); do
            dd if=/tmp/idat of=$dev_path bs=$erasesize count=$num seek=$i 2&>/dev/null || write_failed "$count $name $i"
            dd if=$dev_path of=/tmp/odat bs=$erasesize count=$num skip=$i 2&>/dev/null || read_failed "$count $name $i"
            sync
            diff /tmp/idat /tmp/odat 2&>/dev/null || differ "differ: $count $name $i"
        done

    done

    count=$(($count+1))

done

summary

