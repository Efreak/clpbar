#
# To override any of the following variables, you can set the veriable in your
# environment before calling make.  Be sure that make will honor environmental
# overrides, usually by passing the -e flag to make or by setting MAKEFLAGS in
# your environment.
#
# Ex:
#
# 	$ ./configure
# 	$ CFLAGS="-g -Wall" make -e
# 	$ make install
#

# CFLAGS =
# CFLAGS = -O2
# CFLAGS = -g -Wall

# LDFLAGS = -s
# LDFLAGS =

bin_PROGRAMS =
noinst_PROGRAMS =
noinst_HEADERS =
TESTS =
nodist_TESTS = 
BUILT_SOURCES =
CLEANFILES = *~
DISTCLEANFILES = ./deps/*.P
EXTRA_DIST =
man_MANS =

noinst_HEADERS += headers.h

noinst_PROGRAMS += test-types
test_types_SOURCES = \
	test-types.c \
	#
TESTS += test-types

# noinst_PROGRAMS += test-args
# test_args_SOURCES = \
# 	error.c \
# 	fd.c \
# 	io.c \
# 	display.c \
# 	args.c \
# 	test-args.c \
# 	#
# TESTS += test-args

noinst_PROGRAMS += test-001-pre
test_001_pre_SOURCES = \
	test-001-pre.c \
	#

noinst_PROGRAMS += test-001-post
test_001_post_SOURCES = \
	test-001-post.c \
	#

TESTS += test-001
EXTRA_DIST += test-001

TESTS += test-002
EXTRA_DIST += test-002
CLEANFILES += test-file-002.1.bin test-file-002.2.bin

# TESTS += test-003
EXTRA_DIST += test-003
CLEANFILES += test-file-003.incomplete.bin
CLEANFILES += test-file-003.1.bin
CLEANFILES += test-file-003.2.bin
CLEANFILES += test-pipe-003.1
CLEANFILES += test-pipe-003.2
CLEANFILES += test-pipe-003.3

# TESTS += test-004
EXTRA_DIST += test-004
CLEANFILES += test-file-004.incomplete.bin
CLEANFILES += test-file-004.bin
CLEANFILES += test-file-004.1.bin
CLEANFILES += test-file-004.2.bin

EXTRA_DIST += itest
EXTRA_DIST += itest2

DISTCLEANFILES += ./log.test-*

bin_PROGRAMS += bar
noinst_HEADERS += types.h
noinst_HEADERS += error.h
noinst_HEADERS += fd.h
noinst_HEADERS += io.h
noinst_HEADERS += display.h
noinst_HEADERS += args.h
bar_SOURCES = \
	error.c \
	fd.c \
	io.c \
	display.c \
	args.c \
	bar.c \
	#

man_MANS += bar.1
EXTRA_DIST += $(man_MANS)

# EXTRA_DIST += PORTING

EXTRA_DIST += autogen

EXTRA_DIST += debian/changelog
EXTRA_DIST += debian/control
EXTRA_DIST += debian/copyright
EXTRA_DIST += debian/dirs
EXTRA_DIST += debian/install
EXTRA_DIST += debian/rules
EXTRA_DIST += sourceforge.mk

EXTRA_DIST += TROUBLESHOOTING
EXTRA_DIST += troubleshooter

EXTRA_DIST += infofile

include sourceforge.mk

dch:
	cd ./debian && dch --check-dirname-level 0

dchi:
	cd ./debian && dch -i --check-dirname-level 0

#
#------------------------------------------------------------------------------
#

FILE_DIR = ../dist
WEB_DIR = ../www
ARCH := $(shell uname -m)

update: update-dist update-www

update-dist: dist
	[ -d $(FILE_DIR) ] || exit 1
	[ -f $(distdir).tar.gz ] || exit 1
	chmod 0755 $(FILE_DIR)
	-rm -fr $(FILE_DIR)/$(PACKAGE)*
	-rm -fr $(FILE_DIR)/$(distdir)
	gzip -dc $(distdir).tar.gz | (cd $(FILE_DIR) && tar -xvf -)
	[ -x /usr/bin/dpkg-buildpackage ] || exit 1
	[ -x /usr/bin/fakeroot ] || exit 1
	cd $(FILE_DIR)/$(distdir) && dpkg-buildpackage -us -uc -rfakeroot
	rm -fr $(FILE_DIR)/$(distdir)
	
	[ -x /usr/bin/alien ] || exit 1
	cd $(FILE_DIR) && fakeroot alien --to-rpm $(PACKAGE)_$(VERSION)_*.deb
	[ -x /usr/bin/alien ] || exit 1
	cd $(FILE_DIR) && fakeroot alien --to-tgz $(PACKAGE)_$(VERSION)_*.deb
	cd $(FILE_DIR) && mv $(PACKAGE)-$(VERSION).tgz $(PACKAGE)-$(VERSION)_$(ARCH).tgz
	
	-rm ./ChangeLog
	cp debian/changelog $(FILE_DIR)/$(PACKAGE)-$(VERSION)-ChangeLog
	[ ! -f README ] || cp README $(FILE_DIR)/$(PACKAGE)-$(VERSION)-README
	[ -x /usr/bin/groff ] || exit 1
	for file in $(man_MANS); do \
		cat $${file} \
			| groff -man -Tascii \
			| sed \
				-e 's,_,,g' \
				-e 's,.,,g' \
			> $(FILE_DIR)/$(PACKAGE)-$(VERSION)-man.txt \
			; \
	done

update-www:
	[ -d $(WEB_DIR) ] || exit 1
	-rm -fr $(WEB_DIR)/doxygen/$(VERSION)
	cat doxygen.conf.in \
	| sed "s,{PACKAGE},$(PACKAGE),g" \
	| sed "s,{VERSION},$(VERSION),g" \
	| sed "s,{DOC_DIR},$(WEB_DIR)/doxygen/$(VERSION),g" \
	> doxygen.conf
	doxygen ./doxygen.conf
	cd $(WEB_DIR) && $(MAKE)

###############################################################################
#
# Automated Sourceforge Compilefarm Testing
#
###############################################################################

# include sourceforge.mk

# $(PACKAGE)-$(VERSION).tar.gz: $(DIST_COMMON) $(SOURCES)
# 	$(MAKE) dist
# 
# update-sourceforge-compilefarm: .update-sourceforge-compilefarm
# .update-sourceforge-compilefarm: $(PACKAGE)-$(VERSION).tar.gz
# 	tar -cvf - ./$(PACKAGE)-$(VERSION).tar.gz \
# 	| ssh -l michaelpeek shell.cf.sourceforge.net \
# 		"tar -xvf -"
# 	touch $@
# 
# test-sourceforge-compilefarm-generic: .test-sourceforge-compilefarm-$(HOST)
# .test-sourceforge-compilefarm-$(HOST): .update-sourceforge-compilefarm
# 	-ssh -l michaelpeek shell.cf.sourceforge.net \
# 		"test-project \"$(HOST)\" \"$(PACKAGE)\" \"$(VERSION)\"" 2>&1 \
# 		| tee log.sourceforge.$(HOST).txt \
# 		| sed 's,^,[$(HOST)]: ,g'
# 
# test-sourceforge-compilefarm: .update-sourceforge-compilefarm
# 	for host in $(SOURCEFORGE_COMPILE_FARM_HOSTS); do \
# 		$(MAKE) test-sourceforge-compilefarm-generic HOST="$${host}" ; \
# 	done ; \
# 	#
#  
# test-sourceforge: .update-sourceforge-compilefarm
# 	-rm -f log.sourceforge.*
# 	$(MAKE) test-sourceforge-compilefarm 2>&1 | tee log.sourceforge.txt

###############################################################################

beep:
	@printf ''
	@sleep 1
	@printf ''
	@sleep 1
	@printf ''

extract-deb: bin-pkgs
	for this_file in $(shell ls -1 $(PKG_DIR)/); do \
		this_name=$$(dpkg-deb --field "$${this_file}" Package) ; \
		this_vers=$$(dpkg-deb --field "$${this_file}" Version) ; \
		this_arch=$$(dpkg-deb --field "$${this_file}" Architecture) ; \
		this_dir="$(PKG_DIR)/$${this_name}_$${this_vers}_$${this_arch}" ; \
		rm -fr "$${this_dir}" || true ; \
		mkdir -p "$${this_dir}" || exit 1 ; \
		dpkg-deb -x "$${this_file}" "$${this_dir}" || exit 1 ; \
		dpkg-deb -e "$${this_file}" "$${this_dir}/DEBIAN" || exit 1 ; \
	done ; \
	#

