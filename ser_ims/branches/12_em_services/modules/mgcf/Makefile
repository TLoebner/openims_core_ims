# $Id: Makefile 671 2009-06-12 07:51:06Z vingarzan $
#
# Makefile for the MGCF module
# \author Dragos Vingarzan dragos dot vingarzan -at- fokus dot fraunhofer dot de
#
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=mgcf.so

ifeq ($(CFG_NAME),sip-router)
	DEFS += -DSER_MOD_INTERFACE
	SERLIBPATH=/../lib
	DEFS += -I../lib -DSER -I../..
	DEFS += -I../../lib 
	LIBS += -L/usr/local/lib -L../lib/cds -l_ser_cds
else
	DEFS += -I/usr/include/libxml2 -I../../lib -DSER -I../..
	LIBS += -L/usr/local/lib -L../../lib/cds -l_ser_cds
endif	

DEFS += -DCDP_FOR_SER


include ../../Makefile.modules


