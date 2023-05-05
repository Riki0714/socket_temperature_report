#!/bin/bash

mkfifo mylist

exec 2<>mylist

thread=2
for((i=0; i <= $thread; i++)); do
	echo >mylist
done

for i in `seq 1 2`; do
	read <mylist
	{
		make client
#		echo "hello"
#		echo $i
#		sleep 2
#		echo "back"

		echo >mylist
	} &
done

wait
echo "you have"

exec 2<&-
exec 2>&-

rm -f mylist
