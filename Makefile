#
# Minimum environment and virtual path setup
#
SHELL		= /bin/sh
srcdir		= .

LIBTOOLCLEAN	= $(LIBTOOL) --mode=clean rm -f

#
# Compiler arguments
#
CFLAGS		=
#CFLAGS		= -fno-strict-aliasing -g -O2 -Ulinux -Dlinux=linux -I/usr/include/rpm  -D_REENTRANT -D_GNU_SOURCE -fno-strict-aliasing -pipe -Wdeclaration-after-statement -I/usr/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -I/usr/include/gdbm  -I/usr/lib64/perl5/5.8.8/x86_64-linux-thread-multi/CORE  
LIBTOOL		= $(SHELL) $(top_builddir)/libtool 

# Misc Compiling Stuff
CC	        = gcc
LINKCC	        = gcc

# use libtool versioning the way they recommend.
# The (slightly clarified) rules:
#
# - If any interfaces/structures have been removed or changed since the
#   last update, increment current, and set age and revision to 0. Stop.
#
# - If any interfaces have been added since the last public release, then
#   increment current and age, and set revision to 0. Stop.
# 
# - If the source code has changed at all since the last update,
#   then increment revision (c:r:a becomes c:r+1:a). 
#
# Note: maintenance releases (eg 5.2.x) should never have changes
#       that would require a current to be incremented.
#
# policy: we increment major releases of LIBCURRENT by 5 starting at
# 5.3 was at 10, 5.4 is at 15, ...  This leaves some room for needed
# changes for past releases if absolutely necessary.
# 

LIB_EXTENSION   = la
LIB_VERSION     =
LINK		= $(LIBTOOL) --mode=link $(LINKCC)

# libtool definitions
.SUFFIXES: .c .o .lo .rc
.c.lo:
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
.rc.lo:
	$(LIBTOOL) --mode=compile --tag=CC windres -o $@ -i $<

top_builddir=..

# use GNU vpath, if available, to only set a path for source and headers
# VPATH will pick up objects too, which is bad if you are sharing a
# source dir...
#vpath %.h $(srcdir)
#vpath %.c $(srcdir)
# fallback to regular VPATH for non-gnu...
VPATH = $(srcdir)

#
# Things to install
#

INSTALLBINPROGS	= snmpwalk$(EXEEXT)


# USELIBS/USEAGENTLIBS are for dependencies
#USELIBS		= ./snmplib/libnetsnmp.$(LIB_EXTENSION)$(LIB_VERSION) 
USELIBS			= -lnetsnmp

#
# link path in src dir
LOCAL_LIBS	=
LIBS		= $(USELIBS) 
OSUFFIX		= lo
OBJS  = *.o
LOBJS = *.lo

#
# build rules
#
snmpwalk$(EXEEXT):    snmpwalk.$(OSUFFIX) $(USELIBS)
	$(LINK) ${CFLAGS} -o $@ snmpwalk.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

objs: ${OBJS} ${LOBJS}


#
# cleaning targets
#
clean: cleansubdirs $(OTHERCLEANTODOS)
	$(LIBTOOLCLEAN) ${OBJS} ${LOBJS} core $(STANDARDCLEANTARGETS) $(OTHERCLEANTARGETS)
# These aren't real targets, let gnu's make know that.
.PHONY: clean cleansubdirs lint \
	all objs \
	depend dependdirs
./snmpwalk.lo: ../include/net-snmp/net-snmp-config.h
./snmpwalk.lo: ../include/net-snmp/system/linux.h
./snmpwalk.lo: ../include/net-snmp/system/sysv.h
./snmpwalk.lo: ../include/net-snmp/system/generic.h
./snmpwalk.lo: ../include/net-snmp/machine/generic.h 
./snmpwalk.lo: ../include/net-snmp/net-snmp-includes.h
./snmpwalk.lo: ../include/net-snmp/definitions.h ../include/net-snmp/types.h
./snmpwalk.lo:  ../include/net-snmp/library/snmp_api.h
./snmpwalk.lo: ../include/net-snmp/library/asn1.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_impl.h
./snmpwalk.lo: ../include/net-snmp/library/snmp.h
./snmpwalk.lo: ../include/net-snmp/library/snmp-tc.h
./snmpwalk.lo: ../include/net-snmp/utilities.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_client.h
./snmpwalk.lo: ../include/net-snmp/library/system.h
./snmpwalk.lo: ../include/net-snmp/library/tools.h
./snmpwalk.lo: ../include/net-snmp/library/int64.h
./snmpwalk.lo: ../include/net-snmp/library/mt_support.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpwalk.lo: ../include/net-snmp/library/callback.h
./snmpwalk.lo: ../include/net-snmp/library/data_list.h
./snmpwalk.lo: ../include/net-snmp/library/oid_stash.h
./snmpwalk.lo: ../include/net-snmp/library/check_varbind.h
./snmpwalk.lo: ../include/net-snmp/library/container.h
./snmpwalk.lo: ../include/net-snmp/library/factory.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_logging.h
./snmpwalk.lo: ../include/net-snmp/library/container_binary_array.h
./snmpwalk.lo: ../include/net-snmp/library/container_list_ssll.h
./snmpwalk.lo: ../include/net-snmp/library/container_iterator.h
./snmpwalk.lo: ../include/net-snmp/library/container.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_assert.h
./snmpwalk.lo: ../include/net-snmp/version.h
./snmpwalk.lo: ../include/net-snmp/session_api.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_transport.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_service.h
./snmpwalk.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpwalk.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpwalk.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpwalk.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpwalk.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpwalk.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpwalk.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpwalk.lo: ../include/net-snmp/library/ucd_compat.h
./snmpwalk.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmpwalk.lo: ../include/net-snmp/library/mib.h
./snmpwalk.lo: ../include/net-snmp/library/parse.h
./snmpwalk.lo: ../include/net-snmp/varbind_api.h
./snmpwalk.lo: ../include/net-snmp/config_api.h
./snmpwalk.lo: ../include/net-snmp/library/read_config.h
./snmpwalk.lo: ../include/net-snmp/library/default_store.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_enum.h
./snmpwalk.lo: ../include/net-snmp/library/vacm.h
./snmpwalk.lo: ../include/net-snmp/output_api.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_debug.h
./snmpwalk.lo: ../include/net-snmp/snmpv3_api.h
./snmpwalk.lo: ../include/net-snmp/library/snmpv3.h
./snmpwalk.lo: ../include/net-snmp/library/transform_oids.h
./snmpwalk.lo: ../include/net-snmp/library/keytools.h
./snmpwalk.lo: ../include/net-snmp/library/scapi.h
./snmpwalk.lo: ../include/net-snmp/library/lcd_time.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpwalk.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpwalk.lo: ../include/net-snmp/library/snmpusm.h
