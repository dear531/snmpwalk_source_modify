#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#define STANDARD_CPU_OID	".1.3.6.1.2.1.25.3.3.1.2"
#define STANDARD_MEM_TYPE	".1.3.6.1.2.1.25.2.3.1.2"
#define STANDARD_MEM_UNIT	".1.3.6.1.2.1.25.2.3.1.4"
#define STANDARD_MEM_SIZE	".1.3.6.1.2.1.25.2.3.1.5"

#define CPUOID	".1.3.6.1.4.1.2021.11.11.0"
#define MEMOID	".1.3.6.1.4.1.2021.4.11.0"

int get_data(char *oid, char* buff)
{
	int data = -1;
	int ret;
	if (0 == memcmp(oid, STANDARD_CPU_OID, sizeof(STANDARD_CPU_OID))) {
		sscanf(buff, "HOST-RESOURCES-MIB::hrProcessorLoad.%*d = INTEGER: %d", &data);
	} else if (0 == memcmp(oid, STANDARD_MEM_TYPE, sizeof(STANDARD_MEM_TYPE))
			&& NULL != strstr(buff, "hrStorageRam")) {
		sscanf(buff, "HOST-RESOURCES-MIB::hrStorageType.%d", &data);
		fprintf(stdout, "mem index :%d\n", data);
	}

	if (0 <= data) {
		ret = data;
	} else {
		ret = -1;
	}

	return ret;
}

int get_mem_type(char *command, int show_mod)
{
	char *oid = STANDARD_MEM_TYPE;
	char buf[1024] = {0};
	int walk_ret;
	int data_flag = 1;
	int ret;

	FILE * fp;
	fp = popen(command, "r");
	if (NULL == fp) {
		syslog(LOG_INFO, "popen error :%s\n",
				strerror(errno));
		ret = -1;
		goto finish;
	}

	while (NULL != fgets(buf, sizeof(buf), fp)) {
		if (0 == strncasecmp(buf, "USAGE:", strlen("USAGE:"))) {
			data_flag = 0;
		} else if (0 == strncasecmp(buf, "result:", strlen("result:"))) {
			sscanf(buf ,"result:%d", &walk_ret);
			break;
		}

	    if (1 != data_flag) {
			memset(buf, 0x00, strlen(buf));
			continue;
		}
		get_data(oid, buf);
	}

	if (NULL != fp)
		pclose(fp);
finish:

	return ret;
}

int get_cpu(char *command, int show_mod)
{
	char *oid = STANDARD_CPU_OID;
	char buf[1024] = {0};
	int walk_ret;
	int data = -1;
	int ret;
	int data_flag = 1;
	int counter = 0;
	int total = 0;

	FILE * fp;
	fp = popen(command, "r");
	if (NULL == fp) {
		syslog(LOG_INFO, "popen error :%s\n",
				strerror(errno));
		ret = -1;
		goto finish;
	}

	while (NULL != fgets(buf, sizeof(buf), fp)) {
		if (0 == strncasecmp(buf, "USAGE:", strlen("USAGE:"))) {
			data_flag = 0;
		} else if (0 == strncasecmp(buf, "result:", strlen("result:"))) {
			sscanf(buf ,"result:%d", &walk_ret);
			break;
		}

	    if (1 != data_flag) {
			memset(buf, 0x00, strlen(buf));
			continue;
		}

		data = get_data(oid, buf);

		if (0 <= data) {
			total += data;
			counter++;
			data  = -1;
		}

		if (1 == show_mod) {
			fprintf(stdout, "%s", buf);
		}

		memset(buf, 0x00, strlen(buf));
	}

	if (0 == walk_ret && 0 < total) {
		/* success : walk return 0 and get data */
		ret = 100 - total / counter;
	} else {
		/* other */
		ret = -1;
	}

	if (NULL != fp)
		pclose(fp);
finish:

	return ret;
}



int main(void)
{
#if 0
	char *command = "/SmartGrid/snmp/bin/snmpwalk -v 3 -l authNoPriv -u zhangliuying -a MD5 -A zhangliuying 192.168.12.76 .1.3.6.1.4.1.2021.11.11.0 2>&1\necho \"result:$?\"";
#endif
#if 0
	char *command = "/SmartGrid/snmp/bin/snmpwalk -v 2c -c public 192.168.12.80 .1.3.6.1.2.1.25.3.3.1.2 2>&1\necho \"result:$?\"";
#else
	char *command = "/SmartGrid/snmp/bin/snmpwalk -v 2c -c public 192.168.12.80 .1.3.6.1.2.1.25.2.3.1.2 2>&1\necho \"result:$?\"";
#endif
	
	int data = 0;
#if 0
	data = get_cpu(command, 1);
#else
	data = get_mem_type(command, 1);
#endif

	fprintf(stdout, "data: %d\n", data);

	return 0;
}
