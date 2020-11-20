
ARM=$(grep "ARM" /proc/cpuinfo)

MYNAME=$0

if [ -z "${ARM}" ]; then
	# X86 PC

	adb wait-for-device
	adb shell stop
	adb shell sync
	sleep 5
	adb push $MYNAME /data/
	adb shell /data/$MYNAME | tee ./mmc-speed_"$(date "+%F_%H-%M-%S")".txt

	exit 0
fi

target=${1:-"/data/dat"}

alias dd="busybox dd"

[ -d "/tmp" ] && testdat=/tmp/dat || testdat=/dev/dat

count=${2:-20}

blkszs="$((128*1024)) $((256*1024)) $((512*1024)) $((1*1024*1024)) $((2*1024*1024)) $((4*1024*1024)) $((8*1024*1024)) $((16*1024*1024)) $((32*1024*1024))"

usage()
{
    echo "Usage:"
    echo -e "\t$0 [device] [count]\n"
    exit 0
}

write_disk()
{
	blksz=$1
	i=1
	while [ $i -le $count ]; do
		dd if=/dev/urandom of=$testdat bs=$blksz count=1 2>/dev/null
		sync
		dd if=$testdat of=$target bs=$blksz conv=fsync 2>&1 | grep "copied"
		sync
		i=$(($i+1))
	done
	rm -f $testdat
}

read_disk()
{
	blksz=$1
	i=1
	while [ $i -le $count ]; do
		sync
		echo 3 > /proc/sys/vm/drop_caches
		sync
		dd if=$target of=/dev/null bs=$blksz count=1 2>&1 | grep "copied"
		sync
		i=$(($i+1))
	done
}

[ "$1" = "-h" ] && usage

echo -e "\n*** Disk Speed Test ***"
echo -e "$(date "+%F %T")"

echo -e "\nWrite ..."
for blksz in $blkszs; do
	echo "=== $(($blksz/1024))K ==="
	write_disk $blksz
done

echo -e "\nRead ..."
for blksz in $blkszs; do
	echo "=== $(($blksz/1024))K ==="
	read_disk $blksz
done

rm -f $target

