#!/bin/sh

set -e 
export PATH="./bin:$PATH"

case $1 in
    rk2928)
        parameter=~/imt/tools/rkflash/rk2928.par
        shift 1;;
    rk3066)
        parameter=~/imt/tools/rkflash/rk3066.par
        shift 1;;
    rk3188)
        parameter=~/imt/tools/rkflash/rk3188.par
        shift 1;;
    rk3288)
        parameter=~/imt/tools/rkflash/rk3288.par
        shift 1;;
    *)
        parameter=~/imt/tools/rkflash/rk3188.par
esac


size()
{
	file=$1

	len=$(stat $file | awk '/Size/{print $2}')
	echo $(($len/512 + 1))
}


die()
{
	echo $@
	exit 1
}

targets="boot system"

if [ $# -gt 0 ]; then
	targets="$@"
fi

if [ $# -eq 2 ] && [ -r $2 ]; then
	targets="$1"
	file_override="$2"
fi

case $1 in
	-b)
		rk_flash -b
		exit 0
		;;
esac

[ -e $parameter ] || die "$parameter NOT exist"

while read section offset file; do
	[[ $section = \#* ]] && continue;	# sh NOT compatible!!

	for target in $targets; do
		if [ $target = $section ]; then

			[ -n "$file_override" ] && file=$file_override

			[ -r $file ] || die "$file NOT readable"

			echo "flashing $section : $file"
			rk_flash -w $offset $(size $file) $file
		fi
	done
done < $parameter

