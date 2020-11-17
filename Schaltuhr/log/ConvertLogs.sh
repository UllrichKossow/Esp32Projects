#!/bin/bash
n=0
for f in log*.txt; do
	case $n in
		0)
		newname=on-1.txt
		;;
		1)
		newname=off-1.txt
		;;
		2)
		newname=on-2.txt
		;;
		3)
		newname=off-2.txt
		;;
		4)
		newname=on-3.txt
		;;
		5)
		newname=off-3.txt
		;;
	esac
	n=$(($n + 1))
        if [ -f $newname ]; then
            echo $newname
        fi
#	mv $f $newname	
done
for f in o*.txt; do
	grep Code <$f |sed "s/a0/\na0/g"> code-$f
done
