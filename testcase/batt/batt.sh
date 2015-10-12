
cd /sys/devices/platform/s3c2440-i2c.0/i2c-0/0-0034/axp192-batt-coulomb.9/pmic-batt/

echo "${1}" > adc_interval

echo "interval	volt_a	volt_b	current	capacity"

while :
do
	echo -n $(cat adc_interval) "	" $(cat volt_a) "	" $(cat volt_b) "	" $(cat curr) "	" $(cat cap)
	echo
#	sleep 1
done
