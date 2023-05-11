#!/bin/bash

mkfifo mylist

exec 5<>mylist

thread=5
for((i=0; i <= $thread; i++)); do
	echo >mylist
done

for i in `seq 1 5`; do
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

exec 5<&-
exec 5>&-

rm -f mylist
