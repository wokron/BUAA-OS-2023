#!/bin/bash

IFS_OLD=$IFS
IFS=$'\n'

i=1
t=8

>$2

for line in `cat $1`
do
	if [ $i -eq $t ]
	then
		echo $line >> $2
		t=$((t*4))
	fi

	if [ $i -eq 1024 ]
	then
		echo $line >> $2
		break
	fi

	i=$((i+1))
done
IFS=$IFS_OLD

