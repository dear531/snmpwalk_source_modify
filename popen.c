#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#define STANDARD_CPU_OID	".1.3.6.1.2.1.25.3.3.1.2"
#define STANDARD_MEM_TYPE	".1.3.6.1.2.1.25.2.3.1.2"
#define STANDARD_MEM_UNIT	".1.3.6.1.2.1.25.2.3.1.4"
#define STANDARD_MEM_SIZE	".1.3.6.1.2.1.25.2.3.1.5"
#define STANDARD_MEM_USED	".1.3.6.1.2.1.25.2.3.1.6"

#define WALK_COMMAND	"/SmartGrid/snmp/bin/snmpwalk"
#define REDIRECT_ERR	"2>&1"
#define SHOW_RESULT		"\necho \"result:$?\"\n"

static int snmp_oid(char *rsinfo, char *oid, int show_mod)
{
	char buf[1024] = {0};
	char prompt[1024] = {0};
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
			sscanf(buf, "HOST-RESOURCES-MIB::hrProcessorLoad.%*d = INTEGER: %d", &data);
			if (0 <= data) {
				total += data;
				counter++;
				data  = -1;
			}
			strcat(prompt, buf);
		} else if (0 == memcmp(oid, STANDARD_MEM_TYPE, sizeof(STANDARD_MEM_TYPE))) {
			if (NULL != strstr(buf, "hrStorageRam")) {
				sscanf(buf, "HOST-RESOURCES-MIB::hrStorageType.%d", &data);
			}
			if (0 < data) {
				total = data;
				data = -1;
				memset(prompt, 0x00, strlen(prompt));
				strcat(prompt, buf);
			}

			if (0 == total) {
				strcat(prompt, buf);
			}
		} else if (0 == memcmp(oid, STANDARD_MEM_UNIT, sizeof(STANDARD_MEM_UNIT) -1)) {
			sscanf(buf, "HOST-RESOURCES-MIB::hrStorageAllocationUnits.%*d = INTEGER: %d Bytes", &data);
#define DATA_OPRATION() do {					\
							if (0 < data) {		\
								total = data;	\
								data = -1;		\
							}					\
							strcat(prompt, buf);\
						}while (0)
			DATA_OPRATION();
		} else if (0 == memcmp(oid, STANDARD_MEM_SIZE, sizeof(STANDARD_MEM_SIZE) - 1)) {
			sscanf(buf, "HOST-RESOURCES-MIB::hrStorageSize.%*d = INTEGER: %d", &data);
			DATA_OPRATION();
		} else if (0 == memcmp(oid, STANDARD_MEM_USED, sizeof(STANDARD_MEM_USED) - 1)) {
			sscanf(buf, "HOST-RESOURCES-MIB::hrStorageUsed.%*d = INTEGER: %d", &data);
			DATA_OPRATION();
		} else {
			memcpy(prompt, "invalid oid\n", sizeof("invalid oid\n"));
		}
#undef DATA_OPRATION

		memset(buf, 0x00, strlen(buf));
	}

	if (0 == memcmp(oid, STANDARD_MEM_TYPE, sizeof(STANDARD_MEM_TYPE)) && 0 == total && 0 == walk_ret) {
		memset(prompt, 0x00, strlen(prompt));
		memcpy(prompt, "cannot find HOST-RESOURCES-TYPES::hrStorageRam\n",
				sizeof("cannot find HOST-RESOURCES-TYPES::hrStorageRam\n"));
	}

	if (1 == show_mod) {
		fprintf(stdout, "%s", prompt);
	}

	if (0 == memcmp(oid, STANDARD_CPU_OID, sizeof(STANDARD_CPU_OID)) && 0 == walk_ret && 0 < total) {
		/* success : walk return 0 and get data */
		ret = total / counter;
	} else if (0 == memcmp(oid, STANDARD_MEM_TYPE, sizeof(STANDARD_MEM_TYPE)) && 0 < total) {
		ret = total;
	} else if (0 == memcmp(oid, STANDARD_MEM_UNIT, sizeof(STANDARD_MEM_UNIT) - 1) && 0 < total) {
		ret = total;
	} else if (0 == memcmp(oid, STANDARD_MEM_SIZE, sizeof(STANDARD_MEM_SIZE) - 1) && 0 < total) {
		ret = total;
	} else if (0 == memcmp(oid, STANDARD_MEM_USED, sizeof(STANDARD_MEM_USED) - 1) && 0 < total) {
		ret = total;
	} else {
		/* other */
		ret = -1;
	}

	if (NULL != fp)
		pclose(fp);
finish:

	return ret;
}

#define SNMP_DEBUG	0
unsigned long int get_cpu_free(char *rsinfo, int show_mod)
{
	unsigned long int data = 0;
	data = snmp_oid(rsinfo, STANDARD_CPU_OID, show_mod);
#if SNMP_DEBUG
	fprintf(stdout, "cpu :%d\n", data);
#endif
	return data;
}
#define check_data() do{				\
						if (0 >= data) {\
							goto err;	\
						}				\
					} while(0)
unsigned long int get_mem_free(char *rsinfo, int show_mod)
{
	unsigned long int data = 0;
	char tmpoid[sizeof(STANDARD_MEM_UNIT) + sizeof(".dd") - 1] = {0};
	unsigned long int index;
	unsigned long int unit;
	unsigned long int size;
	unsigned long int used;
	data = snmp_oid(rsinfo, STANDARD_MEM_TYPE, show_mod);
#if SNMP_DEBUG
	fprintf(stdout, "index :%d\n", data);
#endif
	check_data();

	index = data;
	memset(tmpoid, 0x00, strlen(tmpoid));
	sprintf(tmpoid, "%s.%d", STANDARD_MEM_UNIT, index);
	data = snmp_oid(rsinfo, tmpoid, show_mod);
	check_data();
	unit = data;
#if SNMP_DEBUG
	fprintf(stdout, "unit :%d\n", data);
#endif

	memset(tmpoid, 0x00, strlen(tmpoid));
	sprintf(tmpoid, "%s.%d", STANDARD_MEM_SIZE, index);
	data = snmp_oid(rsinfo, tmpoid, show_mod);
	check_data();
	size = data;
#if SNMP_DEBUG
	fprintf(stdout, "size :%d, mem total: %ld\n", data, size * unit);
#endif

	memset(tmpoid, 0x00, strlen(tmpoid));
	sprintf(tmpoid, "%s.%d", STANDARD_MEM_USED, index);
	data = snmp_oid(rsinfo, tmpoid, show_mod);
	check_data();
	used = data;
#if SNMP_DEBUG
	fprintf(stdout, "used :%d, mem used: %ld\n", used, used * unit);
#endif
	return (size - used) * unit;
err:
	return -1;
}

unsigned long int snmp_walk(char *rsinfo, int show_mod)
{
	unsigned long int cpu_free;
	unsigned long int mem_free;
    unsigned long int data;
	data = get_cpu_free(rsinfo, show_mod);
    check_data();
    cpu_free = data;
	data = get_mem_free(rsinfo, show_mod);
    mem_free = data;
    check_data();
    if (1 == show_mod) {
        fprintf(stdout, "cpu free :%ld %%\n", cpu_free);
        fprintf(stdout, "mem free :%ld Bytes\n", mem_free);
    }
    return ;
err:
    return -1;
}
#undef SNMP_DEBUG
#undef check_data
#if 1
int main(void)
{
	char *rsinfo = "-v 2c -c public 192.168.12.80";
    snmp_walk(rsinfo, 1);
	return 0;
}
#endif
/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
