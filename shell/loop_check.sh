#/bin/bash

while ((1))
do
	ps aux | grep walk4rs >> loop_check.log;
	sleep 1;
	echo "----" >> loop_check.log
done
