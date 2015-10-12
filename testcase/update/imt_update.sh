#!/system/bin/sh

updateimg=/mnt/update/update.img

echo "imt_update start"

ts1=$(date +%s)
[ -e "$updateimg" ] && update $updateimg
ts2=$(date +%s)

echo "imt_update end"

echo "Eclipse $(($ts2 - $ts1)) seconds"

