# $Id$
#
# example module makefile
#
# 
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=sl.so
LIBS= 

ifeq ($(CFG_NAME),sip-router)
	DEFS += -DSER_MOD_INTERFACE
	SERLIBPATH=/../lib
	#SER_LIBS+=$(SERLIBPATH)/srdb1/srdb1
else
endif

include ../../Makefile.modules
