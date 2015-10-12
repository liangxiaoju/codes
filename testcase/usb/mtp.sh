#!/bin/bash

cd ${0%/*}

logfile=$(pwd)/log/mtp_"$(date "+%F_%H-%M-%S")".txt
mountpoint=$(pwd)/mtp

mkdir -p ${logfile%/*}
mkdir -p $mountpoint

if [ -z "`which mtpfs`" ]; then
	echo "Please:"
	echo "	1. install mtpfs"
	echo "	2. uncomment user_allow_other in /etc/fuse.conf"
	echo "	3. add yourself to fuse group"
	exit 0
fi

MTP()
{
	action=$1
	local err=0

	case $action in
		mount)
			echo "mounting mtpfs at $mountpoint"
			local count=10
			while true; do
				mtpfs -o allow_other,direct_io $mountpoint
				sync
				sleep 5
				if `ls $mountpoint &>/dev/null`; then
					break
				fi
				fusermount -u $mountpoint
				sync
				sleep 1

				count=$(($count-1))
				[[ $count -le 0 ]] && break
				echo "retry mount"
			done

			if [ $count -gt 0 ]; then
				echo "success"
				err=0
			else
				echo "fail"
				err=1
			fi
			;;
		unmount)
			echo "unmounting mtpfs at $mountpoint"
			sync
			sleep 5
			local count=10
			while ! `fusermount -u $mountpoint &>/dev/null`; do
				count=$(($count-1))
				[[ $count -le 0 ]] && break
				sync
				sleep 5
				echo "retry unmount"
			done

			if [ $count -gt 0 ]; then
				echo "success"
				err=0
			else
				echo "fail"
				err=1
			fi
			;;
		*)
			;;
	esac

	return $err
}

switch_to_mtp()
{
	adb wait-for-device
	adb shell 'while [ "$(getprop sys.boot_completed)" != "1" ]; do sleep 1; done'
	for sr in /dev/sr[1-9]; do
		[[ -e "$sr" ]] && eject $sr
	done
	sleep 10
	adb wait-for-device
	echo "switch to mtp mode"
	adb shell svc usb setFunction mtp,adb
	sleep 5
	adb wait-for-device

	MTP mount
	if [ $? -ne 0 ]; then
		MTP unmount
		result="Failed to switch to mtp mode"
		echo $result
		../tool/CONN/record.py \
			--team="CONN" \
			--module="USB" \
			--testcase="test for mtp" \
			--result="$result"
		exit 1
	fi

	MTP unmount
}

testcase_copy()
{
	echo "start testcase: copy test for mtp"
	for i in $(seq 20); do

		echo "===$i==="

		tmpdat=`mktemp -u`
		tmpdatname=${tmpdat##*/}
		tmpdatback=${tmpdat}.back

		echo "generate random data at $tmpdat"
		dd if=/dev/urandom of=$tmpdat bs=100M count=1 &>/dev/null

		MTP mount
		echo "copy PC:$tmpdat to MTP:$tmpdatname"
		cp $tmpdat $mountpoint/
		MTP unmount

		MTP mount
		echo "move MTP:$tmpdatname back to PC:$tmpdatback"
		mv $mountpoint/$tmpdatname $tmpdatback
		sync
		MTP unmount

		echo "check md5sum"
		sum1=`md5sum $tmpdat | awk '{ print $1 }'`
		sum2=`md5sum $tmpdatback | awk '{ print $1 }'`
		if [ "$sum1" = "$sum2" ]; then
			echo "md5sum PASS"
		else
			echo "md5sum FAIL"
		fi

		rm $tmpdat $tmpdatback

	done

	count_fail=`grep "FAIL" $logfile | wc -l`
	count_pass=`grep "PASS" $logfile | wc -l`

	echo "PASS: $count_pass FAIL: $count_fail"
	result="<font color=green>PASS: $count_pass</font><br><font color=red>FAIL: $count_fail</font>"

	../tool/CONN/record.py \
		--team="CONN" \
		--module="USB" \
		--testcase="copy test for mtp" \
		--description="copy file between PC and Phone<br>then compare their md5sum" \
		--result="$result" \
		--attachment="$logfile"
}

switch_to_mtp

testcase_copy 2>&1 | tee $logfile

