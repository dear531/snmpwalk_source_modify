#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#define CPUOID	".1.3.6.1.4.1.2021.11.11.0"
#define MEMOID	".1.3.6.1.4.1.2021.4.11.0"

int get_data(char *oid, char* buff)
{
	int data = -1;
	int ret;
	if (0 == memcmp(oid, CPUOID, sizeof(CPUOID))) {
		sscanf(buff, "UCD-SNMP-MIB::ssCpuIdle.0 = INTEGER: %d", &data);
	} else if (0 == memcmp(oid, MEMOID, sizeof(MEMOID))) {
		sscanf(buff, "UCD-SNMP-MIB::memTotalFree.0 = INTEGER: %d", &data);
	}
	if (0 <= data) {
		ret = data;
	} else {
		ret = -1;
	}

	return ret;
}

int get_result(FILE *fp, int show_mod, char *oid, int (*get_data)(char *oid, char *prompt))
{
	char buf[1024] = {0};
	char prompt[1024] = {0};
	int walk_ret;
	int data = 0;
	int ret;
	while (NULL != fgets(buf, sizeof(buf), fp)) {
		if (0 == strncasecmp(buf, "USAGE:", strlen("USAGE:"))) {
			break;
		} else if (0 == strncasecmp(buf, "result:", strlen("result:"))) {
			sscanf(buf ,"result:%d", &walk_ret);
			break;
		} else {
			strcat(prompt, buf);
			data = get_data(oid, buf);
		}
		memset(buf, 0x00, strlen(buf));
	}

	if (1 == show_mod) {
		fprintf(stdout, "%s", prompt);
	}

	if (0 == walk_ret && 0 <= data) {
		/* success : walk return 0 and get data */
		ret = data;
	} else {
		/* other */
		ret = -1;
	}

	return ret;
}



int main(void)
{
	char commmod[512] = {0};
	char *commond = "/SmartGrid/snmp/bin/snmpwalk -v 3 -l authNoPriv -u zhangliuying -a MD5 -A zhangliuying 192.168.12.76 .1.3.6.1.4.1.2021.11.11.0 2>&1\necho \"result:$?\"";
	FILE * fp;
	fp = popen(commond, "r");
	if (NULL == fp) {
		syslog(LOG_INFO, "popen error :%s\n",
				strerror(errno));
		goto finish;
	}
	
	int data = 0;
	data = get_result(fp, 1, CPUOID, get_data);

	fprintf(stdout, "data: %d\n", data);

	if (NULL != fp)
		pclose(fp);
finish:
	return 0;
}
