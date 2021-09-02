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

typedef struct _ArchiveData ArchiveData;

static
const gsize LA_BLOCK_SIZE = 1024;

struct _ArchiveData
{
/*
 * Source stream
 * This object MUST NOT
 * be used as source_object argument
 * because a reference to this stream
 * is held throughout #ArchiveData lifespan.
 *
 */
  union
  { /* Mainly for readability purposes */
    GObject        *stream;
    GInputStream   *istream;
    GOutputStream  *ostream;

  /*
   * Seekable streams are only
   * used under certain circumstances
   * because libarchive lacks seekable
   * support at API level, and therefor
   * an entire new libarchive object must
   * be created if you want non-sequential IO
   *
   */
    GSeekable      *seekable;
  };

  /*
   * Read function cached
   * block
   *
   */
  gpointer block;

  /*
   * GIO miscellaneous objects
   * (GError is glib's but you
   * got the point here).
   *
   */
  GCancellable* cancellable;
  GError* error;
};

static
void archive_data_free(ArchiveData* thi5) {
/*
 * Check pending errors
 *
 */
  if G_UNLIKELY(thi5->error != NULL)
  {
    g_warning("Pending error at object finalization: %s\r\n",
              thi5->error->message);
    g_clear_error(&(thi5->error));
  }

/*
 * Finalize
 *
 */
  g_clear_object(&(thi5->stream));
  g_clear_object(&(thi5->cancellable));
  g_clear_pointer(&(thi5->block), g_free);

/*
 * Structure
 *
 */
  g_slice_free(ArchiveData, thi5);
}

static
G_DEFINE_QUARK(aks-archive-stream-use,
               stream_use);
static
G_DEFINE_QUARK(aks-archive-data,
               archive_data);

static int
archive_open(struct archive* ar,
             ArchiveData* data)
{
/*
 * Ensure stream is not in use
 *
 */
  gpointer pused =
  g_object_get_qdata
  (G_OBJECT(data->stream),
   stream_use_quark());

  gboolean used =
  GPOINTER_TO_INT(pused);
  if G_UNLIKELY(used == TRUE)
  {
    g_critical("attempt to open a stream which is already in use\r\n");
    g_assert_not_reached();
  }

/*
 * Tag stream as used
 *
 */
  g_object_set_qdata
  (G_OBJECT(data->stream),
   stream_use_quark(),
   GINT_TO_POINTER(TRUE));
}

static int
archive_close(struct archive* ar,
              ArchiveData* data)
{
/*
 * Untag stream as used
 *
 */
  g_object_set_qdata
  (G_OBJECT(data->stream),
   stream_use_quark(),
   GINT_TO_POINTER(FALSE));
}

void
_aks_archive_set_cancellable(GObject         *source_object,
                             struct archive  *ar,
                             GCancellable    *cancellable)
{
/*
 * Update cancellable object
 *
 */
  ArchiveData* data =
  g_object_get_qdata
  (source_object,
   archive_data_quark());

  g_set_object
  (&(data->cancellable),
   cancellable);
}

GError*
_aks_archive_get_gerror(GObject         *source_object,
                        struct archive  *ar)
{
  GError* tmp_err = NULL;

/*
 * Check for pending GError object
 *
 */
  ArchiveData* data =
  g_object_get_qdata
  (source_object,
   archive_data_quark());
  tmp_err = g_steal_pointer
  (&(data->error));

  if G_UNLIKELY(tmp_err == NULL)
  {
  /*
   * Check for libarchive errors
   *
   */
    int errno_ =
    archive_errno(ar);
    if G_UNLIKELY(errno_ != ARCHIVE_OK)
    {
      tmp_err =
      g_error_new
      (AKS_FILE_ERROR,
       AKS_FILE_ERROR_ARCHIVE
       + errno_,
       "libarchive: %i: %s\r\n",
       errno_,
       archive_error_string(ar));
    }
  }
return tmp_err;
};

void
_aks_archive_switch_source_object(GObject         *old_source_object,
                                  GObject         *new_source_object,
                                  struct archive  *ar G_GNUC_UNUSED)
{
  ArchiveData* data =
  g_object_steal_qdata
  (old_source_object,
   archive_data_quark());

  g_assert(data != NULL);

  g_object_set_qdata_full
  (new_source_object,
   archive_data_quark(),
   data,
   (GDestroyNotify)
   archive_data_free);
}

/*
 * Read part
 *
 */

static la_ssize_t
archive_read(struct archive  *ar,
             ArchiveData     *data,
             const void     **pblock)
{
  GError* tmp_err = NULL;
  pblock[0] = data->block;

  gsize read = 0;
  g_input_stream_read_all
  (data->istream,
   data->block,
   LA_BLOCK_SIZE,
   &read,
   data->cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    data->error = tmp_err;
    pblock[0] = NULL;
    return ARCHIVE_FATAL;
  }
return (la_ssize_t) read;
}

static la_int64_t
archive_seek(struct archive  *ar,
             ArchiveData     *data,
             la_int64_t       offset,
             int              whence)
{
  GError* tmp_err = NULL;
  gint code = G_SEEK_SET;

  if G_UNLIKELY
    (G_IS_SEEKABLE(data->seekable) == FALSE
     || g_seekable_can_seek(data->seekable) == FALSE)
  {
    data->error =
    g_error_new
    (AKS_FILE_ERROR,
     AKS_FILE_ERROR_UNSEEKABLE_INPUT,
     "Seekable input needed\r\n");
    return ARCHIVE_FATAL;
  }

  switch(whence)
  {
    case SEEK_SET: code = G_SEEK_SET; break;
    case SEEK_CUR: code = G_SEEK_CUR; break;
    case SEEK_END: code = G_SEEK_END; break;
    default:
      g_critical("No standard seek target\r\n");
      g_assert_not_reached();
      break;
  }

  g_seekable_seek
  (data->seekable,
   (goffset) offset,
   code,
   data->cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    data->error = tmp_err;
    return ARCHIVE_FATAL;
  }

  return (la_int64_t)
  g_seekable_tell(data->seekable);
}

static la_int64_t
archive_skip(struct archive  *ar,
             ArchiveData     *data,
             la_int64_t       request)
{
  GError* tmp_err = NULL;

  gssize skipped =
  g_input_stream_skip
  (data->istream,
   (gsize) request,
   data->cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    data->error = tmp_err;
    return ARCHIVE_FATAL;
  }
return (la_int64_t) skipped;
}

void
_aks_archive_read_free(GObject        *source_object,
                       struct archive *ar)
{
  archive_read_close(ar);
  archive_read_free(ar);

  g_object_set_qdata
  (source_object,
   archive_data_quark(),
   NULL);
}

struct archive*
_aks_archive_read_make(GObject        *source_object,
                       GInputStream   *stream,
                       GCancellable   *cancellable,
                       GError        **error)
{
/*
 * Create archive object
 *
 */
  struct archive* ar =
  archive_read_new();

/*
 * Allocate block and
 * take a reference to
 * stream
 *
 */
  ArchiveData* data =
  g_slice_new0(ArchiveData);
  data->block =
  g_malloc(LA_BLOCK_SIZE);
  data->istream =
  g_object_ref(stream);

  g_object_set_qdata_full
  (G_OBJECT(source_object),
   archive_data_quark(),
   data,
   (GDestroyNotify)
   archive_data_free);

/*
 * Register supported
 * compression algorithms
 * and formats
 *
 */
  archive_read_support_filter_all(ar);
  archive_read_support_format_all(ar);

/*
 * Register custom callbacks
 *
 */
  archive_read_set_open_callback (ar, (archive_open_callback*) archive_open);
  archive_read_set_close_callback(ar, (archive_close_callback*) archive_close);
  archive_read_set_read_callback (ar, (archive_read_callback*) archive_read);
  archive_read_set_seek_callback (ar, (archive_seek_callback*) archive_seek);
  archive_read_set_skip_callback (ar, (archive_skip_callback*) archive_skip);
  archive_read_set_callback_data (ar, data);

/*
 * Update cancellable
 *
 */
  _aks_archive_set_cancellable
  (G_OBJECT(source_object),
   ar,
   cancellable);

/*
 * Open archive
 *
 */
  int return_ =
  archive_read_open1(ar);
  if G_UNLIKELY(return_ < 0)
  {
    g_propagate_error
    (error,
     _aks_archive_get_gerror
     (source_object,
      ar));

    _aks_archive_read_free
    (source_object,
     ar);
    return NULL;
  }
return ar;
}

gboolean
_aks_archive_read_skip_til_entry(GObject               *source_object,
                                 struct archive        *ar,
                                 struct archive_entry  *til,
                                 GCancellable          *cancellable,
                                 GError               **error)
{
  GError* tmp_err = NULL;
  gboolean success = TRUE;
  struct archive_entry* entry;

  const gchar* name =
  archive_entry_pathname_utf8(til);
  guint hash = g_str_hash(name);

  for(;;)
  {
    int return_ =
    archive_read_next_header(ar, &entry);
    if G_UNLIKELY(return_ < 0)
    {
      g_propagate_error
      (error,
       _aks_archive_get_gerror
       (G_OBJECT(source_object),
        ar));
      goto_error();
    } else
    if(return_ == ARCHIVE_EOF)
    {
      g_set_error
      (error,
       AKS_FILE_ERROR,
       AKS_FILE_ERROR_FILE_NOT_FOUND,
       "file '%s' not found in archive\r\n",
       name);
      goto_error();
    } else
    if G_UNLIKELY(return_ != ARCHIVE_OK)
    {
      g_set_error
      (error,
       AKS_FILE_ERROR,
       AKS_FILE_ERROR_FAILED,
       "%s: " G_STRINGIFY(__LINE__) ": "
       "unknown return code '%i'\r\n",
       G_STRFUNC, return_);
      goto_error();
    }

    const gchar* name_ =
    archive_entry_pathname_utf8(entry);
    guint hash_ = g_str_hash(name_);

    if G_UNLIKELY(hash_ == hash)
    {
      gboolean matches =
      g_str_equal(name, name_);
      if G_UNLIKELY(matches == TRUE)
      {
        /* not an error */
        goto _error_;
      }
    }
  }

_error_:
return success;
}

gboolean
_aks_archive_dump_to_stream(GObject         *source_object,
                            struct archive  *ar,
                            GOutputStream   *stream,
                            GCancellable    *cancellable,
                            GError         **error)
{
  GError* tmp_err = NULL;
  gboolean success = TRUE;
  const void* block;
  la_int64_t offset;
  size_t size;

  _aks_archive_set_cancellable
  (G_OBJECT(source_object),
   ar,
   cancellable);

  for(;;)
  {
    int return_ =
    archive_read_data_block(ar, &block, &size, &offset);
    if G_UNLIKELY(return_ < 0)
    {
      g_propagate_error
      (error,
       _aks_archive_get_gerror
       (G_OBJECT(source_object),
        ar));
      goto_error();
    }

    if(return_ == ARCHIVE_EOF)
      break;

    g_output_stream_write_all
    (stream,
     block,
     (gsize) size,
     NULL,
     cancellable,
     &tmp_err);

    if G_UNLIKELY(tmp_err != NULL)
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }
  }

_error_:
return success;
}

GBytes*
_aks_archive_dump_to_bytes(GObject         *source_object,
                           struct archive  *ar,
                           GCancellable    *cancellable,
                           GError         **error)
{
  GError* tmp_err = NULL;
  gboolean success = TRUE;
  GBytes* return_ = NULL;

  GOutputStream* stream = (GOutputStream*)
  g_memory_output_stream_new_resizable();

  _aks_archive_dump_to_stream
  (G_OBJECT(source_object),
   ar,
   stream,
   cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  g_output_stream_close(stream, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  return_ =
  g_memory_output_stream_steal_as_bytes
  (G_MEMORY_OUTPUT_STREAM(stream));

_error_:
  g_object_unref(stream);
return return_;
}
