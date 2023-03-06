#!/bin/bash

mkdir mydir

chmod 777 ./mydir

echo 2023 >myfile

mv ./moveme ./mydir

cp ./copyme ./mydir
mv ./mydir/copyme ./mydir/copied

cat ./readme

gcc bad.c 2>err.txt

mkdir ./gen
n=10

if [ $# -ge 1 ]
then
	n=$1
fi

i=1
while [ $i -le $n ]
do
	>gen/$i.txt
	i=$((i+1))
done




