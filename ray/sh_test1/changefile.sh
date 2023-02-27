#!/bin/bash
a=1

while [ $a -le 100 ]
do
	if [ $a -gt 70 ]	   #if loop variable is greater than 70
	then
		rm -r file$a 
	elif [ $a -gt 40 ]         # else if loop variable is great than 40
	then
		mv file$a newfile$a
	fi
	a=$((a+1))           #don't forget change the loop variable
done
