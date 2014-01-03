conf: configure.sh config.h.in

aclocal.m4:
	aclocal

configure.sh: configure.ac aclocal.m4
	autoconf -o configure.sh

config.h.in: configure.ac aclocal.m4
	autoheader && touch config.h.in
