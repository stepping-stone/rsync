
noinst_LIBRARIES = libcompat.a

libcompat_a_SOURCES = \
	lib/wildmatch.c lib/compat.c lib/snprintf.c lib/mdfour.c lib/md5.c \
	lib/permstring.c lib/pool_alloc.c lib/sysacls.c lib/sysxattrs.c
libcompat_a_LIBADD = $(LIBOBJS)

BUILT_SOURCES = proto.h rounding.h

bin_PROGRAMS = rsync

rsync_SOURCES = \
	flist.c rsync.c generator.c receiver.c cleanup.c sender.c exclude.c \
	util.c util2.c main.c checksum.c match.c syscall.c log.c backup.c delete.c \
	options.c io.c compat.c hlink.c token.c uidlist.c socket.c hashtable.c \
	fileio.c batch.c clientname.c chmod.c acls.c xattrs.c \
	progress.c pipe.c \
	params.c loadparm.c clientserver.c access.c connection.c authenticate.c
rsync_LDADD = libcompat.a

# Test targets
tls_SOURCES = tls.c syscall.c lib/compat.c lib/snprintf.c lib/permstring.c lib/sysxattrs.c
testrun_SOURCES = testrun.c
getgroups_SOURCES = getgroups.c
getfsdev_SOURCES = getfsdev.c
trimslash_SOURCES = trimslash.c syscall.c lib/compat.c lib/snprintf.c
t_unsafe_SOURCES = t_unsafe.c syscall.c util.c util2.c t_stub.c lib/compat.c lib/snprintf.c lib/wildmatch.c

noinst_PROGRAMS = tls testrun getgroups getfsdev trimslash t_unsafe
TESTS = runtests.sh

if BUILD_ZLIB
noinst_LIBRARIES += libz.a
libz_a_SOURCES = \
	zlib/deflate.c zlib/inffast.c zlib/inflate.c zlib/inftrees.c \
	zlib/trees.c zlib/zutil.c zlib/adler32.c zlib/compress.c zlib/crc32.c

rsync_LDADD += libz.a
endif

if BUILD_POPT
noinst_LIBRARIES += libpopt.a
libpopt_a_SOURCES = \
	popt/findme.c popt/popt.c popt/poptconfig.c \
	popt/popthelp.c popt/poptparse.c

rsync_LDADD += libpopt.a
tls_LDADD = libpopt.a
endif

bin_SCRIPTS = rsync-ssl stunnel-rsync
EXTRA_DIST = rsync-ssl.in stunnel-rsync.in stunnel-rsyncd.conf.in
CLEANFILES = $(bin_SCRIPTS) stunnel-rsyncd.conf proto.h proto.h-tstamp rounding.h

# The following two targets follow a common idiom for provisioning script files

edit = sed \
	-e 's|@stunnel4[@]|$(stunnel4)|g' \
	-e 's|@bindir[@]|$(bindir)|g' \
	-e 's|@prefix[@]|$(prefix)|g'

$(bin_SCRIPTS): Makefile
	rm -f $@ $@.tmp
	srcdir=''; \
		test -f ./$@.in || srcdir=$(srcdir)/; \
		$(edit) $${srcdir}$@.in >$@.tmp
	chmod +x $@.tmp
	chmod go-w $@.tmp
	mv $@.tmp $@

stunnel-rsyncd.conf: Makefile
	rm -f $@ $@.tmp
	srcdir=''; \
		test -f ./$@.in || srcdir=$(srcdir)/; \
		$(edit) $${srcdir}$@.in >$@.tmp
	chmod go-w $@.tmp
	mv $@.tmp $@

rsync-ssl: $(srcdir)/rsync-ssl.in
stunnel-rsync: $(srcdir)/stunnel-rsync.in
stunnel-rsyncd.conf: $(srcdir)/stunnel-rsyncd.conf.in

proto: proto.h-tstamp

proto.h: proto.h-tstamp
	$(AM_V_GEN)if test -f proto.h; then :; else cp -p $(srcdir)/proto.h .; fi

proto.h-tstamp: $(srcdir)/*.c $(srcdir)/lib/compat.c config.h
	$(AM_V_GEN)$(PERL) $(srcdir)/mkproto.pl $(srcdir)/*.c $(srcdir)/lib/compat.c

rounding.h: rounding.c rsync.h
	$(AM_V_GEN)for r in 0 1 3; do \
		if $(COMPILE) -o rounding -DEXTRA_ROUNDING=$$r -I. $(srcdir)/rounding.c >rounding.out 2>&1; then \
		echo "#define EXTRA_ROUNDING $$r" >rounding.h; \
		if test -f "$$HOME/build_farm/build_test.fns"; then \
			echo "EXTRA_ROUNDING is $$r" >&2; \
		fi; \
		break; \
		fi; \
	done
	$(AM_V_GEN)rm -f rounding
	$(AM_V_GEN)if test -f rounding.h; then : ; else \
		cat rounding.out 1>&2; \
		echo "Failed to create rounding.h!" 1>&2; \
		exit 1; \
	fi
	$(AM_V_GEN)rm -f rounding.out

