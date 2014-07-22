#
# Minimum environment and virtual path setup
#
LIBTOOLCLEAN	= --mode=clean rm -f

#
# Compiler arguments
#
CFLAGS		= -g -O2
#CFLAGS		= -fno-strict-aliasing -g -O2 -Ulinux -Dlinux=linux -I/usr/include/rpm  -D_REENTRANT -D_GNU_SOURCE -fno-strict-aliasing -pipe -Wdeclaration-after-statement -I/usr/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -I/usr/include/gdbm  -I/usr/lib64/perl5/5.8.8/x86_64-linux-thread-multi/CORE  

# Misc Compiling Stuff
CC	        = gcc
LINKCC	        = gcc
LINK		= $(LINKCC)

# libtool definitions
.SUFFIXES: .c .o
.c:
	--mode=compile $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

# USELIBS/USEAGENTLIBS are for dependencies
USELIBS			= -lnetsnmp

#
# link path in src dir
LIBS		= $(USELIBS) 
OSUFFIX		= o
OBJS  = *.o

#
# build rules
#
snmpwalk$(EXEEXT):    snmpwalk.$(OSUFFIX) $(USELIBS)
	$(LINK) ${CFLAGS} -o $@ snmpwalk.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

objs: ${OBJS}


#
# cleaning targets
#
clean: $(OTHERCLEANTODOS)
	$(LIBTOOLCLEAN) ${OBJS} core $(STANDARDCLEANTARGETS) $(OTHERCLEANTARGETS)
# These aren't real targets, let gnu's make know that.
.PHONY:
	clean objs
