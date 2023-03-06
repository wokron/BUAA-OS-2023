#!/bin/bash

if [ $# -eq 1 ];
then
    # Your code here. (1/4)
	grep -E "WARN|ERROR" $1 >bug.txt
else
    case $2 in
    "--latest")
        # Your code here. (2/4)
	echo `tail $1 -n 5`
    ;;
    "--find")
    	>$3.txt
        # Your code here. (3/4)
	for file in `find . -maxdepth 1`
	do
		echo $file
		grep $3 $file >>$3.txt	
	done
    ;;
    "--diff")
        # Your code here. (4/4)

    ;;
    esac
fi
