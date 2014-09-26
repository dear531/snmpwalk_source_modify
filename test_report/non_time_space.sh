#/bin/bash
while ((1))
do
	curl http://192.168.12.78/ >> /dev/null
	curl http://192.168.12.78:81/ >> /dev/null
	wget http://192.168.12.78:81/ >> /dev/null
done
