#!/bin/make
# @(#)Makefile	1.2 04 May 1995 02:06:57
#
# vnwhite - Von Neumann whitener
#
# @(#) $Revision: 1.1 $
# @(#) $Id: Makefile,v 1.1 2006/03/17 07:54:39 chongo Exp $
# @(#) $Source: /usr/local/src/bin/vnwhite/RCS/Makefile,v $
#
# Copyright (c) 2004 by Landon Curt Noll.  All Rights Reserved.
#
# Permission to use, copy, modify, and distribute this software and
# its documentation for any purpose and without fee is hereby granted,
# provided that the above copyright, this permission notice and text
# this comment, and the disclaimer below appear in all of the following:
#
#       supporting documentation
#       source copies
#       source works derived from this source
#       binaries derived from this source or from derived source
#
# LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
# EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
# USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
#
# chongo (Landon Curt Noll, http://www.isthe.com/chongo/index.html) /\oo/\
#
# Share and enjoy! :-)


SHELL= /bin/sh
CC= cc
CFLAGS= -O3 -g3 -Wall -W
RM= rm

TOPNAME= cmd
INSTALL= install

DESTDIR= /usr/local/bin

TARGETS= vnwhite

all: ${TARGETS}

vnwhite: vnwhite.c
	${CC} ${CFLAGS} vnwhite.c -o vnwhite

configure:
	@echo nothing to configure

clean quick_clean quick_distclean distclean:
	${RM} -f vnwhite.o

clobber quick_clobber: clean
	${RM} -f vnwhite

install: all
	${INSTALL} -m 0555 ${TARGETS} ${DESTDIR}
