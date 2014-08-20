conf: configure.sh config.h.in

aclocal.m4:
	aclocal

Makefile.in: Makefile.am
	automake

configure.sh: configure.ac aclocal.m4 Makefile.in
	autoconf -o configure.sh

config.h.in: configure.ac aclocal.m4
	autoheader && touch config.h.in
