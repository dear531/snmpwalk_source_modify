#/bin/bash
i=0
rm -rf curl.log
date >> report.txt
while ((i < 5000))
do
	curl http://192.168.12.78/ >> curl.log
	((i++));
	echo $i;
done
echo "http://192.168.12.78:80/" >> report.txt
echo "zyy://192.168.12.95" >> report.txt
cat curl.log |awk '{if ($1 == "zyy://192.168.12.95") print $1}' |wc -l >> report.txt 
echo "using" >> report.txt
cat curl.log |awk '{if ($1 == "using") print $1}' |wc -l >> report.txt
echo "zml" >> report.txt
cat curl.log |awk '{if ($1 == "zml") print $1}' |wc -l >> report.txt
date >> report.txt
echo "====" >> report.txt

