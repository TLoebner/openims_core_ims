# $Id$ Makefile  $Date$ $Author$ dvi
#
# Makefile for the Proxy-CSCF module
# \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
#
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=pcscf.so

include ../../Makefile.modules

DEFS += -I/usr/include/libxml2 -I../../lib -DSER -I../..
LIBS += -lxml2 -L../../lib/cds -l_ser_cds

#DEFS += -DWITH_IMS_PM