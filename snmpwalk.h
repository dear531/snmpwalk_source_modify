/* 
 * The file subjoin snmpwalk.c jointly get info cpu and memory
 * information etc.
 * @auther:zhangly
 * @date:2014.7.31
 */
#ifndef __snmpwalk_h__
#define __snmpwalk_h__
struct get_info{
	/* hold node of mib */
	char *oid;
	/*
	 * hold function for handle info assign into
	 * pointer of function global varialbe global_get_info
	 */
	int (*get_handle)(const u_char *buf);
};
/* hold row of table pdu */
static int cpu_counter = 0;
/* every cpu num user value percent */
float **num = NULL;
static int 
get_cpu_info(const u_char *buf)
{
	float **p;
	int i;
	p = num;
	num = realloc(num, sizeof(*num) * (cpu_counter + 1));
	if (NULL == num)
		goto failure;
	num[cpu_counter] = malloc(sizeof(*num[cpu_counter]));
	if (NULL == num[cpu_counter])
		goto failure;
	/* 
	 * SNMPv2-SMI::enterprises.99999.16.1.1.1 = STRING: "10:35:24 AM  CPU   %user   %nice    %sys %iowait
	 * %irq   %soft  %steal   %idle    intr/s "
	 * SNMPv2-SMI::enterprises.99999.16.1.1.2 = STRING: "10:35:24 AM    0    0.97    0.00    2.82   15.13    0.00
	 * 0.04    0.00   81.04     27.88 "
	 * ....
	 * sscanf discast start several invalid value
	 */
	sscanf(buf, "%*s %*s %*s %*s %*s %*s %f", num[cpu_counter]);
	cpu_counter++;
	return 0;
failure:
	for (i = 0; i < cpu_counter; i++)
		free(num[i]);
	free(num);
	return 1;
}

struct get_info cpu = {
	/* cpu oid of standard mib */
	.oid = ".1.3.6.1.4.1.99999.16",
	/* cpu headle function */
	.get_handle = get_cpu_info,
};
static struct{
	int used;
	int total;
	int free;
}mem_info;
static mem_counter = 0;
enum mem_status
{
	MEM_SECCESS,
	MEM_FAILRUE,
	MEM_OTHER,
};
static int get_mem_info(const u_char *buf)
{
	if (1 == mem_counter && NULL != buf) {
	/* 
	 * SNMPv2-SMI::enterprises.99999.15.1.1.1 = STRING: "             total       used       free     shared
	 * buffers     cached "
	 * SNMPv2-SMI::enterprises.99999.15.1.1.2 = STRING: "Mem:          7976        754       7221          0
	 * 37         74 "
	 * SNMPv2-SMI::enterprises.99999.15.1.1.3 = STRING: "Swap:         3914          0       3914
	 */
		sscanf(buf, "%*s %*s %*s %*s %d %d %d",
			&mem_info.total, &mem_info.used, &mem_info.free);
		return MEM_SECCESS;
	} else if (1 == mem_counter) {
		/* mem_counter == 1 && buf == NULL FAILURE */
		return MEM_FAILRUE;
	} else {
		return MEM_OTHER;
	}
}
struct get_info mem = {
	/* memory oid of standard mib */
	.oid = ".1.3.6.1.4.1.99999.15",
	/* memory headle function */
	.get_handle = get_mem_info,
};

/* 
 * in function snmpwalk of snmpwalk.c handle data
 * .e.g : global_get_info = cpu.get_handle get info for cpu
 */
static int (*global_get_info)(const u_char *buf);
#endif
