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

/* 
 * in function snmpwalk of snmpwalk.c handle data
 * .e.g : global_get_info = cpu.get_handle get info for cpu
 */
extern int mibs_snmpwalk(int snmpargc, char *snmpargv[], int mibargc, char *mibargv[]);
#endif
