# $Id$ Makefile $Date$ $Author$ dvi Dragos Vingarzan
# 
# Makefile for the CDiameterPeer - standalone compilation
#

CC := gcc


# Sources 

sources_base := \
	$(wildcard cdp/*.c) \
	$(wildcard utils/*.c) \
	$(wildcard *.c)

sources_exclude := $(wildcard cdp/mod.*) $(wildcard cdp/cdp_load.*)

sources_base := $(filter-out $(sources_exclude), $(sources_base))

sources := $(sources_base)

alldep := Makefile

mkdep := $(CC) -MM

objects_base 	:= $(sources_base:.c=.o) 
objects		:= $(sources:.c=.o) 

depends_base	:= $(sources_base:.c=.d)
depends		:= $(sources:.c=.d)

# Defines and libs

libs := -lxml2

defines_gen := -DSHM_MEM -DSHM_MMAP -DPKG_MALLOC -DARCH=\"i386\" -DOS=\"linux\" \
-DFAST_LOCK -DADAPTIVE_WAIT -DHAVE_SCHED_YIELD -DADAPTIVE_WAIT_LOOPS=128 -D__CPU_i386 -DHAVE_MSGHDR_MSG_CONTROL \
-I/usr/include/libxml2 

defines_dbg := -DDBG_QM_MALLOC 

defines_release := -DF_MALLOC

cflags := -O0 -g3 -Wall 


defines := $(defines_gen) $(defines_dbg) 

ifeq ($(mode),release)
	cflags := -O3 -Wall
	defines := $(defines_gen) $(defines_release)
endif



#Rules

%.o: %.c  $(alldep)
		@echo $@
		$(CC) $(cflags) $(defines) -c $< 		-o $@

%.d: %.c $(alldep)
		@set -e; $(mkdep) $(defines) $< \
		|  sed 's#\(\($*D)\)\?$(*F)\)\.o[ :]*#$*.o $@ : #g' > $@; \
		[ -s $@ ] || rm -f $@



all: $(depends_base) $(objects_base)
	$(CC) -o main $(objects_base) $(libs)
		
		

clean:
	rm -rf $(objects) main

distclean: 
	rm -rf $(objects) $(depends) main core*


