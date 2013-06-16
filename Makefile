#
# Copyright (c) 2012 Hans Petter Selasky. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
#
# Makefile for virtual_oss_ctl
#

VERSION=1.0.3
PACKAGE=virtual_oss_ctl-${VERSION}
EXT_HEADER=../virtual_oss/virtual_oss.h

PREFIX?=/usr/local

all: Makefile.unix virtual_oss.h
	make -f Makefile.unix -j2 all

Makefile.unix: virtual_oss_ctl.pro
	qmake-qt4 PREFIX=${PREFIX} -o Makefile.unix virtual_oss_ctl.pro

help:
	@echo "Targets are: all, install, clean, package, help"

.if exists(${EXT_HEADER})
virtual_oss.h: ${EXT_HEADER}
	cp -v ${.ALLSRC} ${.TARGET}
.endif

install: Makefile.unix
	make -f Makefile.unix install

clean: Makefile.unix
	make -f Makefile.unix clean

package: clean virtual_oss.h
	tar -cvf ${PACKAGE}.tar \
		Makefile virtual_oss*.pro virtual_oss*.qrc \
		virtual_oss*.cpp virtual_oss*.h virtual_oss*.png \
		virtual_oss*.desktop
	rm -rf ${PACKAGE}
	mkdir ${PACKAGE}
	tar -xvf ${PACKAGE}.tar -C ${PACKAGE}
	rm -rf ${PACKAGE}.tar
	tar -jcvf ${PACKAGE}.tar.bz2 ${PACKAGE}
