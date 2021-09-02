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
#include <aks_file_private.h>

static GFile*
aks_file_g_file_iface_dup(GFile* pself) {
  AksFile* self = AKS_FILE(pself);

/*
 * Create duplicate
 * object
 *
 */
  AksFile* dst =
  g_object_new
  (AKS_TYPE_FILE,
   "dup", TRUE,
   NULL);

/*
 * Copy entry tree
 *
 */
  FileNode* root = (FileNode*)
  g_node_copy_deep
  (&(self->root->node_),
   (GCopyFunc)
   _aks_node_data_ref,
   NULL);

/*
 * Copy other data
 *
 */
  dst->base_stream = g_object_ref(self->base_stream);
  dst->cache_level = self->cache_level;
  dst->start_position = self->start_position;
  dst->current = self->current;

return G_FILE(dst);
}

static gboolean
aks_file_g_file_iface_equal(GFile* pself1,
                            GFile* pself2)
{
  AksFile* self1 = AKS_FILE(pself1);
  AksFile* self2 = AKS_FILE(pself2);

  return
  (self1->base_stream == self2->base_stream
   && self1->current == self1->current
   && (self1->current == NULL
       || _aks_node_data_equal
          (self1->current->data,
           self2->current->data)));
}

gchar*
aks_file_g_file_iface_get_basename(GFile* pself) {
  const gchar* path = g_file_peek_path(pself);
return g_path_get_basename(path);
}

GFile*
aks_file_g_file_iface_get_parent(GFile* pself) {
  AksFile* self = AKS_FILE(pself);

  if G_UNLIKELY(self->current == NULL)
    return NULL;
  if G_UNLIKELY(self->current->parent == NULL)
    return NULL;

  AksFile* parent = (AksFile*)g_file_dup(pself);
  parent->current = self->current->parent;
return G_FILE(parent);
}

gchar*
aks_file_g_file_iface_get_parse_name(GFile* pself) {
return g_utf8_make_valid(g_file_peek_path(pself), -1);
}

static void
bringup_path(FileNode* node,
             FileNode* parent_,
             GString* path)
{
  FileNode* parent = node->parent;
  if(parent != parent_)
  {
    g_assert(parent != NULL);

  /*
   * Append parents
   *
   */
    bringup_path(parent, parent_, path);

  /*
   * Append this
   *
   */
    g_string_append(path, node->data->name);
#if G_OS_WINDOWS
    g_string_append_c(path, '\\');
#else // G_OS_WINDOWS
    g_string_append_c(path, '/');
#endif // G_OS_WINDOWS
  }
  else
  {
    g_string_append(path, "/");
  }
}

gchar*
aks_file_g_file_iface_get_path(GFile* pself) {
  AksFile* self = AKS_FILE(pself);

  if G_UNLIKELY(self->current == NULL)
    return NULL;

  GString* path = g_string_sized_new(64);
  bringup_path(self->current, NULL, path);
return g_string_free(path, FALSE);
}

gchar*
aks_file_g_file_iface_get_relative_path(GFile* pself1,
                                        GFile* pself2)
{
  AksFile* self1 = AKS_FILE(pself1);
  AksFile* self2 = AKS_FILE(pself2);

  if G_UNLIKELY
    (self1->current == NULL
     || self2->current == NULL)
    return NULL;

  GString* path = g_string_sized_new(64);
  bringup_path(self2->current, self1->current, path);
return g_string_free(path, FALSE);
}

guint
aks_file_g_file_iface_get_hash(GFile* pself) {
  AksFile* self = AKS_FILE(pself);
return g_str_hash(g_file_peek_path(pself));
}

gboolean
aks_file_g_file_iface_is_native(GFile* pself) {
return FALSE;
}

static GInputStream*
peek_stream(AksFile* self,
            struct archive_entry* entry,
            GCancellable   *cancellable,
            GError        **error)
{
  GInputStream* stream = NULL;
  gboolean success = TRUE;
  GError* tmp_err = NULL;

/*
 * Make archive
 *
 */
  struct archive* ar =
  _aks_archive_read_make
  (G_OBJECT(self),
   self->base_stream,
   cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * Search entry
 *
 */
  _aks_archive_read_skip_til_entry
  (G_OBJECT(self),
   ar,
   entry,
   cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * Open stream
 *
 */
  stream =
  g_object_new(G_TYPE_MEMORY_INPUT_STREAM, NULL);

  _aks_archive_switch_source_object
  (G_OBJECT(self),
   G_OBJECT(stream),
   ar);

_error_:
  if G_UNLIKELY(success == FALSE)
    g_clear_object(&stream);
  if G_UNLIKELY(ar != NULL)
    _aks_archive_read_free
    (G_OBJECT(self),
     ar);
return stream;
}

static GBytes*
peek_bytes(AksFile* self,
           struct archive_entry* entry,
           GCancellable   *cancellable,
           GError        **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GBytes* bytes = NULL;

/*
 * Make archive
 *
 */
  struct archive* ar =
  _aks_archive_read_make
  (G_OBJECT(self),
   self->base_stream,
   cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * Search entry
 *
 */
  _aks_archive_read_skip_til_entry
  (G_OBJECT(self),
   ar,
   entry,
   cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * Dump data
 *
 */
  bytes =
  _aks_archive_dump_to_bytes
  (G_OBJECT(self),
   ar,
   cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

_error_:
  if G_UNLIKELY(success == FALSE)
    g_clear_pointer(&bytes, g_bytes_unref);
  if G_UNLIKELY(ar != NULL)
    _aks_archive_read_free
    (G_OBJECT(self),
     ar);
return bytes;
}

GFileInputStream*
aks_file_g_file_iface_read_fn(GFile          *pself,
                              GCancellable   *cancellable,
                              GError        **error)
{
  AksFile* self = AKS_FILE(pself);
  GInputStream* result = NULL;
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  FileNode* node = self->current;
  if G_UNLIKELY(node == NULL)
  {
    g_set_error
    (error,
     G_IO_ERROR,
     G_IO_ERROR_INVAL,
     "invalid file\r\n");
    goto_error();
  }

  switch(self->cache_level)
  {
  case AKS_CACHE_LEVEL_NONE:
    {
      result =
      peek_stream(self, node->data->entry, cancellable, &tmp_err);
      if G_UNLIKELY(tmp_err != NULL)
      {
        g_propagate_error(error, tmp_err);
        goto_error();
      }
    }
    break;
  case AKS_CACHE_LEVEL_OTF:
  case AKS_CACHE_LEVEL_FULL:
    {
      GBytes* bytes = node->data->cache;
      if G_UNLIKELY(bytes == NULL)
      {
        if G_UNLIKELY
          (self->cache_level == AKS_CACHE_LEVEL_FULL
           || node->data->entry == NULL)
        {
          g_set_error
          (error,
           G_IO_ERROR,
           G_IO_ERROR_INVAL,
           "invalid file\r\n");
          goto_error();
        }

        bytes = peek_bytes(self, node->data->entry, cancellable, &tmp_err);
        if G_UNLIKELY(tmp_err != NULL)
        {
          g_propagate_error(error, tmp_err);
          goto_error();
        }

        node->data->cache = bytes;
      }

      result = (GInputStream*)
      g_memory_input_stream_new_from_bytes(bytes);
    }
    break;
  }

_error_:
  if G_UNLIKELY(success == FALSE)
    g_clear_object(&result);
return (GFileInputStream*) result;
}

void _aks_file_g_file_iface_init(GFileIface* iface) {
  iface->dup = aks_file_g_file_iface_dup;
#if 0
  iface->enumerate_children = aks_file_g_file_iface_enumarate_children;
#endif // 0
  iface->equal = aks_file_g_file_iface_equal;
  iface->get_basename = aks_file_g_file_iface_get_basename;
  iface->get_parent = aks_file_g_file_iface_get_parent;
  iface->get_parse_name = aks_file_g_file_iface_get_parse_name;
  iface->get_path = aks_file_g_file_iface_get_path;
  iface->get_relative_path = aks_file_g_file_iface_get_relative_path;
  iface->hash = aks_file_g_file_iface_get_hash;
  iface->is_native = aks_file_g_file_iface_is_native;
  iface->read_fn = aks_file_g_file_iface_read_fn;
}
