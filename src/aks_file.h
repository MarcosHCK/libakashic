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
#ifndef __LIBAKASHIC_AKS_FILE__
#define __LIBAKASHIC_AKS_FILE__
#include <aks_enums.h>
#include <gio/gio.h>

/**
 * AKS_FILE_ERROR:
 *
 * Error domain for AKS_FILE. Errors in this domain will be from the #AksFileError enumeration.
 * See #GError for more information on error domains.
 */
#define AKS_FILE_ERROR (aks_file_error_quark())

/**
 * AksFileError:
 * @AKS_FILE_ERROR_FAILED: generic error condition.
 * @AKS_FILE_ERROR_UNSEEKABLE_INPUT: error emitted when a
 * seekable base stream is needed. Note: seekable streams
 * are only needed if archive is in zip format (which put
 * master directory entry at archive's end) or if full caching
 * is not enabled.
 * @AKS_FILE_ERROR_FILE_NOT_FOUND: self explainable.
 * @AKS_FILE_ERROR_INVALID_FILE: the entry represents a
 * non-file entity, such as a folder.
 * @AKS_FILE_ERROR_ARCHIVE: base error code for libarchive
 * specific errors.
 *
 * Error code returned by #AksFile object API.
 * Note that %AKS_FILE_ERROR_FAILED is here only for compatibility with
 * error domain definition paradigm as defined on GLib documentation.
 */
typedef enum {
  AKS_FILE_ERROR_FAILED,
  AKS_FILE_ERROR_UNSEEKABLE_INPUT,
  AKS_FILE_ERROR_FILE_NOT_FOUND,
  AKS_FILE_ERROR_INVALID_FILE,
  AKS_FILE_ERROR_ARCHIVE,
} AksFileError;

#define AKS_TYPE_FILE             (aks_file_get_type ())
#define AKS_FILE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AKS_TYPE_FILE, AksFile))
#define AKS_FILE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), AKS_TYPE_FILE, AksFileClass))
#define AKS_IS_FILE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AKS_TYPE_FILE))
#define AKS_IS_FILE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), AKS_TYPE_FILE))
#define AKS_FILE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AKS_TYPE_FILE, AksFileClass))

typedef struct _AksFile       AksFile;
typedef struct _AksFileClass  AksFileClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
aks_file_get_type();
GQuark
aks_file_error_quark();

struct _AksFileClass
{
  GObjectClass parent_class;
};

GFile*
aks_file_new(GInputStream  *base_stream,
             AksCacheLevel  cache_level,
             const gchar   *filename,
             GCancellable  *cancellable,
             GError       **error);
void
aks_file_new_async(GInputStream        *base_stream,
                   AksCacheLevel        cache_level,
                   const gchar         *filename,
                   int                  io_priority,
                   GCancellable        *cancellable,
                   GAsyncReadyCallback  callback,
                   gpointer             user_data);
GFile*
aks_file_new_finish(GAsyncResult   *res,
                    GError        **error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __LIBAKASHIC_AKS_FILE__
