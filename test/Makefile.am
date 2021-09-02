# Copyright 2021-2022 MarcosHCK
# This file is part of libakashic.
#
# libakashic is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# libakashic is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libakashic. If not, see <http://www.gnu.org/licenses/>.
#

#
# Some variables
#

PACKAGE_NAME=@PACKAGE_NAME@
PACKAGE_RELEASE=@PACKAGE_RELEASE@
PACKAGE_VERSION_MAYOR=@PACKAGE_VERSION_MAYOR@
PACKAGE_VERSION_MINOR=@PACKAGE_VERSION_MINOR@
PACKAGE_VERSION_MICRO=@PACKAGE_VERSION_MICRO@
PACKAGE_VERSION_BUILD=@PACKAGE_VERSION_BUILD@

EXTRA_DIST=
VOID=

#
# Header files
#

EXTRA_DIST+=\
	tap-driver.sh \
	$(VOID)

#
# Test suit
#

TEST_LOG_DRIVER=\
	env AM_TAP_AWK='$(AWK)' $(SHELL) \
	$(top_srcdir)/test/tap-driver.sh

TESTS=test1.test

#
# Binaries and libraries
#

check_PROGRAMS=test

test_SOURCES=\
	test.c \
	$(VOID)

test_CFLAGS=\
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	-I${top_builddir}/src/ \
	$(VOID)

test_LDADD=\
	$(GIO_LIBS) \
	$(GLIB_LIBS) \
	-L${top_builddir}/src/ \
	-lakashic \
	$(VOID)
