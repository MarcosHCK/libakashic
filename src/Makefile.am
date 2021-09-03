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
	aks_file_info.h \
	aks_file_private.h \
	aks_stream.h \
	$(VOID)

#
# Source files where
# at least an enum is defined
#
ENUM_FILES=\
	aks_enums.h \
	$(VOID)

aks_enums.c: $(ENUM_FILES) aks_enums.c.template
	$(GLIB_MKENUMS) --symbol-prefix "aks" \
									--identifier-prefix "Aks" \
									--template "aks_enums.c.template" \
									--output $@ \
									$(ENUM_FILES)

#
# Installable headers
#

pkginclude_HEADERS=\
	aks_enums.h \
	aks_file.h \
	libakashic.h \
	$(VOID)

#
# Binaries and libraries
#

lib_LTLIBRARIES=libakashic.la

libakashic_la_SOURCES=\
	aks_archive.c \
	aks_enums.c \
	aks_file.c \
	aks_file_enumerator.c \
	aks_file_iface.c \
	aks_file_info.c \
	aks_file_node.c \
	aks_stream.c \
	$(VOID)

libakashic_la_CFLAGS=\
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBARCHIVE_CFLAGS) \
	$(VOID)

libakashic_la_LIBADD=\
	$(GIO_LIBS) \
	$(GLIB_LIBS) \
	$(LIBARCHIVE_LIBS) \
	$(VOID)

libakashic_la_LDFLAGS=\
	-rpath ${libdir} \
	-release ${PACKAGE_RELEASE} \
	-version-info ${PACKAGE_VERSION_MAYOR}:${PACKAGE_VERSION_MINOR}:${PACKAGE_VERSION_MICRO} \
	$(VOID)
