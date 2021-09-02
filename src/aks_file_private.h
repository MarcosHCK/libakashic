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
#ifndef __LIBAKASHIC_AKS_FILE_PRIVATE__
#define __LIBAKASHIC_AKS_FILE_PRIVATE__
#include <aks_file.h>
#include <archive.h>
#include <archive_entry.h>

typedef union  _FileNode      FileNode;
typedef struct _FileNodeData  FileNodeData;
typedef guint                 FileNodeHash;

#define goto_error() \
G_STMT_START { \
  success = FALSE; \
  goto _error_; \
} G_STMT_END

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
_aks_node_data_get_type();

struct _AksFile
{
  GObject parent_instance;

  /*<private>*/
  GInputStream* base_stream;
  AksCacheLevel cache_level;
  gchar* filename;
  FileNode* current;

  goffset start_position;
  gboolean dup;

  union _FileNode
  {
    GNode node_;
    struct
    {
      struct _FileNodeData
      {
        gchar* name;
        guint hash_;

      /*
       * Cache
       *
       */
        GBytes* cache;
        struct archive_entry* entry;
        grefcount refs;
      } *data;

      FileNode* next;
      FileNode* prev;
      FileNode* parent;
      FileNode* children;
    };
  } *root;
};

FileNodeData*
_aks_node_data_new();
FileNodeData*
_aks_node_data_ref(FileNodeData* data);
void
_aks_node_data_unref(FileNodeData* data);
gboolean
_aks_node_data_equal(FileNodeData* data1,
                     FileNodeData* data2);

void
_aks_archive_set_cancellable(GObject         *source_object,
                             struct archive  *ar,
                             GCancellable    *cancellable);
GError*
_aks_archive_get_gerror(GObject         *source_object,
                        struct archive  *ar);
void
_aks_archive_switch_source_object(GObject         *old_source_object,
                                  GObject         *new_source_object,
                                  struct archive  *ar G_GNUC_UNUSED);
void
_aks_archive_read_free(GObject        *source_object,
                       struct archive *ar);
struct archive*
_aks_archive_read_make(GObject        *source_object,
                       GInputStream   *stream,
                       GCancellable   *cancellable,
                       GError        **error);
gboolean
_aks_archive_read_skip_til_entry(GObject               *source_object,
                                 struct archive        *ar,
                                 struct archive_entry  *til,
                                 GCancellable          *cancellable,
                                 GError               **error);
gboolean
_aks_archive_dump_to_stream(GObject         *source_object,
                            struct archive  *ar,
                            GOutputStream   *stream,
                            GCancellable    *cancellable,
                            GError         **error);
GBytes*
_aks_archive_dump_to_bytes(GObject         *source_object,
                           struct archive  *ar,
                           GCancellable    *cancellable,
                           GError         **error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __LIBAKASHIC_AKS_FILE_PRIVATE__
