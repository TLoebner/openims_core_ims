# $Id$ Makefile  $Date$ $Author$ dvi
#
# example module makefile
#
#
# WARNING: do not run this directly, it should be run by the master Makefile



include ../../Makefile.defs
auto_gen=
NAME=scscf.so


DEFS += -I/usr/include/libxml2 -I../../lib -DSER -I../.. 
LIBS += -lxml2 -L../../lib/cds -l_ser_cds

DEFS += -DWITH_IMS_PM

include ../../Makefile.modules



