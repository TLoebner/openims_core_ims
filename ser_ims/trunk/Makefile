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
#  2006-12-09  added new group_include as make option and defined groups 
#               defining which modules to include. Also added new target 
#               print-modules that you can use to check which modules will be 
#               compiled (greger)
#  2007-01-10  added new group_include targets mysql, radius, and presence 
#               improved print-modules output fixed problem in include/exclude
#               logic when using group_include (greger)
#  2007-03-01  fail if a module or a required utility make fail unless 
#              err_fail=0; don't try to make modules with no Makefiles (andrei)

auto_gen=lex.yy.c cfg.tab.c #lexx, yacc etc
auto_gen_others=cfg.tab.h  # auto generated, non-c

#include  source related defs
include Makefile.sources

# whether or not the entire build process should fail if building a module or
#  an utility fails
err_fail?=1

# whether or not to install ser.cfg or just ser.cfg.default
# (ser.cfg will never be overwritten by make install, this is usefull
#  when creating packages)
skip_cfg_install?=

#extra modules to exclude
skip_modules?=

# Set document format
# Alternatives are txt, html, xhtml, and pdf (see Makefile.doc)
doc_format?=html

# Module group definitions, default only include the standard group
# Make backwards compatible, don't set group_include default...
#group_include?="standard"

# Modules in this group are considered a standard part of SER (due to 
# widespread usage) and have no external compile or link dependencies (note 
# that some of these interplay with external systems).
module_group_standard=acc_syslog auth avp avpops ctl dispatcher diversion enum\
				eval exec fifo flatstore gflags maxfwd mediaproxy \
				nathelper options pdt permissions pike print ratelimit \
				registrar rr sanity sl textops timer tm uac unixsock uri \
				usrloc xlog

# Modules in this group are considered a standard part of SER (due to 
# widespread usage) but they have dependencies that must be satisfied for 
# compilation.
# acc_radius, auth_radius, avp_radius, uri_radius => radiusclient-ng
# acc_db, auth_db, avp_db, db_ops, domain, lcr, msilo, dialog, speeddial,
# uri_db => database module (mysql, postgres, dbtext)
# mysql, postgres => mysql server and client libraries or postgres server and
#  client libraries or other database back-end (ex. mysql-devel)
# pa, xmlrpc => libxml2
# rls => pa
#
# NOTE! All presence modules (dialog, pa, presence_b2b, rls, xcap) have been
# included in this group due to interdependencies
module_group_standard_dep=acc_db acc_radius auth_db auth_radius avp_db \
				avp_radius \
				db_ops domain lcr msilo mysql dialog pa postgres \
				presence_b2b rls speeddial uri_db xcap xmlrpc

# For mysql
module_group_mysql=acc_db auth_db avp_db db_ops uri_db domain lcr msilo mysql\
				speeddial

# For radius
module_group_radius=acc_radius auth_radius avp_radius

# For presence
module_group_presence=dialog pa presence_b2b rls xcap

# Modules in this group satisfy specific or niche applications, but are 
# considered stable for production use. They may or may not have dependencies
# cpl-c => libxml2
# jabber => expat (library)
# osp => OSP Toolkit (sipfoundry)
# sms => none (external modem)
module_group_stable=cpl-c dbtext jabber osp sms

# Modules in this group are either not complete, untested, or without enough
# reports of usage to allow the module into the stable group. They may or may
# not have dependencies
module_group_experimental=tls oracle

# if not set on the cmd. line or the env, exclude the below modules.
ifneq ($(group_include),)
	# For group_include, default all modules are excluded except those in 
	# include_modules
	exclude_modules?=
else
	# Old defaults for backwards compatibility
	exclude_modules?= 			acc cpl ext extcmd radius_acc radius_auth vm\
							group mangler auth_diameter \
							postgres snmp \
							im \
							jabber \
							cpl-c \
							auth_radius group_radius uri_radius avp_radius \
							acc_radius pa rls presence_b2b xcap xmlrpc\
							osp tls oracle \
							unixsock eval dbg
endif

# always exclude the CVS dir
override exclude_modules+= CVS $(skip_modules)

# Test for the groups and add to include_modules
ifneq (,$(findstring standard,$(group_include)))
	override include_modules+= $(module_group_standard)
endif

ifneq (,$(findstring standard-dep,$(group_include)))
	override include_modules+= $(module_group_standard_dep)
endif

ifneq (,$(findstring mysql,$(group_include)))
	override include_modules+= $(module_group_mysql)
endif

ifneq (,$(findstring radius,$(group_include)))
	override include_modules+= $(module_group_radius)
endif

ifneq (,$(findstring presence,$(group_include)))
	override include_modules+= $(module_group_presence)
endif

ifneq (,$(findstring stable,$(group_include)))
	override include_modules+= $(module_group_stable)
endif

ifneq (,$(findstring experimental,$(group_include)))
	override include_modules+= $(module_group_experimental)
endif

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

# Historically, the resultant set of modules is: modules/* - exclude_modules +
# include_modules
# When group_include is used, we want: include_modules (based on group_include)
# - exclude_modules
ifneq ($(group_include),)
	modules=$(filter-out $(addprefix modules/, \
			$(exclude_modules) $(static_modules)), \
			$(addprefix modules/, $(include_modules) ))
else	
	# Standard, old resultant set
	modules=$(filter-out $(addprefix modules/, \
			$(exclude_modules) $(static_modules)), \
			$(wildcard modules/*))
	modules:=$(filter-out $(modules), $(addprefix modules/, $(include_modules) )) \
			$(modules)
endif
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
export DEFS PROFILE CC LD MKDEP MKTAGS CFLAGS LDFLAGS INCLUDES MOD_CFLAGS \
		MOD_LDFLAGS 
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
ifeq ($(CORE_TLS), 1)
	tar_extra_args+=
else
	tar_extra_args+=--exclude=$(notdir $(CURDIR))/tls/* 
endif

ifneq ($(nodeb),)
	tar_extra_args+=--exclude=$(notdir $(CURDIR))/debian 
	tar_name:=$(tar_name)_nodeb
endif

# sanity checks
ifneq ($(TLS),)
	$(warning "make TLS option is obsoleted, try TLS_HOOKS or CORE_TLS")
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

.PHONY: print-modules
print-modules:
	@echo The following modules was chosen to be included: $(include_modules) ; \
	echo ---------------------------------------------------------- ; \
	echo The following modules will be excluded: $(exclude_modules) ; \
	echo ---------------------------------------------------------- ; \
	echo The following modules will be made: $(modules_basenames) ; \

.PHONY: modules
modules:
	@for r in $(modules) "" ; do \
		if [ -n "$$r" -a -r "$$r/Makefile" ]; then \
			echo  "" ; \
			echo  "" ; \
			if ! $(MAKE) -C $$r && [ ${err_fail} = 1 ] ; then \
				exit 1; \
			fi ; \
		fi ; \
	done; true

$(extra_objs):
	@echo "Extra objs: $(extra_objs)" 
	@for r in $(static_modules_path) "" ; do \
		if [ -n "$$r" -a -r "$$r/Makefile"  ]; then \
			echo  "" ; \
			echo  "Making static module $r" ; \
			if ! $(MAKE) -C $$r static ; then  \
				exit 1; \
			fi ;  \
		fi ; \
	done

.PHONY: utils
utils:
	@for r in $(utils_compile) "" ; do \
		if [ -n "$$r" ]; then \
			echo  "" ; \
			echo  "" ; \
			if ! $(MAKE) -C $$r && [ ${err_fail} = 1 ] ; then \
				exit 1; \
			fi ; \
		fi ; \
	done; true


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
		make -f debian/rules install; \
		make -f debian/rules binary-common; \
		make -f debian/rules clean; \
	else \
		ln -s pkg/debian debian; \
		make -f debian/rules install; \
		make -f debian/rules binary-common; \
		make -f debian/rules clean; \
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

install-cfg-pcscf: $(cfg-prefix)/$(cfg-dir)
		@echo "installing configuration of pcscf" ; \
		sed -s -e "s#/opt/OpenIMSCore/ser_ims/modules/.*/#$(modules-target)#g" \
			-e "s#/opt/OpenIMSCore/pcscf.xml#/$(cfg-dir)pcscf.xml#" \
			-e "s#log_stderror=yes#log_stderror=no#" \
			-e "s#debug=3#debug=1#" \
			< cfg/pcscf.cfg > $(cfg-prefix)/$(cfg-dir)pcscf.cfg.sample ; \
		chmod 644 $(cfg-prefix)/$(cfg-dir)pcscf.cfg.sample ; \
		if [ -z "${skip_cfg_install}" -a \
				! -f $(cfg-prefix)/$(cfg-dir)pcscf.cfg ]; then \
			mv -f $(cfg-prefix)/$(cfg-dir)pcscf.cfg.sample \
				$(cfg-prefix)/$(cfg-dir)pcscf.cfg; \
		fi ; \
		cp cfg/pcscf.xml $(cfg-prefix)/$(cfg-dir)pcscf.xml.sample ; \
		chmod 644 $(cfg-prefix)/$(cfg-dir)pcscf.xml.sample ; \
		if [ -z "${skip_cfg_install}" -a \
				! -f $(cfg-prefix)/$(cfg-dir)pcscf.xml ]; then \
			mv -f $(cfg-prefix)/$(cfg-dir)pcscf.xml.sample \
				$(cfg-prefix)/$(cfg-dir)pcscf.xml; \
		fi ; \
		cp modules/pcscf/reginfo.dtd $(modules-target) ; \
		chmod 644 $(modules-target)/reginfo.dtd ; \
		for s in ipsec_E_Drop.sh ipsec_E_Inc_Req.sh ipsec_E_Inc_Rpl.sh ipsec_E_Out_Req.sh \
			ipsec_E_Out_Rpl.sh ipsec_P_Drop.sh ipsec_P_Inc_Req.sh ipsec_P_Inc_Rpl.sh \
			ipsec_P_Out_Req.sh ipsec_P_Out_Rpl.sh ; do \
			cp modules/pcscf/$$s $(modules-target) ; \
			chmod 755 $(modules-target)/$$s ; \
		done ; \

install-cfg-icscf: $(cfg-prefix)/$(cfg-dir)
		@echo "installing configuration of icscf" ; \
		sed -s -e "s#/opt/OpenIMSCore/ser_ims/modules/.*/#$(modules-target)#g" \
			-e "s#/opt/OpenIMSCore/icscf.xml#/$(cfg-dir)icscf.xml#" \
			-e "s#log_stderror=yes#log_stderror=no#" \
			-e "s#debug=3#debug=1#" \
			< cfg/icscf.cfg > $(cfg-prefix)/$(cfg-dir)icscf.cfg.sample ; \
		chmod 644 $(cfg-prefix)/$(cfg-dir)icscf.cfg.sample ; \
		if [ -z "${skip_cfg_install}" -a \
				! -f $(cfg-prefix)/$(cfg-dir)icscf.cfg ]; then \
			mv -f $(cfg-prefix)/$(cfg-dir)icscf.cfg.sample \
				$(cfg-prefix)/$(cfg-dir)icscf.cfg; \
		fi ; \
		cp cfg/icscf.xml $(cfg-prefix)/$(cfg-dir)icscf.xml.sample ; \
		chmod 644 $(cfg-prefix)/$(cfg-dir)icscf.xml.sample ; \
		if [ -z "${skip_cfg_install}" -a \
				! -f $(cfg-prefix)/$(cfg-dir)icscf.xml ]; then \
			mv -f $(cfg-prefix)/$(cfg-dir)icscf.xml.sample \
				$(cfg-prefix)/$(cfg-dir)icscf.xml; \
		fi ; \

install-cfg-scscf: $(cfg-prefix)/$(cfg-dir)
		@echo "installing configuration of scscf" ; \
		sed -s -e "s#/opt/OpenIMSCore/ser_ims/modules/.*/#$(modules-target)#g" \
			-e "s#/opt/OpenIMSCore/scscf.xml#/$(cfg-dir)scscf.xml#" \
			-e "s#log_stderror=yes#log_stderror=no#" \
			-e "s#debug=3#debug=1#" \
			< cfg/scscf.cfg > $(cfg-prefix)/$(cfg-dir)scscf.cfg.sample ; \
		chmod 644 $(cfg-prefix)/$(cfg-dir)scscf.cfg.sample ; \
		if [ -z "${skip_cfg_install}" -a \
				! -f $(cfg-prefix)/$(cfg-dir)scscf.cfg ]; then \
			mv -f $(cfg-prefix)/$(cfg-dir)scscf.cfg.sample \
				$(cfg-prefix)/$(cfg-dir)scscf.cfg; \
		fi ; \
		cp cfg/scscf.xml $(cfg-prefix)/$(cfg-dir)scscf.xml.sample ; \
		chmod 644 $(cfg-prefix)/$(cfg-dir)scscf.xml.sample ; \
		if [ -z "${skip_cfg_install}" -a \
				! -f $(cfg-prefix)/$(cfg-dir)scscf.xml ]; then \
			mv -f $(cfg-prefix)/$(cfg-dir)scscf.xml.sample \
				$(cfg-prefix)/$(cfg-dir)scscf.xml; \
		fi ; \
		for f in CxDataType.dtd CxDataType_Rel6.xsd CxDataType_Rel7.xsd; do \
			cp modules/scscf/$$f $(modules-target) ; \
			chmod 644 $(modules-target)/$$f ; \
		done ; \

install-cfg-cscf: 
		@echo "installing common configuration of cscfs" ; \
		for f in add-imscore-user_newdb.sh configurator.sh open-ims.dnszone; do \
			cp cfg/$$f ${example-dir}; \
			chmod 644 ${example-dir}/$$f; \
		done; \

install-bin: $(bin-prefix)/$(bin-dir) 
		$(INSTALL-TOUCH) $(bin-prefix)/$(bin-dir)/ser 
		$(INSTALL-BIN) ser $(bin-prefix)/$(bin-dir)

install-modules: modules $(modules-prefix)/$(modules-dir)
	@for r in $(modules_full_path) "" ; do \
		if [ -n "$$r" ]; then \
			if [ -f "$$r" ]; then \
				$(INSTALL-TOUCH) \
					$(modules-prefix)/$(modules-dir)/`basename "$$r"` ; \
				$(INSTALL-MODULES)  "$$r"  $(modules-prefix)/$(modules-dir) ; \
			else \
				echo "ERROR: module $$r not compiled" ; \
				if [ ${err_fail} = 1 ] ; then \
					exit 1; \
				fi ; \
			fi ;\
		fi ; \
	done; true

install-utils: utils $(bin-prefix)/$(bin-dir)
	@for r in $(utils_install) "" ; do \
		if [ -n "$$r" ]; then \
			if [ -f "$$r" ]; then \
				$(INSTALL-TOUCH) \
					$(bin-prefix)/$(bin-dir)/`basename "$$r"` ; \
				$(INSTALL-BIN)  "$$r"  $(bin-prefix)/$(bin-dir) ; \
			else \
				echo "ERROR: $$r not compiled" ; \
				if [ ${err_fail} = 1 ] ; then \
					exit 1; \
				fi ; \
			fi ;\
		fi ; \
	done; true



install-modules-all: install-modules install-modules-doc

install-modules-cscf: install-modules install-cfg-cscf install-modules-doc 

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
	@for r in $(modules_basenames) "" ; do \
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


##################
# making libraries
# 
# you can use:
#    make libs all include_modules=... install prefix=...
#    make libs proper
#
# but libs should be compiled/installed automaticaly when there are any modules which need them

lib_dependent_modules = dialog pa rls presence_b2b xcap

# exports for libs
export cfg-prefix cfg-dir bin-prefix bin-dir modules-prefix modules-dir
export doc-prefix doc-dir man-prefix man-dir ut-prefix ut-dir
export INSTALL INSTALL-CFG INSTALL-BIN INSTALL-MODULES INSTALL-DOC INSTALL-MAN 
export INSTALL-TOUCH

dep_mods = $(filter $(addprefix modules/, $(lib_dependent_modules)), $(modules))
dep_mods += $(filter $(lib_dependent_modules), $(static_modules))

# make 'modules' dependent on libraries if there are modules which need them 
# (experimental)
ifneq ($(strip $(dep_mods)),)
modules:	libs

install-modules:	install-libs

endif

.PHONY: clean_libs libs install-libs

clean_libs:
			$(MAKE) -f Makefile.ser -C lib proper

# cleaning in libs always when cleaning ser
clean:	clean_libs

libs:	
		$(MAKE) -C lib -f Makefile.ser

install-libs:	
		$(MAKE) -C lib -f Makefile.ser install

