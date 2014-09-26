#/bin/bash
while ((1))
do
	date >> weightreport.txt;
	awk '{ if ($1=="server") print $2,$3 }' /SmartGrid/smartl7/conf/smartl7.conf >> weightreport.txt;
	awk '{ if ($1=="weight" || $1=="real_server") print $0 }' /SmartGrid/keepalived/etc/check.conf >> weightreport.txt;
	sleep 60;
done
