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
	} else if (0 == memcmp(oid, STANDARD_MEM_TYPE, sizeof(STANDARD_MEM_TYPE))) {
		sscanf(buff, "HOST-RESOURCES-MIB::hrStorageType.%d", &data);
	}

	if (0 <= data) {
		ret = data;
	} else {
		ret = -1;
	}

	return ret;
}


#define WALK_COMMAND	"/SmartGrid/snmp/bin/snmpwalk"
#define REDIRECT_ERR	"2>&1"
#define SHOW_RESULT		"\necho \"result:$?\"\n"

int snmp_oid(char *rsinfo, char *oid, int show_mod)
{
	char buf[1024] = {0};
	int walk_ret;
	int data = -1;
	int ret;
	int data_flag = 1;
	int counter = 0;
	int total = 0;
	char command[1024] = {0};

	FILE * fp;
	if (NULL == rsinfo || NULL == oid) {
		ret = -1;
		goto finish;
	}
	
	sprintf(command, "%s %s %s %s", WALK_COMMAND, rsinfo, oid, SHOW_RESULT);
	/*
	 * command = "/SmartGrid/snmp/bin/snmpwalk -v 2c -c public 192.168.12.80 .1.3.6.1.2.1.25.2.3.1.2 2>&1\necho \"result:$?\"\n";
	 */

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

		if (0 == memcmp(oid, STANDARD_CPU_OID, sizeof(STANDARD_CPU_OID))) {
			data = get_data(oid, buf);

			if (0 <= data) {
				total += data;
				counter++;
				data  = -1;
			}
		} else if (0 == memcmp(oid, STANDARD_MEM_TYPE, sizeof(STANDARD_MEM_TYPE))
				&& NULL != strstr(buf, "hrStorageRam")) {
			data = get_data(oid, buf);
		}

		if (1 == show_mod) {
			fprintf(stdout, "%s", buf);
		}

		memset(buf, 0x00, strlen(buf));
	}

	if (0 == memcmp(oid, STANDARD_CPU_OID, sizeof(STANDARD_CPU_OID)) && 0 == walk_ret && 0 < total) {
		/* success : walk return 0 and get data */
		ret = 100 - total / counter;
	} else if (0 == memcmp(oid, STANDARD_MEM_TYPE, sizeof(STANDARD_MEM_TYPE)) && 0 <= data) {
		ret = data;
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
#if 1
	char *rsinfo = "-v 2c -c public 192.168.12.80";
#endif
	
	int data = 0;
#if 1
	data = snmp_oid(rsinfo, STANDARD_CPU_OID, 1);
	fprintf(stdout, "cpu :%d\n", data);
	data = snmp_oid(rsinfo, STANDARD_MEM_TYPE, 1);
	fprintf(stdout, "index :%d\n", data);
	data = snmp_oid(rsinfo, STANDARD_MEM_UNIT, 1);
	fprintf(stdout, "unit :%d\n", data);
#endif

	return 0;
}
