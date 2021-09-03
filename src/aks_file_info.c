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
#include <config.h>
#include <aks_file_info.h>
#include <aks_file_private.h>
#include <inttypes.h>

static void
set_file_type(GFileInfo              *info,
              struct archive_entry   *entry)
{
  mode_t mode = (mode_t)
  archive_entry_mode(entry);
  GFileType type = G_FILE_TYPE_UNKNOWN;

  if(__S_ISTYPE(mode, AE_IFREG))
    type = G_FILE_TYPE_REGULAR;
  else
  if(__S_ISTYPE(mode, AE_IFLNK))
    type = G_FILE_TYPE_SYMBOLIC_LINK;
  else
  if(__S_ISTYPE(mode, AE_IFDIR))
    type = G_FILE_TYPE_DIRECTORY;
  else
  if(__S_ISTYPE(mode, AE_IFSOCK)
     && __S_ISTYPE(mode, AE_IFIFO)
     && __S_ISTYPE(mode, AE_IFBLK)
     && __S_ISTYPE(mode, AE_IFCHR))
    type = G_FILE_TYPE_SPECIAL;

  if(type == G_FILE_TYPE_SYMBOLIC_LINK)
    g_file_info_set_attribute_byte_string
    (info,
     G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET,
     (const char*)
     archive_entry_symlink(entry));

  g_file_info_set_attribute_uint32
  (info,
   G_FILE_ATTRIBUTE_STANDARD_TYPE,
   (gint32) type);
}

GFileInfo*
_aks_file_info_get(struct archive_entry      *entry,
                   GFileAttributeMatcher     *matcher,
                   GFileAttributeInfoFlags    flags,
                   GError                   **error)
{
  GFileInfo* info = NULL;
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  gchar* basename =
  g_path_get_basename
  (archive_entry_pathname_utf8(entry));

/*
 * Prepare info
 *
 */

  info =
  g_file_info_new();

  g_file_info_set_attribute_mask
  (info,
   matcher);

/*
 * standard::* info
 *
 */
  g_file_info_set_attribute_uint64
  (info,
   G_FILE_ATTRIBUTE_STANDARD_ALLOCATED_SIZE,
   (guint64)
   archive_entry_size(entry));

  g_file_info_set_attribute_string
  (info,
   G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
   "text/html");

  gchar* basename_utf8 =
  g_utf8_make_valid(basename, -1);

  g_file_info_set_attribute_string
  (info,
   G_FILE_ATTRIBUTE_STANDARD_COPY_NAME,
   basename_utf8);

  g_file_info_set_attribute_string
  (info,
   G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
   basename_utf8);

  g_file_info_set_attribute_string
  (info,
   G_FILE_ATTRIBUTE_STANDARD_EDIT_NAME,
   basename_utf8);

  g_free(basename_utf8);

  g_file_info_set_attribute_boolean
  (info,
   G_FILE_ATTRIBUTE_STANDARD_IS_BACKUP,
   FALSE);

  g_file_info_set_attribute_boolean
  (info,
   G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN,
   FALSE);

  g_file_info_set_attribute_boolean
  (info,
   G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK,
   archive_entry_mode(entry) & AE_IFLNK);

  g_file_info_set_attribute_boolean
  (info,
   G_FILE_ATTRIBUTE_STANDARD_IS_VIRTUAL,
   TRUE);

  g_file_info_set_attribute_boolean
  (info,
   G_FILE_ATTRIBUTE_STANDARD_IS_VOLATILE,
   FALSE);

  g_file_info_set_attribute_byte_string
  (info,
   G_FILE_ATTRIBUTE_STANDARD_NAME,
   basename);

  g_file_info_set_attribute_uint64
  (info,
   G_FILE_ATTRIBUTE_STANDARD_SIZE,
   (guint64)
   archive_entry_size(entry));

  set_file_type(info, entry);
  g_free(basename);

/*
 * time::* info
 *
 */

  g_file_info_set_attribute_uint64
  (info,
   G_FILE_ATTRIBUTE_TIME_ACCESS,
   (guint64)
   archive_entry_atime(entry));
  g_file_info_set_attribute_uint32
  (info,
   G_FILE_ATTRIBUTE_TIME_ACCESS_USEC,
   (guint32)
   archive_entry_atime_nsec(entry));

  g_file_info_set_attribute_uint64
  (info,
   G_FILE_ATTRIBUTE_TIME_CREATED,
   (guint64)
   archive_entry_birthtime(entry));
  g_file_info_set_attribute_uint32
  (info,
   G_FILE_ATTRIBUTE_TIME_CREATED_USEC,
   (guint32)
   archive_entry_birthtime_nsec(entry));

  g_file_info_set_attribute_uint64
  (info,
   G_FILE_ATTRIBUTE_TIME_CHANGED,
   (guint64)
   archive_entry_ctime(entry));
  g_file_info_set_attribute_uint32
  (info,
   G_FILE_ATTRIBUTE_TIME_CHANGED_USEC,
   (guint32)
   archive_entry_ctime_nsec(entry));

  g_file_info_set_attribute_uint64
  (info,
   G_FILE_ATTRIBUTE_TIME_MODIFIED,
   (guint64)
   archive_entry_mtime(entry));
  g_file_info_set_attribute_uint32
  (info,
   G_FILE_ATTRIBUTE_TIME_MODIFIED_USEC,
   (guint32)
   archive_entry_mtime_nsec(entry));

_error_:
  if G_LIKELY(success == TRUE)
    g_file_info_unset_attribute_mask(info);
  else
    g_clear_object(&info);
return info;
}
