#!/bin/sh

BASE_DIR=`dirname $0`/bin
IMGDIR=Images
OUT=update.img
case $1 in
	DEBUG)
		
		BOOTLOADER=$(awk '$1=="bootloader" {print $2}' $IMGDIR/package-file-debug)
		BOOTLOADER=$(echo $BOOTLOADER | tr -d '\r')
		$BASE_DIR/afptool -pack $IMGDIR $OUT.tmp -debug 
		shift 1
		;;
	*)
		
		BOOTLOADER=$(awk '$1=="bootloader" {print $2}' $IMGDIR/package-file)
		BOOTLOADER=$(echo $BOOTLOADER | tr -d '\r')
		$BASE_DIR/afptool -pack $IMGDIR $OUT.tmp
		;;
esac


$BASE_DIR/img_maker $IMGDIR/$BOOTLOADER $OUT.tmp $OUT

rm $OUT.tmp
