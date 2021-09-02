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
#include <aks_stream.h>

/*
 * Object definition
 *
 */

struct _AksStream
{
  GInputStream parent_instance;

  /*<private>*/
  struct archive* ar;
};

enum {
  prop_0,
  prop_archive,
  prop_number,
};

static
GParamSpec* properties[prop_number] = {0};

G_DEFINE_TYPE
(AksStream,
 aks_stream,
 G_TYPE_INPUT_STREAM);

typedef struct {
  void* buffer;
  gsize count;
} ReadData;

static void
read_data_free(ReadData* data) {
  g_slice_free(ReadData, data);
}

#undef goto_error
#define goto_error() \
G_STMT_START { \
  result = -1; \
  goto _error_; \
} G_STMT_END

static void
read_fn(GTask* task,
        AksStream* self,
        ReadData* data,
        GCancellable* cancellable)
{
  _aks_archive_set_cancellable
  (G_OBJECT(self), self->ar, cancellable);
  gssize result = -1;

  la_ssize_t return_ =
  archive_read_data(self->ar, data->buffer, data->count);
  if G_UNLIKELY(return_ < 0)
  {
    g_task_return_error
    (task,
     _aks_archive_get_gerror
     (G_OBJECT(self),
      self->ar));
    goto_error();
  } else
  {
    result = (gssize) return_;
  }

_error_:
  _aks_archive_set_cancellable
  (G_OBJECT(self), self->ar, NULL);
  if G_LIKELY(result != -1)
    g_task_return_int(task, result);
}

static gssize
aks_stream_class_read_fn(GInputStream* stream,
                         void* buffer,
                         gsize count,
                         GCancellable* cancellable,
                         GError** error)
{
  GTask* task =
  g_task_new
  (stream,
   cancellable,
   NULL, NULL);

  ReadData* data =
  g_slice_new(ReadData);
  data->buffer = buffer;
  data->count = count;

  g_task_set_name(task, "[libakashic] AksStream::read_fn");
  g_task_set_priority(task, G_PRIORITY_DEFAULT);
  g_task_set_task_data(task, data, (GDestroyNotify) read_data_free);
  g_task_run_in_thread_sync(task, (GTaskThreadFunc) read_fn);
  gssize return_ = g_task_propagate_int(task, error);
  g_object_unref(task);
return return_;
}

static void
aks_stream_class_read_async(GInputStream* stream,
                            void* buffer,
                            gsize count,
                            int io_priority,
                            GCancellable* cancellable,
                            GAsyncReadyCallback callback,
                            gpointer user_data)
{
  GTask* task =
  g_task_new
  (stream,
   cancellable,
   callback,
   user_data);

  ReadData* data =
  g_slice_new(ReadData);
  data->buffer = buffer;
  data->count = count;

  g_task_set_name(task, "[libakashic] AksStream::read_async");
  g_task_set_priority(task, io_priority);
  g_task_set_task_data(task, data, (GDestroyNotify) read_data_free);
  g_task_run_in_thread(task, (GTaskThreadFunc) read_fn);
  g_object_unref(task);
}

static
gssize aks_stream_class_read_finish(GInputStream* stream, GAsyncResult* res, GError** error) {
  return g_task_propagate_int(G_TASK(res), error);
}

static
void skip_fn(GTask* task,
             AksStream* self,
             gpointer count__,
             GCancellable* cancellable)
{
  _aks_archive_set_cancellable
  (G_OBJECT(self), self->ar, cancellable);
  gsize count_ = GPOINTER_TO_SIZE(count__);
  gboolean success = 0;
  gssize result = 0;

  char skipb[128];

  for(;count_ != 0;)
  {
    la_ssize_t return_ =
    archive_read_data(self->ar, skipb, MIN(count_, sizeof(skipb)));
    if G_UNLIKELY(return_ < 0)
    {
      g_task_return_error
      (task,
       _aks_archive_get_gerror
       (G_OBJECT(self),
        self->ar));
      goto_error();
    } else
    {
      result += (gssize) return_;
      count_ -= (gsize) return_;
    }
  }

_error_:
  _aks_archive_set_cancellable
  (G_OBJECT(self), self->ar, NULL);
  if G_LIKELY(result != -1)
    g_task_return_int(task, result);
}

static
gssize aks_stream_class_skip(GInputStream* stream, gsize count_, GCancellable* cancellable, GError** error) {
  GTask* task =
  g_task_new
  (stream,
   cancellable,
   NULL, NULL);

  g_task_set_name(task, "[libakashic] AksStream::skip");
  g_task_set_priority(task, G_PRIORITY_DEFAULT);
  g_task_set_task_data(task, GSIZE_TO_POINTER(count_), NULL);
  g_task_run_in_thread_sync(task, (GTaskThreadFunc) skip_fn);
  gssize return_ = g_task_propagate_int(task, error);
  g_object_unref(task);
return return_;
}

static
void aks_stream_class_skip_async(GInputStream* stream, gsize count_, int io_priority, GCancellable* cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  GTask* task =
  g_task_new
  (stream,
   cancellable,
   callback,
   user_data);

  g_task_set_name(task, "[libakashic] AksStream::skip_async");
  g_task_set_priority(task, io_priority);
  g_task_set_task_data(task, GSIZE_TO_POINTER(count_), NULL);
  g_task_run_in_thread(task, (GTaskThreadFunc) skip_fn);
  g_object_unref(task);
}

static
gssize aks_stream_class_skip_finish(GInputStream* stream, GAsyncResult* res, GError** error) {
  return g_task_propagate_int(G_TASK(res), error);
}

static
void aks_stream_class_set_property(GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec) {
  AksStream* self = AKS_STREAM(pself);
  switch(prop_id)
  {
  case prop_archive:
    self->ar = g_value_get_pointer(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static
void aks_stream_class_dispose(GObject* pself) {
  AksStream* self = AKS_STREAM(pself);

/*
 * Dispose
 *
 */
  if G_LIKELY(self->ar != NULL)
  {
    _aks_archive_read_free
    (G_OBJECT(self),
     self->ar);
    self->ar = NULL;
  }

/*
 * Chain-up
 *
 */
  G_OBJECT_CLASS(aks_stream_parent_class)->dispose(pself);
}

static
void aks_stream_class_init(AksStreamClass* klass) {
  GInputStreamClass* iclass = G_INPUT_STREAM_CLASS(klass);
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

/*
 * vtable
 *
 */
  iclass->read_fn = aks_stream_class_read_fn;
  iclass->read_async = aks_stream_class_read_async;
  iclass->read_finish = aks_stream_class_read_finish;
  iclass->skip = aks_stream_class_skip;
  iclass->skip_async = aks_stream_class_skip_async;
  iclass->skip_finish = aks_stream_class_skip_finish;
  oclass->set_property = aks_stream_class_set_property;
  oclass->dispose = aks_stream_class_dispose;
}

static
void aks_stream_init(AksStream* self) {
}
