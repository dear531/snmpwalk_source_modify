#/bin/bash
j=0
while ((1))
do
	while((1))
	do
		second=`date "+%S"`;
		sleep 1;
		echo $second;
		if [ "$second" == "00" ];
		then
			break;
		fi
	done
	if [ $[j % 2] -eq 0 ];
	then
		./curl_tcp.sh
	else
		./curl.sh
	fi
done
