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
#include <aks_file_enumerator.h>
#include <aks_file_info.h>
#include <aks_file_private.h>

/*
 * Object definition
 *
 */

struct _AksFileEnumerator
{
  GFileEnumerator parent_instance;

  /*<private>*/
  FileNode* node;
  GFileAttributeMatcher* matcher;
  GFileQueryInfoFlags flags;
};

G_DEFINE_TYPE
(AksFileEnumerator,
 aks_file_enumerator,
 G_TYPE_FILE_ENUMERATOR);

static GFileInfo*
aks_file_enumerator_class_next_file(GFileEnumerator  *pself,
                                    GCancellable     *cancellable,
                                    GError          **error)
{
  AksFileEnumerator* self = AKS_FILE_ENUMERATOR(pself);
  GFileInfo* info = NULL;
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  FileNode* node = self->node;
  if G_LIKELY(node != NULL)
    self->node =
    self->node->next;
  else
    return NULL;

  if G_UNLIKELY
    (node->data->entry == NULL)
  {
    g_set_error
    (error,
     G_IO_ERROR,
     G_IO_ERROR_INVAL,
     "invalid file\r\n");
    goto_error();
  }

  info =
  _aks_file_info_get
  (node->data->entry,
   self->matcher,
   self->flags,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

_error_:
  if G_UNLIKELY(success == FALSE)
    g_clear_object(&info);
return info;
}

static gboolean
aks_file_enumerator_class_close_fn(GFileEnumerator *enumerator,
                                   GCancellable    *cancellable,
                                   GError         **error)
{
return TRUE;
}

static
void aks_file_enumerator_class_dispose(GObject* pself) {
  AksFileEnumerator* self = AKS_FILE_ENUMERATOR(pself);

/*
 * Dispose
 *
 */
  g_clear_pointer(&(self->matcher), g_file_attribute_matcher_unref);

/*
 * Chain-up
 *
 */
  G_OBJECT_CLASS(aks_file_enumerator_parent_class)->dispose(pself);
}

static
void aks_file_enumerator_class_init(AksFileEnumeratorClass* klass) {
  GFileEnumeratorClass* eclass = G_FILE_ENUMERATOR_CLASS(klass);
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

/*
 * vtable
 *
 */
  eclass->next_file = aks_file_enumerator_class_next_file;
  eclass->close_fn = aks_file_enumerator_class_close_fn;
  oclass->dispose = aks_file_enumerator_class_dispose;
}

static
void aks_file_enumerator_init(AksFileEnumerator* self) {
}

/*
 * Object methods
 *
 */

GFileEnumerator*
_aks_file_enumerator_new(AksFile *file_,
                         const char *attributes,
                         GFileQueryInfoFlags flags,
                         GCancellable *cancellable,
                         GError **error)
{
  AksFile* file = (AksFile*)
  g_file_dup(G_FILE(file_));

  AksFileEnumerator* thi5 =
  g_object_new
  (AKS_TYPE_FILE_ENUMERATOR,
   "container", file_,
   NULL);

  thi5->node = file->current->children;
  thi5->matcher = g_file_attribute_matcher_new(attributes);
  thi5->flags = flags;

  g_object_unref(file);
return G_FILE_ENUMERATOR(thi5);
}
