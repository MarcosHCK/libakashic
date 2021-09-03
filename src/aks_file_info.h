/*  Copyright 2021-2022 MarcosHCK
 *  This file is part of libakashic.
 *
 *  libakashic is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libakashic is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with libakashic. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __LIBAKASHIC_AKS_FILE_INFO__
#define __LIBAKASHIC_AKS_FILE_INFO__
#include <archive_entry.h>
#include <gio/gio.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

GFileInfo*
_aks_file_info_get(struct archive_entry      *entry,
                   GFileAttributeMatcher     *matcher,
                   GFileAttributeInfoFlags    flags,
                   GError                   **error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __LIBAKASHIC_AKS_FILE_INFO__
