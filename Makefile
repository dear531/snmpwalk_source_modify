#
# Minimum environment and virtual path setup
#
SHELL		= /bin/sh
srcdir		= .
top_srcdir	= ..
VERSION		= 5.4.4


#
# Programs
#
LIBTOOLCLEAN	= $(LIBTOOL) --mode=clean rm -f

#
# Compiler arguments
#
#CFLAGS		= -fno-strict-aliasing -g -O2 -Ulinux -Dlinux=linux -I/usr/include/rpm  -D_REENTRANT -D_GNU_SOURCE -fno-strict-aliasing -pipe -Wdeclaration-after-statement -I/usr/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -I/usr/include/gdbm  -I/usr/lib64/perl5/5.8.8/x86_64-linux-thread-multi/CORE  
CFLAGS		= -Wall -g
LIBTOOL		= $(SHELL) $(top_builddir)/libtool 
EXEEXT		= 

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
LINK		= $(LIBTOOL) --mode=link $(LINKCC)
# RANLIB 	= ranlib
RANLIB		= :

# libtool definitions
.SUFFIXES: .c .o .lo .rc
.c.lo:
	$(LIBTOOL) --mode=compile $(CC) $(CFLAGS) -c -o $@ $<
.rc.lo:
	$(LIBTOOL) --mode=compile --tag=CC windres -o $@ -i $<

# include paths
#

top_builddir=..

# use GNU vpath, if available, to only set a path for source and headers
# VPATH will pick up objects too, which is bad if you are sharing a
# source dir...
#vpath %.h $(srcdir)
#vpath %.c $(srcdir)
# fallback to regular VPATH for non-gnu...
VPATH = $(srcdir)



# USELIBS/USEAGENTLIBS are for dependencies
USELIBS		= ../snmplib/libnetsnmp.$(LIB_EXTENSION)

#
# link path in src dir
LOCAL_LIBS	= -L../snmplib/.libs
LIBS		= $(USELIBS) 
OSUFFIX		= lo
OBJS  = *.o
LOBJS = *.lo

all:snmpwalk

#
# build rules
#
snmpwalk$(EXEEXT):    snmpwalk.$(OSUFFIX) $(USELIBS)
	$(LINK) ${CFLAGS} -o $@ snmpwalk.$(OSUFFIX) $(LOCAL_LIBS) ${LIBS} 

#
# cleaning targets
#
clean: cleansubdirs $(OTHERCLEANTODOS)
	$(LIBTOOLCLEAN) ${OBJS} ${LOBJS} core $(STANDARDCLEANTARGETS) $(OTHERCLEANTARGETS)



# These aren't real targets, let gnu's make know that.
.PHONY: clean cleansubdirs lint \
	install installprogs installheaders installlibs \
	installbin installsbin installsubdirs \
	all subdirs standardall objs \
	depend nosysdepend distdepend dependdirs nosysdependdirs distdependdirs

# DO NOT DELETE THIS LINE -- make depend depends on it.
