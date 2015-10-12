#!/system/bin/sh

target=$1

blksz=512k
[ -d "/tmp" ] && tmpdir=/tmp || tmpdir=/dev

strings='\x00\x00 \xff\xff \x55\xaa \xaa\x55'

DD="busybox dd"

usage()
{
    echo "Usage:"
    echo -e "\t$0 device\n"
    echo -en "\t"
    echo -E "Write [$strings] to device sequentially."
    exit 0
}

write_target()
{
    str=$1
    echo "generate image."
    while true; do
        echo -en $str$str$str$str$str$str$str$str;
        if [ $? -gt 0 ]; then break; fi
    done | $DD ibs=16 obs=$blksz 2>/dev/null | $DD bs=$blksz count=1 of=$tmpdir/$blksz 2>/dev/null

    echo "start write."
    while true; do
        $DD if=$tmpdir/$blksz bs=$blksz 2>/dev/null;
        if [ $? -gt 0 ]; then break; fi
    done | $DD bs=$blksz of=$target conv=fsync

    echo "finish write."
    rm -f $tmpdir/$blksz
    sync
}

[ $# -eq 0 ] && usage

while true; do
    for string in $strings; do
        echo -E "Writing '$string' ..."
        write_target $string
        echo "Done"
    done
done

