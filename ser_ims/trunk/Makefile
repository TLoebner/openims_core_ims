# $Id$
#
# sip_router makefile
#
# WARNING: requires gmake (GNU Make)
#  Arch supported: Linux, FreeBSD, SunOS (tested on Solaris 8), OpenBSD (3.2),
#  NetBSD (1.6).
#
#  History:
#  --------
#              created by andrei
#  2003-02-24  make install no longer overwrites ser.cfg  - patch provided
#               by Maxim Sobolev   <sobomax@FreeBSD.org> and 
#                  Tomas Bj??rklund <tomas@webservices.se>
#  2003-03-11  PREFIX & LOCALBASE must also be exported (andrei)
#  2003-04-07  hacked to work with solaris install (andrei)
#  2003-04-17  exclude modules overwritable from env. or cmd. line,
#               added include_modules and skip_modules (andrei)
#  2003-05-30  added extra_defs & EXTRA_DEFS
#               Makefile.defs force-included to allow recursive make
#               calls -- see comment (andrei)
#  2003-06-02  make tar changes -- unpacks in $NAME-$RELEASE  (andrei)
#  2003-06-03  make install-cfg will properly replace the module path
#               in the cfg (re: /usr/.*lib/ser/modules)
#              ser.cfg.default is installed only if there is a previous
#               cfg. -- fixes packages containing ser.cfg.default (andrei)
#  2003-08-29  install-modules-doc split from install-doc, added 
#               install-modules-all, removed README.cfg (andrei)
#              added skip_cfg_install (andrei)
#  2004-09-02  install-man will automatically "fix" the path of the files
#               referred in the man pages
#  2006-02-14  added utils & install-utils (andrei)
#  2006-03-15  added nodeb parameter for make tar (andrei)
#  2006-09-29  added modules-doc as target and doc_format= as make option (greger)
#

auto_gen=lex.yy.c cfg.tab.c #lexx, yacc etc
auto_gen_others=cfg.tab.h  # auto generated, non-c

#include  source related defs
include Makefile.sources

# whether or not to install ser.cfg or just ser.cfg.default
# (ser.cfg will never be overwritten by make install, this is usefull
#  when creating packages)
skip_cfg_install?=

#extra modules to exclude
skip_modules?=

# Set document format
# Alternatives are txt, html, xhtml, and pdf (see Makefile.doc)
doc_format?=html

# if not set on the cmd. line or the env, exclude this modules:
exclude_modules?= 			acc cpl ext extcmd radius_acc radius_auth vm\
							group mangler auth_diameter \
							postgres snmp \
							im \
							jabber \
							cpl-c \
							auth_radius group_radius uri_radius avp_radius \
							acc_radius pa rls presence_b2b xcap xmlrpc\
							osp tls \
							unixsock eval
# always exclude the CVS dir
override exclude_modules+= CVS $(skip_modules)

#always include this modules
include_modules?=

# first 2 lines are excluded because of the experimental or incomplete
# status of the modules
# the rest is excluded because it depends on external libraries
#
static_modules=
static_modules_path=$(addprefix modules/, $(static_modules))
extra_sources=$(wildcard $(addsuffix /*.c, $(static_modules_path)))
extra_objs=$(extra_sources:.c=.o)

static_defs= $(foreach  mod, $(static_modules), \
		-DSTATIC_$(shell echo $(mod) | tr [:lower:] [:upper:]) )

override extra_defs+=$(static_defs) $(EXTRA_DEFS)
export extra_defs

modules=$(filter-out $(addprefix modules/, \
			$(exclude_modules) $(static_modules)), \
			$(wildcard modules/*))
modules:=$(filter-out $(modules), $(addprefix modules/, $(include_modules) )) \
			$(modules)
modules_names=$(shell echo $(modules)| \
				sed -e 's/modules\/\([^/ ]*\)\/*/\1.so/g' )
modules_basenames=$(shell echo $(modules)| \
				sed -e 's/modules\/\([^/ ]*\)\/*/\1/g' )
#modules_names=$(patsubst modules/%, %.so, $(modules))
modules_full_path=$(join  $(modules), $(addprefix /, $(modules_names)))


# which utils need compilation (directory path) and which to install
# (full path including file name)
utils_compile=	utils/gen_ha1 utils/serunix utils/sercmd
utils_install=	utils/gen_ha1/gen_ha1 utils/serunix/serunix \
				scripts/sc scripts/mysql/ser_mysql.sh utils/sercmd/sercmd


ALLDEP=Makefile Makefile.sources Makefile.defs Makefile.rules

#include general defs (like CC, CFLAGS  a.s.o)
# hack to force makefile.defs re-inclusion (needed when make calls itself with
# other options -- e.g. make bin)
makefile_defs=0
DEFS:=
include Makefile.defs

NAME=$(MAIN_NAME)

#export relevant variables to the sub-makes
export DEFS PROFILE CC LD MKDEP MKTAGS CFLAGS LDFLAGS INCLUDES MOD_CFLAGS MOD_LDFLAGS 
export LIBS
export LEX YACC YACC_FLAGS
export PREFIX LOCALBASE
# export relevant variables for recursive calls of this makefile 
# (e.g. make deb)
#export LIBS
#export TAR 
#export NAME RELEASE OS ARCH 
#export cfg-prefix cfg-dir bin-prefix bin-dir modules-prefix modules-dir
#export doc-prefix doc-dir man-prefix man-dir ut-prefix ut-dir
#export cfg-target modules-target
#export INSTALL INSTALL-CFG INSTALL-BIN INSTALL-MODULES INSTALL-DOC INSTALL-MAN 
#export INSTALL-TOUCH

tar_name=$(NAME)-$(RELEASE)_src

tar_extra_args+=$(addprefix --exclude=$(notdir $(CURDIR))/, \
					$(auto_gen) $(auto_gen_others))
ifneq ($(TLS),)
	tar_extra_args+=
else
	tar_extra_args+=--exclude=$(notdir $(CURDIR))/tls* 
endif

ifneq ($(nodeb),)
	tar_extra_args+=--exclude=$(notdir $(CURDIR))/debian 
	tar_name:=$(tar_name)_nodeb
endif
# include the common rules
include Makefile.rules

#extra targets 

$(NAME): $(extra_objs) # static_modules

lex.yy.c: cfg.lex cfg.tab.h $(ALLDEP)
	$(LEX) $<

cfg.tab.c cfg.tab.h: cfg.y  $(ALLDEP)
	$(YACC) $(YACC_FLAGS) $<

.PHONY: all
all: $(NAME) modules



.PHONY: modules
modules:
	-@for r in $(modules) "" ; do \
		if [ -n "$$r" ]; then \
			echo  "" ; \
			echo  "" ; \
			$(MAKE) -C $$r ; \
		fi ; \
	done 

$(extra_objs):
	-@echo "Extra objs: $(extra_objs)" 
	-@for r in $(static_modules_path) "" ; do \
		if [ -n "$$r" ]; then \
			echo  "" ; \
			echo  "Making static module $r" ; \
			$(MAKE) -C $$r static ; \
		fi ; \
	done 

.PHONY: utils
utils:
	-@for r in $(utils_compile) "" ; do \
		if [ -n "$$r" ]; then \
			echo  "" ; \
			echo  "" ; \
			$(MAKE) -C $$r ; \
		fi ; \
	done 


dbg: ser
	gdb -command debug.gdb

.PHONY: tar
.PHONY: dist

dist: tar

tar: 
	$(TAR) -C .. \
		--exclude=$(notdir $(CURDIR))/test* \
		--exclude=$(notdir $(CURDIR))/tmp* \
		--exclude=$(notdir $(CURDIR))/debian/ser \
		--exclude=$(notdir $(CURDIR))/debian/ser-* \
		--exclude=$(notdir $(CURDIR))/ser_tls* \
		--exclude=CVS* \
		--exclude=.svn* \
		--exclude=.cvsignore \
		--exclude=*.[do] \
		--exclude=*.so \
		--exclude=*.il \
		--exclude=$(notdir $(CURDIR))/ser \
		--exclude=*.gz \
		--exclude=*.bz2 \
		--exclude=*.tar \
		--exclude=*.patch \
		--exclude=.\#* \
		--exclude=*.swp \
		${tar_extra_args} \
		-cf - $(notdir $(CURDIR)) | \
			(mkdir -p tmp/_tar1; mkdir -p tmp/_tar2 ; \
			    cd tmp/_tar1; $(TAR) -xf - ) && \
			    mv tmp/_tar1/$(notdir $(CURDIR)) \
			       tmp/_tar2/"$(NAME)-$(RELEASE)" && \
			    (cd tmp/_tar2 && $(TAR) \
			                    -zcf ../../"$(tar_name)".tar.gz \
			                               "$(NAME)-$(RELEASE)" ) ; \
			    rm -rf tmp/_tar1; rm -rf tmp/_tar2

# binary dist. tar.gz
.PHONY: bin
bin:
	mkdir -p tmp/ser/usr/local
	$(MAKE) install basedir=tmp/ser prefix=/usr/local 
	$(TAR) -C tmp/ser/ -zcf ../$(NAME)-$(RELEASE)_$(OS)_$(ARCH).tar.gz .
	rm -rf tmp/ser

.PHONY: deb
deb:
	-@if [ -d debian ]; then \
		dpkg-buildpackage -rfakeroot -tc; \
	else \
		ln -s pkg/debian debian; \
		dpkg-buildpackage -rfakeroot -tc; \
		rm debian; \
	fi

.PHONY: sunpkg
sunpkg:
	mkdir -p tmp/ser
	mkdir -p tmp/ser_sun_pkg
	$(MAKE) install basedir=tmp/ser prefix=/usr/local
	(cd pkg/solaris; \
	pkgmk -r ../../tmp/ser/usr/local -o -d ../../tmp/ser_sun_pkg/ -v "$(RELEASE)" ;\
	cd ../..)
	cat /dev/null > ../$(NAME)-$(RELEASE)-$(OS)-$(ARCH)-local
	pkgtrans -s tmp/ser_sun_pkg/ ../$(NAME)-$(RELEASE)-$(OS)-$(ARCH)-local \
		IPTELser
	gzip -9 ../$(NAME)-$(RELEASE)-$(OS)-$(ARCH)-local
	rm -rf tmp/ser
	rm -rf tmp/ser_sun_pkg

.PHONY: modules-doc
modules-doc:
	-@for r in $(modules) "" ; do \
		if [ -n "$$r" ]; then \
			echo  "" ; \
			echo  "" ; \
			$(MAKE) -C $$r/doc $(doc_format) ; \
		fi ; \
	done 

.PHONY: install
install: all mk-install-dirs install-cfg install-bin install-modules \
	install-doc install-man install-utils
		mv -f $(bin-prefix)/$(bin-dir)/sc $(bin-prefix)/$(bin-dir)/serctl #fix

.PHONY: dbinstall
dbinstall:
	-@echo "Initializing ser database"
	scripts/mysql/ser_mysql.sh create
	-@echo "Done"

mk-install-dirs: $(cfg-prefix)/$(cfg-dir) $(bin-prefix)/$(bin-dir) \
			$(modules-prefix)/$(modules-dir) $(doc-prefix)/$(doc-dir) \
			$(man-prefix)/$(man-dir)/man8 $(man-prefix)/$(man-dir)/man5


$(cfg-prefix)/$(cfg-dir): 
		mkdir -p $(cfg-prefix)/$(cfg-dir)

$(bin-prefix)/$(bin-dir):
		mkdir -p $(bin-prefix)/$(bin-dir)

$(modules-prefix)/$(modules-dir):
		mkdir -p $(modules-prefix)/$(modules-dir)


$(doc-prefix)/$(doc-dir):
		mkdir -p $(doc-prefix)/$(doc-dir)

$(man-prefix)/$(man-dir)/man8:
		mkdir -p $(man-prefix)/$(man-dir)/man8

$(man-prefix)/$(man-dir)/man5:
		mkdir -p $(man-prefix)/$(man-dir)/man5
		
# note: on solaris 8 sed: ? or \(...\)* (a.s.o) do not work
install-cfg: $(cfg-prefix)/$(cfg-dir)
		sed -e "s#/usr/.*lib/ser/modules/#$(modules-target)#g" \
			< etc/ser.cfg > $(cfg-prefix)/$(cfg-dir)ser.cfg.sample
		chmod 644 $(cfg-prefix)/$(cfg-dir)ser.cfg.sample
		if [ -z "${skip_cfg_install}" -a \
				! -f $(cfg-prefix)/$(cfg-dir)ser.cfg ]; then \
			mv -f $(cfg-prefix)/$(cfg-dir)ser.cfg.sample \
				$(cfg-prefix)/$(cfg-dir)ser.cfg; \
		fi
		# radius dictionary
		$(INSTALL-TOUCH) $(cfg-prefix)/$(cfg-dir)/dictionary.ser 
		$(INSTALL-CFG) etc/dictionary.ser $(cfg-prefix)/$(cfg-dir)
#		$(INSTALL-CFG) etc/ser.cfg $(cfg-prefix)/$(cfg-dir)

install-bin: $(bin-prefix)/$(bin-dir) 
		$(INSTALL-TOUCH) $(bin-prefix)/$(bin-dir)/ser 
		$(INSTALL-BIN) ser $(bin-prefix)/$(bin-dir)

install-modules: modules $(modules-prefix)/$(modules-dir)
	-@for r in $(modules_full_path) "" ; do \
		if [ -n "$$r" ]; then \
			if [ -f "$$r" ]; then \
				$(INSTALL-TOUCH) \
					$(modules-prefix)/$(modules-dir)/`basename "$$r"` ; \
				$(INSTALL-MODULES)  "$$r"  $(modules-prefix)/$(modules-dir) ; \
			else \
				echo "ERROR: module $$r not compiled" ; \
			fi ;\
		fi ; \
	done 

install-utils: utils $(bin-prefix)/$(bin-dir)
	-@for r in $(utils_install) "" ; do \
		if [ -n "$$r" ]; then \
			if [ -f "$$r" ]; then \
				$(INSTALL-TOUCH) \
					$(bin-prefix)/$(bin-dir)/`basename "$$r"` ; \
				$(INSTALL-BIN)  "$$r"  $(bin-prefix)/$(bin-dir) ; \
			else \
				echo "ERROR: $$r not compiled" ; \
			fi ;\
		fi ; \
	done 



install-modules-all: install-modules install-modules-doc


install-doc: $(doc-prefix)/$(doc-dir) install-modules-doc
	$(INSTALL-TOUCH) $(doc-prefix)/$(doc-dir)/INSTALL 
	$(INSTALL-DOC) INSTALL $(doc-prefix)/$(doc-dir)
	$(INSTALL-TOUCH) $(doc-prefix)/$(doc-dir)/README-MODULES 
	$(INSTALL-DOC) README-MODULES $(doc-prefix)/$(doc-dir)
	$(INSTALL-TOUCH) $(doc-prefix)/$(doc-dir)/AUTHORS 
	$(INSTALL-DOC) AUTHORS $(doc-prefix)/$(doc-dir)
	$(INSTALL-TOUCH) $(doc-prefix)/$(doc-dir)/NEWS
	$(INSTALL-DOC) NEWS $(doc-prefix)/$(doc-dir)
	$(INSTALL-TOUCH) $(doc-prefix)/$(doc-dir)/README 
	$(INSTALL-DOC) README $(doc-prefix)/$(doc-dir)


install-modules-doc: $(doc-prefix)/$(doc-dir)
	-@for r in $(modules_basenames) "" ; do \
		if [ -n "$$r" ]; then \
			if [ -f modules/"$$r"/README ]; then \
				$(INSTALL-TOUCH)  $(doc-prefix)/$(doc-dir)/README ; \
				$(INSTALL-DOC)  modules/"$$r"/README  \
									$(doc-prefix)/$(doc-dir)/README ; \
				mv -f $(doc-prefix)/$(doc-dir)/README \
						$(doc-prefix)/$(doc-dir)/README."$$r" ; \
			fi ; \
		fi ; \
	done 


install-man: $(man-prefix)/$(man-dir)/man8 $(man-prefix)/$(man-dir)/man5
		sed -e "s#/etc/ser/ser\.cfg#$(cfg-target)ser.cfg#g" \
			-e "s#/usr/sbin/#$(bin-target)#g" \
			-e "s#/usr/lib/ser/modules/#$(modules-target)#g" \
			-e "s#/usr/share/doc/ser/#$(doc-target)#g" \
			< ser.8 >  $(man-prefix)/$(man-dir)/man8/ser.8
		chmod 644  $(man-prefix)/$(man-dir)/man8/ser.8
		sed -e "s#/etc/ser/ser\.cfg#$(cfg-target)ser.cfg#g" \
			-e "s#/usr/sbin/#$(bin-target)#g" \
			-e "s#/usr/lib/ser/modules/#$(modules-target)#g" \
			-e "s#/usr/share/doc/ser/#$(doc-target)#g" \
			< ser.cfg.5 >  $(man-prefix)/$(man-dir)/man5/ser.cfg.5
		chmod 644  $(man-prefix)/$(man-dir)/man5/ser.cfg.5
