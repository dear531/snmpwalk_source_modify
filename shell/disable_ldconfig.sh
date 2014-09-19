#!/bin/bash

echo "" > /etc/ld.so.conf.d/netsnmp.conf 
ldconfig
echo "/SmartGrid/snmp/lib/" > /etc/ld.so.conf.d/netsnmp.conf
