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

G_DEFINE_QUARK(aks-file-error-quark,
               aks_file_error);

void _aks_file_g_file_iface_init(GFileIface* iface);
static
void aks_file_g_initiable_iface_init(GInitableIface* iface);
static
void aks_file_g_async_initable_iface_init(GAsyncInitableIface* iface);

/*
 * Object definition
 *
 */

enum {
  prop_0,
  prop_dup,
  prop_base_stream,
  prop_cache_level,
  prop_filename,
  prop_number,
};

static
GParamSpec* properties[prop_number] = {0};

static FileNode*
search_node_for_file(AksFile   *self,
                     GFile     *full_,
                     gboolean   make,
                     FileNode **last_)
{
  GFile* parent = g_file_get_parent(full_);
  if G_LIKELY(parent != NULL)
  {
/*
 * Get parent's node
 *
 */
    FileNode *children, *node =
    search_node_for_file(self, parent, make, last_);
    if G_UNLIKELY(node == NULL)
    {
      g_object_unref(parent);
      return NULL;
    }
    else
    {
      if G_UNLIKELY(last_ != NULL)
        *last_ = node;
    }

/*
 * Iterate children to find
 * next node
 *
 */
    gchar* name = g_file_get_relative_path(parent, full_);
    FileNodeHash hash_ = g_str_hash(name);
    g_object_unref(parent);
    FileNodeData* data;

    for(children = node->children;
        children != NULL;
        children = children->next)
    {
      data = children->data;
      if G_UNLIKELY(data->hash_ == hash_)
      {
        gboolean matches =
        g_str_equal(data->name, name);
        if G_UNLIKELY(matches == TRUE)
        {
          g_free(name);
          return children;
        }
      }
    }

/*
 * Wanted children doesn't exists
 * if creation is allowed, make
 * it brand-new
 *
 */
    if G_UNLIKELY(make == TRUE)
    {
      data = _aks_node_data_new();
      children = (FileNode*) g_node_new(data);
      children->data = data;

      data->name = g_strdup(name);
      data->hash_ = g_str_hash(data->name);
      g_free(name);

    /*
     * Append node
     *
     */
      g_node_append
      (&(node->node_),
       &(children->node_));
      return children;
    }

    g_free(name);
  }
  else
  {
/*
 * No parents means root
 *
 */
    return self->root;
  }
return NULL;
}

#if DEBUG

static void
print_entries_inner(FileNode *node,
                    gint     *plevel)
{
  gint i, level = plevel[0];
  int fd = fileno(stderr);

  /* One space character plus three UTF-8 sequence characters */
  /* Then, add six UTF-8 sequence characters (two three bytes */
  /* long sequences ) and a final space character             */
  GString* string_ = g_string_sized_new((level << 2) + 6 + 1);

  for(i = 0;i < level;i++)
  {
    FileNode* thi5 = node;
    gint j;

    for(j = 0;j < (level - i);j++)
      thi5 = thi5->parent;

    if(thi5->next == NULL)
    {
      g_string_append_c(string_, ' ');
      g_string_append_c(string_, ' ');
    }
    else
    {
      g_string_append_c(string_, 0xe2);
      g_string_append_c(string_, 0x94);
      g_string_append_c(string_, 0x82);
      g_string_append_c(string_, ' ');
    }
  }

  if(level > 0)
    if(node->next == NULL)
      g_string_append(string_, "\xe2\x94\x94" "\xe2\x94\x80");
    else
      g_string_append(string_, "\xe2\x94\x9c" "\xe2\x94\x80");

  g_string_append_c(string_, ' ');

  write(fd, string_->str, string_->len);
  g_string_assign(string_, node->data->name);
  write(fd, string_->str, string_->len);
  write(fd, "\r\n", 2);

  g_string_free(string_, TRUE);

  int level_ = level + 1;
  g_node_children_foreach
  (&(node->node_),
   G_TRAVERSE_ALL,
   (GNodeForeachFunc)
   print_entries_inner,
   &level_);
}

static void
print_entries(FileNode* node) {
  int level = 0;
  print_entries_inner(node, &level);
}

#endif // DEBUG

G_DEFINE_TYPE_WITH_CODE
(AksFile,
 aks_file,
 G_TYPE_OBJECT,
 G_IMPLEMENT_INTERFACE
 (G_TYPE_FILE,
  _aks_file_g_file_iface_init)
 G_IMPLEMENT_INTERFACE
 (G_TYPE_INITABLE,
  aks_file_g_initiable_iface_init)
 G_IMPLEMENT_INTERFACE
 (G_TYPE_ASYNC_INITABLE,
  aks_file_g_async_initable_iface_init)
 );

static void
init_fn(GTask* task,
        AksFile* self,
        gpointer task_data,
        GCancellable* cancellable)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

/*
 * Skip initialization if
 * this object is a copy
 *
 */
  if(self->dup == TRUE)
    goto _error_;

/*
 * Prepare object
 *
 */
  if(self->cache_level == AKS_CACHE_LEVEL_NONE
     || self->cache_level == AKS_CACHE_LEVEL_OTF)
  {
    if G_UNLIKELY
      (G_IS_SEEKABLE(self->base_stream) == FALSE
       || g_seekable_can_seek(G_SEEKABLE(self->base_stream)) == FALSE)
    {
      g_task_return_new_error
      (task,
       AKS_FILE_ERROR,
       AKS_FILE_ERROR_UNSEEKABLE_INPUT,
       "Seekable input needed\r\n");
      goto_error();
    }

    self->start_position =
    g_seekable_tell(G_SEEKABLE(self->base_stream));
  }

/*
 * Create exploration archive object
 *
 */
  struct archive* ar = NULL;

  ar =
  _aks_archive_read_make
  (G_OBJECT(self),
   self->base_stream,
   cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_task_return_error(task, tmp_err);
    if G_UNLIKELY(ar != NULL)
    _aks_archive_read_free
    (G_OBJECT(self),
     ar);
    goto_error();
  }

  g_assert(ar != NULL);
  struct archive_entry* entry;

/*
 * Explore archive
 *
 */
  for(;;)
  {
    int return_ =
    archive_read_next_header(ar, &entry);
    if G_UNLIKELY(return_ < 0)
    {
      g_task_return_error
      (task,
       _aks_archive_get_gerror
       (G_OBJECT(self),
        ar));
      goto_error();
    } else
    if(return_ == ARCHIVE_EOF)
    {
      break;
    } else
    if G_UNLIKELY(return_ != ARCHIVE_OK)
    {
      g_task_return_new_error
      (task,
       AKS_FILE_ERROR,
       AKS_FILE_ERROR_FAILED,
       "%s: " G_STRINGIFY(__LINE__) ": "
       "unknown return code '%i'\r\n",
       G_STRFUNC, return_);
      goto_error();
    }

  /*
   * Get node for this entry
   *
   */
    const gchar* name =
    archive_entry_pathname_utf8(entry);

    GFile* full_ =
    g_file_new_build_filename
    ("/", name, NULL);

    FileNode* thi5 =
    search_node_for_file
    (self,
     full_,
     TRUE,
     NULL);

    FileNodeData* data = thi5->data;
    g_object_unref(full_);

  /*
   * Copy needed data
   *
   */
    if(self->cache_level == AKS_CACHE_LEVEL_FULL)
    {
      data->cache =
      _aks_archive_dump_to_bytes
      (G_OBJECT(self),
       ar,
       cancellable,
       &tmp_err);

      if G_UNLIKELY(tmp_err != NULL)
      {
        g_task_return_error(task, tmp_err);
        goto_error();
      }
    }
    else
    {
      data->entry = archive_entry_clone(entry);
    }
  }

#if DEBUG
  print_entries(self->root);
#endif // DEBUG

_error_:
  if G_LIKELY(success == TRUE)
    g_task_return_boolean(task, TRUE);
  else
  if G_UNLIKELY(ar != NULL)
    _aks_archive_read_free(G_OBJECT(self), ar);
}

static
gboolean aks_file_g_async_initable_iface_init_sync(GInitable* pself, GCancellable* cancellable, GError** error) {
  GTask* task =
  g_task_new
  (pself,
   cancellable,
   NULL, NULL);

  g_task_set_name(task, "[libakashic] AksFile::init");
  g_task_set_priority(task, G_PRIORITY_DEFAULT);
  g_task_run_in_thread_sync(task, (GTaskThreadFunc) init_fn);
  gboolean return_ = g_task_propagate_boolean(task, error);
  g_object_unref(task);
return return_;
}

static
void aks_file_g_initiable_iface_init(GInitableIface* iface) {
  iface->init = aks_file_g_async_initable_iface_init_sync;
}

static
void aks_file_g_async_initable_iface_init_async(GAsyncInitable* pself, int io_priority, GCancellable* cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GTask* task =
  g_task_new
  (pself,
   cancellable,
   callback,
   user_data);

  g_task_set_name(task, "[libakashic] AksFile::init_async");
  g_task_set_priority(task, io_priority);
  g_task_run_in_thread(task, (GTaskThreadFunc) init_fn);
  g_object_unref(task);
}

static
gboolean aks_file_g_async_initable_iface_init_finish(GAsyncInitable* pself, GAsyncResult* res, GError** error) {
  return g_task_propagate_boolean(G_TASK(res), error);
}

static
void aks_file_g_async_initable_iface_init(GAsyncInitableIface* iface) {
  iface->init_async = aks_file_g_async_initable_iface_init_async;
  iface->init_finish = aks_file_g_async_initable_iface_init_finish;
}

static
void aks_file_class_get_property(GObject* pself, guint prop_id, GValue* value, GParamSpec* pspec) {
  AksFile* self = AKS_FILE(pself);
  switch(prop_id)
  {
  case prop_base_stream:
    g_value_set_object(value, self->base_stream);
    break;
  case prop_cache_level:
    g_value_set_enum(value, self->cache_level);
    break;
  case prop_filename:
    g_value_set_string(value, g_file_peek_path(G_FILE(self)));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static
void aks_file_class_set_property(GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec) {
  AksFile* self = AKS_FILE(pself);
  switch(prop_id)
  {
  case prop_dup:
    self->dup = g_value_get_boolean(value);
    break;
  case prop_base_stream:
    g_set_object(&(self->base_stream), g_value_get_object(value));
    break;
  case prop_cache_level:
    self->cache_level = g_value_get_enum(value);
    break;
  case prop_filename:
    {
      GFile* full_ =
      g_file_new_for_path
      (g_value_get_string(value));

      FileNode* node =
      search_node_for_file(self, full_, FALSE, NULL);
      g_object_unref(full_);
      self->current = node;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static
void aks_file_class_finalize(GObject* pself) {
  AksFile* self = AKS_FILE(pself);

/*
 * Finalize
 *
 */
  g_node_destroy(&(self->root->node_));

/*
 * Chain-up
 *
 */
  G_OBJECT_CLASS(aks_file_parent_class)->finalize(pself);
}

static void
dispose_node(FileNode* node) {

/*
 * Unref data
 *
 */
  g_clear_pointer
  (&(node->data),
   _aks_node_data_unref);

/*
 * Dispose node children
 *
 */
  g_node_children_foreach
  (&(node->node_),
   G_TRAVERSE_ALL,
   (GNodeForeachFunc)
   dispose_node,
   NULL);
}

static
void aks_file_class_dispose(GObject* pself) {
  AksFile* self = AKS_FILE(pself);

/*
 * Dispose
 *
 */
  g_clear_object(&(self->base_stream));
  dispose_node(self->root);

/*
 * Chain-up
 *
 */
  G_OBJECT_CLASS(aks_file_parent_class)->dispose(pself);
}

static
void aks_file_class_init(AksFileClass* klass) {
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

/*
 * vtable
 *
 */
  oclass->get_property = aks_file_class_get_property;
  oclass->set_property = aks_file_class_set_property;
  oclass->finalize = aks_file_class_finalize;
  oclass->dispose = aks_file_class_dispose;

/*
 * Properties
 *
 */
  properties[prop_dup] =
    g_param_spec_boolean("dup",
                         "dup",
                         "dup",
                         FALSE,
                         G_PARAM_WRITABLE
                         | G_PARAM_CONSTRUCT_ONLY
                         | G_PARAM_STATIC_STRINGS);

  properties[prop_base_stream] =
    g_param_spec_object("base-stream",
                        "base-stream",
                        "base-stream",
                        G_TYPE_INPUT_STREAM,
                        G_PARAM_READWRITE
                        | G_PARAM_CONSTRUCT
                        | G_PARAM_STATIC_STRINGS);

  properties[prop_cache_level] =
    g_param_spec_enum("cache-level",
                      "cache-level",
                      "cache-level",
                      AKS_TYPE_CACHE_LEVEL,
                      AKS_CACHE_LEVEL_OTF,
                      G_PARAM_READWRITE
                      | G_PARAM_CONSTRUCT
                      | G_PARAM_STATIC_STRINGS);

  properties[prop_filename] =
    g_param_spec_string("filename",
                        "filename",
                        "filename",
                        "/",
                        G_PARAM_READWRITE
                        | G_PARAM_CONSTRUCT
                        | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(oclass,
                                    prop_number,
                                    properties);
}

static
void aks_file_init(AksFile* self) {
/*
 * Prepare root
 *
 */
  FileNodeData* data =
  _aks_node_data_new();
  FileNode* root = (FileNode*)
  g_node_new(data);
  root->data = data;

  data->name = g_strdup("/");
  data->hash_ = g_str_hash(data->name);
  data->entry = NULL;
  self->root = root;
}

/*
 * Object methods
 *
 */

GFile*
aks_file_new(GInputStream  *base_stream,
             AksCacheLevel  cache_level,
             GCancellable  *cancellable,
             GError       **error)
{
  return (GFile*)
  g_initable_new
  (AKS_TYPE_FILE,
   cancellable,
   error,
   "base-stream", base_stream,
   "cache-level", cache_level,
   NULL);
}

void
aks_file_new_async(GInputStream        *base_stream,
                   AksCacheLevel        cache_level,
                   int                  io_priority,
                   GCancellable        *cancellable,
                   GAsyncReadyCallback  callback,
                   gpointer             user_data)
{
  g_async_initable_new_async
  (AKS_TYPE_FILE,
   io_priority,
   cancellable,
   callback,
   user_data,
   "base-stream", base_stream,
   "cache-level", cache_level,
   NULL);
}

GFile*
aks_file_new_finish(GAsyncResult   *res,
                    GError        **error)
{
  return (GFile*)
  g_async_initable_new_finish
  (g_task_get_source_object
   (G_TASK(res)),
   res,
   error);
}
