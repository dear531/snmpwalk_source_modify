#/bin/bash
while ((1))
do
		snmpwalk -v 2c -c public 192.168.12.95 .1.3.6.1.2.1.25.2.3.1.2;
		snmpwalk -v 2c -c public 192.168.12.95 .1.3.6.1.2.1.25.2.3.1.4;
		snmpwalk -v 2c -c public 192.168.12.95 .1.3.6.1.2.1.25.2.3.1.5;
		snmpwalk -v 2c -c public 192.168.12.95 .1.3.6.1.2.1.25.2.3.1.6;
		snmpwalk -v 2c -c public 192.168.12.95 .1.3.6.1.2.1.25.3.3.1.2;
		sleep 1;
done
