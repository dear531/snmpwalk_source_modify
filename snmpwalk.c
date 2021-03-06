/*
 * snmpwalk.c - send snmp GETNEXT requests to a network entity, walking a
 * subtree.
 *
 */
/**********************************************************************
	Copyright 1988, 1989, 1991, 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/
/*
 * The source code official of net-snmp-5.4.4 developmant,
 * above descrip net-snmp-5.4.4 
 * @snmpwalk function form source code main import
 * @author:zhangly
 * @date:2014.7.31
 */
#include <net-snmp/net-snmp-config.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <stdio.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <net-snmp/net-snmp-includes.h>

#define NETSNMP_DS_WALK_INCLUDE_REQUESTED	        1
#define NETSNMP_DS_WALK_PRINT_STATISTICS	        2
#define NETSNMP_DS_WALK_DONT_CHECK_LEXICOGRAPHIC	3
#define NETSNMP_DS_WALK_TIME_RESULTS     	        4
#define NETSNMP_DS_WALK_DONT_GET_REQUESTED	        5
#include "snmpwalk.h"

oid             objid_mib[] = { 1, 3, 6, 1, 2, 1 };
int             numprinted = 0;

/*
 * pirnt error information flag,
 * is 0: close print for smart machine,
 * is 1: open print for debug
 */
static int snmp_show_flag = 0;

/* hold row of table pdu */
static int cpu_counter = 0;
/* every cpu num user value percent */
static float *num = NULL;
static int 
get_cpu_info(const u_char *buf)
{
	num = realloc(num, sizeof(*num) * (cpu_counter + 1));
	if (NULL == num)
		goto failure;
	/* 
	 * SNMPv2-SMI::enterprises.99999.16.1.1.1 = STRING: "10:35:24 AM  CPU   %user   %nice    %sys %iowait
	 * %irq   %soft  %steal   %idle    intr/s "
	 * SNMPv2-SMI::enterprises.99999.16.1.1.2 = STRING: "10:35:24 AM    0    0.97    0.00    2.82   15.13    0.00
	 * 0.04    0.00   81.04     27.88 "
	 * ....
	 * sscanf discast start several invalid value
	 */
	sscanf((char *)buf, "%*s %*s %*s %*s %*s %*s %f", num + cpu_counter);
	cpu_counter++;
	return 0;
failure:
	if (NULL != num)
		free(num);
	return 1;
}

static struct get_info cpu = {
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
static int mem_counter = 0;
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
		sscanf((char *)buf, "%*s %*s %*s %*s %d %d %d",
			&mem_info.total, &mem_info.used, &mem_info.free);
		return MEM_SECCESS;
	} else if (1 == mem_counter) {
		/* mem_counter == 1 && buf == NULL FAILURE */
		return MEM_FAILRUE;
	} else {
		return MEM_OTHER;
	}
}
static struct get_info mem = {
	/* memory oid of standard mib */
	.oid = ".1.3.6.1.4.1.99999.15",
	/* memory headle function */
	.get_handle = get_mem_info,
};

static int (*global_get_info)(const u_char *buf);
static void
usage(void)
{
    fprintf(stderr, "USAGE: snmpwalk ");
    snmp_parse_args_usage(stderr);
    fprintf(stderr, " [OID]\n\n");
    snmp_parse_args_descriptions(stderr);
    fprintf(stderr,
            "  -C APPOPTS\t\tSet various application specific behaviours:\n");
    fprintf(stderr, "\t\t\t  p:  print the number of variables found\n");
    fprintf(stderr, "\t\t\t  i:  include given OID in the search range\n");
    fprintf(stderr, "\t\t\t  I:  don't include the given OID, even if no results are returned\n");
    fprintf(stderr,
            "\t\t\t  c:  do not check returned OIDs are increasing\n");
    fprintf(stderr,
            "\t\t\t  t:  Display wall-clock time to complete the request\n");
}

static void
snmp_get_and_print(netsnmp_session * ss, oid * theoid, size_t theoid_len)
{
    netsnmp_pdu    *pdu, *response;
    netsnmp_variable_list *vars;
    int             status;

    pdu = snmp_pdu_create(SNMP_MSG_GET);
    snmp_add_null_var(pdu, theoid, theoid_len);

    status = snmp_synch_response(ss, pdu, &response);
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
        for (vars = response->variables; vars; vars = vars->next_variable) {
            numprinted++;
#define PRINT_FLAG	0
#if PRINT_FLAG
            print_variable(vars->name, vars->name_length, vars);
#else
			/*
			 * replace from origin net-snmp-5.4.4 to our code
			 * from print_variable to sprint_realloc_variable
			 */
					u_char *buf = NULL;
					size_t buf_len = 0;
					size_t out_len = 0;
					sprint_realloc_variable(&buf, &buf_len, &out_len, 1,
							vars->name, vars->name_length, vars);
/* flag for get infomartion show to standard output */
					if (SNMP_SHOW == snmp_show_flag)
						fprintf(stdout, "%s\n", buf);
					global_get_info(buf);
					if (out_len > 0)
						free(buf);
#endif
        }
    }
    if (response) {
        snmp_free_pdu(response);
    }
}

static void
optProc(int argc, char *const *argv, int opt)
{
    switch (opt) {
    case 'C':
        while (*optarg) {
            switch (*optarg++) {
            case 'i':
                netsnmp_ds_toggle_boolean(NETSNMP_DS_APPLICATION_ID,
					  NETSNMP_DS_WALK_INCLUDE_REQUESTED);
                break;

            case 'I':
                netsnmp_ds_toggle_boolean(NETSNMP_DS_APPLICATION_ID,
					  NETSNMP_DS_WALK_DONT_GET_REQUESTED);
                break;

            case 'p':
                netsnmp_ds_toggle_boolean(NETSNMP_DS_APPLICATION_ID,
					  NETSNMP_DS_WALK_PRINT_STATISTICS);
                break;

            case 'c':
                netsnmp_ds_toggle_boolean(NETSNMP_DS_APPLICATION_ID,
				    NETSNMP_DS_WALK_DONT_CHECK_LEXICOGRAPHIC);
                break;

            case 't':
                netsnmp_ds_toggle_boolean(NETSNMP_DS_APPLICATION_ID,
                                          NETSNMP_DS_WALK_TIME_RESULTS);
                break;
                
            default:
				if (SNMP_SHOW == snmp_show_flag)
					fprintf(stderr, "Unknown flag passed to -C: %c\n",
                        optarg[-1]);
				return;
            }
        }
        break;
    }
}


static int
snmpwalk(int argc, char *argv[])
{
    netsnmp_session session, *ss;
    netsnmp_pdu    *pdu, *response;
    netsnmp_variable_list *vars;
    int             arg;
    oid             name[MAX_OID_LEN];
    size_t          name_length;
    oid             root[MAX_OID_LEN];
    size_t          rootlen;
    int             count;
    int             running;
    int             status;
    int             check;
    int             exitval = 0;
    struct timeval  tv1, tv2;

    netsnmp_ds_register_config(ASN_BOOLEAN, "snmpwalk", "includeRequested",
			       NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_WALK_INCLUDE_REQUESTED);

    netsnmp_ds_register_config(ASN_BOOLEAN, "snmpwalk", "excludeRequested",
			       NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_WALK_DONT_GET_REQUESTED);

    netsnmp_ds_register_config(ASN_BOOLEAN, "snmpwalk", "printStatistics",
			       NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_WALK_PRINT_STATISTICS);

    netsnmp_ds_register_config(ASN_BOOLEAN, "snmpwalk", "dontCheckOrdering",
			       NETSNMP_DS_APPLICATION_ID,
			       NETSNMP_DS_WALK_DONT_CHECK_LEXICOGRAPHIC);

    netsnmp_ds_register_config(ASN_BOOLEAN, "snmpwalk", "timeResults",
                               NETSNMP_DS_APPLICATION_ID,
			       NETSNMP_DS_WALK_TIME_RESULTS);

    /*
     * get the common command line arguments 
     */
    switch (arg = snmp_parse_args(argc, argv, &session, "C:", optProc)) {
    case -2:
		return 1;
    case -1:
#if 0
        usage();
#endif
		return 2;
    default:
        break;
    }

    /*
     * get the initial object and subtree 
     */
    if (arg < argc) {
        /*
         * specified on the command line 
         */
        rootlen = MAX_OID_LEN;
        if (snmp_parse_oid(argv[arg], root, &rootlen) == NULL) {
			if (SNMP_SHOW == snmp_show_flag)
				snmp_perror(argv[arg]);
			return 2;
        }
    } else {
        /*
         * use default value 
         */
        memmove(root, objid_mib, sizeof(objid_mib));
        rootlen = sizeof(objid_mib) / sizeof(oid);
    }

    SOCK_STARTUP;

    /*
     * open an SNMP session 
     */
    ss = snmp_open(&session);
    if (ss == NULL) {
        /*
         * diagnose snmp_open errors with the input netsnmp_session pointer 
         */
		if (SNMP_SHOW == snmp_show_flag)
			snmp_sess_perror("snmpwalk", &session);
        SOCK_CLEANUP;
		return 2;
    }

    /*
     * get first object to start walk 
     */
    memmove(name, root, rootlen * sizeof(oid));
    name_length = rootlen;

    running = 1;

    check =
        !netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID,
                        NETSNMP_DS_WALK_DONT_CHECK_LEXICOGRAPHIC);
    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_WALK_INCLUDE_REQUESTED)) {
        snmp_get_and_print(ss, root, rootlen);
    }

    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID,
                               NETSNMP_DS_WALK_TIME_RESULTS))
        gettimeofday(&tv1, NULL);
    while (running) {
        /*
         * create PDU for GETNEXT request and add object name to request 
         */
        pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
        snmp_add_null_var(pdu, name, name_length);

        /*
         * do the request 
         */
        status = snmp_synch_response(ss, pdu, &response);
        if (status == STAT_SUCCESS) {
            if (response->errstat == SNMP_ERR_NOERROR) {
                /*
                 * check resulting variables 
                 */
                for (vars = response->variables; vars;
                     vars = vars->next_variable) {
                    if ((vars->name_length < rootlen)
                        || (memcmp(root, vars->name, rootlen * sizeof(oid))
                            != 0)) {
                        /*
                         * not part of this subtree 
                         */
                        running = 0;
                        continue;
                    }
                    numprinted++;
#if PRINT_FLAG
                    print_variable(vars->name, vars->name_length, vars);
					void print_variable(const oid *objid, size_t objidlen, const netsnmp_variable_list
							*variable);
					int sprint_realloc_variable(u_char **buf, size_t *buf_len, size_t *out_len, int
							allow_realloc, const  oid  *objid, size_t objidlen, const netsnmp_variable_list *variable);
#else
					u_char *buf = NULL;
					size_t buf_len = 0;
					size_t out_len = 0;
					sprint_realloc_variable(&buf, &buf_len, &out_len, 1,
							vars->name, vars->name_length, vars);
					if (SNMP_SHOW == snmp_show_flag)
						fprintf(stdout, "%s\n", buf);
					global_get_info(buf);
					if (out_len > 0)
						free(buf);
#endif

                    if ((vars->type != SNMP_ENDOFMIBVIEW) &&
                        (vars->type != SNMP_NOSUCHOBJECT) &&
                        (vars->type != SNMP_NOSUCHINSTANCE)) {
                        /*
                         * not an exception value 
                         */
                        if (check
                            && snmp_oid_compare(name, name_length,
                                                vars->name,
                                                vars->name_length) >= 0) {
					if (SNMP_SHOW == snmp_show_flag) {
                            fprintf(stderr, "Error: OID not increasing: ");
                            fprint_objid(stderr, name, name_length);
                            fprintf(stderr, " >= ");
                            fprint_objid(stderr, vars->name,
                                         vars->name_length);
                            fprintf(stderr, "\n");
					}
                            running = 0;
                            exitval = 1;
                        }
                        memmove((char *) name, (char *) vars->name,
                                vars->name_length * sizeof(oid));
                        name_length = vars->name_length;
                    } else
                        /*
                         * an exception value, so stop 
                         */
                        running = 0;
                }
            } else {
                /*
                 * error in response, print it 
                 */
                running = 0;
				if (SNMP_SHOW == snmp_show_flag)
				{
					if (response->errstat == SNMP_ERR_NOSUCHNAME) {
						printf("End of MIB\n");
					} else {
						if (SNMP_SHOW == snmp_show_flag)
							fprintf(stderr, "Error in packet.\nReason: %s\n",
									snmp_errstring(response->errstat));
						if (response->errindex != 0) {
							if (SNMP_SHOW == snmp_show_flag)
								fprintf(stderr, "Failed object: ");
							for (count = 1, vars = response->variables;
								 vars && count != response->errindex;
								 vars = vars->next_variable, count++)
								/*EMPTY*/;
							if (vars)
								fprint_objid(stderr, vars->name,
											 vars->name_length);
							fprintf(stderr, "\n");
						}
						exitval = 2;
					}
				}
            }
        } else if (status == STAT_TIMEOUT) {
		if (SNMP_SHOW == snmp_show_flag)
		{
            fprintf(stderr, "Timeout: No Response from %s\n",
                    session.peername);
		}
            running = 0;
            exitval = 1;
        } else {                /* status == STAT_ERROR */
			if (SNMP_SHOW == snmp_show_flag)
				snmp_sess_perror("snmpwalk", ss);
            running = 0;
            exitval = 1;
        }
        if (response)
            snmp_free_pdu(response);
    }
    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID,
                               NETSNMP_DS_WALK_TIME_RESULTS))
        gettimeofday(&tv2, NULL);

    if (numprinted == 0 && status == STAT_SUCCESS) {
        /*
         * no printed successful results, which may mean we were
         * pointed at an only existing instance.  Attempt a GET, just
         * for get measure. 
         */
        if (!netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_WALK_DONT_GET_REQUESTED)) {
            snmp_get_and_print(ss, root, rootlen);
        }
    }
    snmp_close(ss);

    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID,
                               NETSNMP_DS_WALK_PRINT_STATISTICS)) {
		if (SNMP_SHOW == snmp_show_flag)
			printf("Variables found: %d\n", numprinted);
    }
    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID,
                               NETSNMP_DS_WALK_TIME_RESULTS)) {
		if (SNMP_SHOW == snmp_show_flag)
			fprintf (stderr, "Total traversal time = %f seconds\n",
                 (double) (tv2.tv_usec - tv1.tv_usec)/1000000 +
                 (double) (tv2.tv_sec - tv1.tv_sec));
    }

    SOCK_CLEANUP;
    return exitval;
}

static int
walk_info_operation(int argc, char *argv[])
{
	int ret;
	if (memcmp(cpu.oid, argv[argc - 1], strlen(cpu.oid) + 1) == 0) {
	/*
	 * snmpwalk -v 3 -l authNoPriv -u usm_user -a MD5(or SHA) -A authenpassword
	 * realipaddrs .1.3.6.1.4.1.99999.16
	 */
		global_get_info = cpu.get_handle;
		ret = snmpwalk(argc, argv);
		if (ret > 0)
			return ret;
	} else if (memcmp(mem.oid, argv[argc - 1], strlen(mem.oid) + 1) == 0) {
	/*
	 * snmpwalk -v 3 -l authNoPriv -u usm_user -a MD5(or SHA) -A authenpassword
	 * realipaddrs .1.3.6.1.4.1.99999.15
	 */
		global_get_info = mem.get_handle;
		ret = snmpwalk(argc, argv);
		if (ret > 0)
			return ret;
	}
	return ret;
}


/*
 * char *snmp_argv[] = {"snmpwalk", "-v", "3", "-l", "authNoPriv",
 * 	"-u", "zhangliuying", "-a", "MD5", "-A", "zhangliuying",
 * 	"192.168.12.78"
 * };
 * char *mib_argv[] = {
 * 	".1.3.6.1.4.1.99999.16", ".1.3.6.1.4.1.99999.15"
 * };
 * 	cpu 			and 		memory
 */

#include <strings.h>
int
mibs_snmpwalk(int snmp_argc, char *snmp_argv[], int mib_argc, char *mib_argv[], int flag)
{
	char **heap_argv;
	int i, ret = 0;
	snmp_show_flag = flag;

	heap_argv = malloc(sizeof(*heap_argv) * snmp_argc + 1);

	for (i = 0; i < snmp_argc; i++)
		heap_argv[i] = strdup(snmp_argv[i]);

	for (i = 0; i < mib_argc; i++) {

		/* every unique one of several oids */
		heap_argv[snmp_argc] = mib_argv[i];
		/*
		 * whenever running snmpwalk will clear user and password,
		 * so reset user and password
		 */
		bcopy(snmp_argv[6], heap_argv[6], strlen(snmp_argv[6]));
		bcopy(snmp_argv[10], heap_argv[10], strlen(snmp_argv[10]));

		/* call operation function get info */
		ret = walk_info_operation(snmp_argc + 1, heap_argv);
		heap_argv[snmp_argc] = NULL;
		if (ret > 0)
			goto flaid;
	}
flaid:
	for (i = 0; i < snmp_argc + 1; i++)
		if (NULL != heap_argv[i])
			free(heap_argv[i]);
	free(heap_argv);
	return ret;
}

int main(int argc, char *argv[])
{
    char *snmpargv[] = {"snmpwalk", "-v", "3", "-l", "authNoPriv",
     "-u", "zhangliuying", "-a", "MD5", "-A", "zhangliuying",
     "192.168.12.78"
    };   
    char *mibargv[] = {
		".1.3.6.1.4.1.99999.15", ".1.3.6.1.4.1.99999.16",
    };   
	int ret;

    ret = mibs_snmpwalk(sizeof(snmpargv) / sizeof(*snmpargv), snmpargv, sizeof(mibargv) / sizeof(*mibargv), mibargv,
			SNMP_SHOW);
	switch (ret) {
		case 0:
			fprintf(stdout, "success\n");
			break;
		case 1:
			fprintf(stdout, "get failed\n");
			break;
		case 2:
			fprintf(stdout, "set flaide\n");
			break;
	}

	return 0;
}
