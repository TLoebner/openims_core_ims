# $Id: Makefile $date aon $
#
# Emergency CSCF
#
# 
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=ecscf.so

ifeq ($(CFG_NAME),sip-router)
	DEFS += -DSER_MOD_INTERFACE
	SERLIBPATH=../../lib
	SER_LIBS+=$(SERLIBPATH)/srdb1/srdb1
	LIBS += -L../../libs/cds -L../../lib/lost -l_ser_cds -l_lost_client
else
	LIBS += -L../../libs/cds -L../../lib/lost -l_ser_cds -l_lost_client
endif	
DEFS += -I/usr/include/libxml2 -I../../lib -DSER -I../..
LIBS += -lxml2 -L../../lib/cds -l_ser_cds -L../../lib/lost -l_lost_client

DEFS += -DCDP_FOR_SER
DEFS += -I/usr/include/libxml2 
LIBS += -L$(LOCALBASE)/lib -lxml2

include ../../Makefile.modules
