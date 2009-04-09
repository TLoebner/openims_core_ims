# $Id: Makefile $date aon $
#
# Emergency CSCF
#
# 
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=ecscf.so
LIBS=

# DEFS+=-I/usr/include/libxml2 -I$(LOCALBASE)/include/libxml2
# LIBS+=-L/usr/include/lib  -L$(LOCALBASE)/lib -lxml2

# experimental (vku)
DEFS += -I/usr/include/libxml2 -I$(LOCALBASE)/include/libxml2 -I$(LOCALBASE)/include -Wall -I../../lib -DSER -I../..
# LIBS += -L$(LOCALBASE)/lib -L/usr/pkg/lib -L../../lib/xcap -L../../lib/presence -L../../lib/cds -lxcap -lpresence -lcds -lxml2 -lcurl 
LIBS += -L$(LOCALBASE)/lib -L/usr/pkg/lib -L../../lib/xcap -L../../lib/presence -L../../lib/cds -L../../lib/lost -l_ser_xcap -l_ser_presence -l_ser_cds -l_lost_client

include ../../Makefile.modules
