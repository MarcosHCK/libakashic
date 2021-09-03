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
#include <libakashic.h>
#include <stdio.h>

typedef struct _AksFileFixture AksFileFixture;
struct _AksFileFixture
{
  GInputStream* input;
  AksFile* file;
};

static void
aks_file_fixture_set_up(AksFileFixture* fixture,
                        gconstpointer user_data)
{
  GError* tmp_err = NULL;
  GInputStream* stream;
  AksFile* file;

  GFile* file_ =
  g_file_new_for_path("test.a");

  stream = (GInputStream*)
  g_file_read(file_, NULL, &tmp_err);
  g_object_unref(file_);

  g_assert_no_error(tmp_err);

  file = (AksFile*)
  aks_file_new(stream, GPOINTER_TO_INT(user_data), "/test.c", NULL, &tmp_err);

  g_assert_no_error(tmp_err);

  fixture->input = stream;
  fixture->file = file;
}

static void
aks_file_fixture_tear_down(AksFileFixture* fixture,
                           gconstpointer user_data)
{
  g_clear_object(&(fixture->input));
  g_clear_object(&(fixture->file));
}

static void
aks_file_fixture_test_read(AksFileFixture* fixture,
                           gconstpointer user_data)
{
  GError* tmp_err = NULL;
  GInputStream* input = (GInputStream*)
  g_file_read(G_FILE(fixture->file), NULL, &tmp_err);

  g_assert_no_error(tmp_err);

  GFile* stdout = g_file_new_for_path("/dev/stdout");
  GOutputStream* output = (GOutputStream*)
  g_file_append_to(stdout, G_FILE_CREATE_NONE, NULL, &tmp_err);
  g_object_unref(stdout);

  g_assert_no_error(tmp_err);

  g_output_stream_splice
  (output,
   input,
   G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE
   | G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET,
   NULL,
   &tmp_err);

  g_assert_no_error(tmp_err);

  g_object_unref(output);
  g_object_unref(input);
}

typedef union  _EnumNode      EnumNode;
typedef struct _EnumNodeData  EnumNodeData;

union _EnumNode
{
  GNode node_;
  struct
  {
    struct _EnumNodeData
    {
      GFile* file;
      GFileInfo* info;
    } *data;
    EnumNode* next;
    EnumNode* prev;
    EnumNode* parent;
    EnumNode* children;
  };
};

static void
dispose_node(EnumNode* node) {
  EnumNodeData* data =
  node->data;

  g_clear_object(&(data->file));
  g_clear_object(&(data->info));
  g_slice_free(EnumNodeData, data);

  g_node_children_foreach
  (&(node->node_),
   G_TRAVERSE_ALL,
   (GNodeForeachFunc)
   dispose_node,
   NULL);
}

#define goto_error() \
G_STMT_START { \
  success = FALSE; \
  goto _error_; \
} G_STMT_END

static gboolean
load_tree(GFile          *dir,
          EnumNode       *tree,
          GCancellable   *cancellable,
          GError        **error)
{
  GError* tmp_err = NULL;
  gboolean success = TRUE;
  GFileEnumerator* enumerator = NULL;

  enumerator =
  g_file_enumerate_children
  (dir,
       G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME
   "," G_FILE_ATTRIBUTE_STANDARD_NAME
   "," G_FILE_ATTRIBUTE_STANDARD_SIZE
   "," G_FILE_ATTRIBUTE_STANDARD_TYPE,
   G_FILE_QUERY_INFO_NONE,
   cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  g_assert(enumerator != NULL);

  for(;;)
  {
    GFileInfo* info = NULL;
    GFile* file = NULL;

    g_file_enumerator_iterate
    (enumerator,
     &info,
     &file,
     NULL,
     &tmp_err);

    if G_UNLIKELY(tmp_err != NULL)
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }

    if(info == NULL)
      break;

    EnumNodeData* data =
    g_slice_new(EnumNodeData);
    EnumNode* node =
    (gpointer)
    g_node_new(data);

    g_node_append
    (&(tree->node_),
     &(node->node_));

    data->file = g_object_ref(file);
    data->info = g_object_ref(info);

    if(g_file_info_get_attribute_uint32
       (info,
        G_FILE_ATTRIBUTE_STANDARD_TYPE)
       == G_FILE_TYPE_DIRECTORY)
    {
      success =
      load_tree(file, node, cancellable, &tmp_err);
      if G_UNLIKELY(tmp_err != NULL)
      {
        g_propagate_error(error, tmp_err);
        goto_error();
      }
    }
  }

_error_:
  g_clear_object(&enumerator);
return success;
}

static void
print_entries_inner(EnumNode *node,
                    gint     *plevel)
{
  gint i, level = plevel[0];
  int fd = fileno(stderr);

  /* One space character plus three UTF-8 sequence characters */
  /* Then, add six UTF-8 sequence characters (two three bytes */
  /* long sequences ) and a final space character             */
  GString* string_ = g_string_sized_new((level << 2) + 6 + 1);

/*
 * Calculate things
 *
 */

  for(i = 0;i < level;i++)
  {
    EnumNode* thi5 = node;
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

/*
 * Print things
 *
 */

  if(level > 0)
    if(node->next == NULL)
      g_string_append(string_, "\xe2\x94\x94" "\xe2\x94\x80");
    else
      g_string_append(string_, "\xe2\x94\x9c" "\xe2\x94\x80");

  g_string_append_c(string_, ' ');
  write(fd, string_->str, string_->len);

/*
 * Append name
 *
 */

  g_string_erase
  (string_,
   0, -1);

  g_string_append
  (string_,
   g_file_info_get_attribute_byte_string
   (node->data->info,
    G_FILE_ATTRIBUTE_STANDARD_NAME));

  g_string_append_printf
  (string_,
   " %lliB",
   g_file_info_get_attribute_uint64
   (node->data->info,
    G_FILE_ATTRIBUTE_STANDARD_SIZE));

  g_string_append_printf
  (string_,
   " %s",
   g_enum_to_string
   (G_TYPE_FILE_TYPE,
    g_file_info_get_attribute_uint32
    (node->data->info,
     G_FILE_ATTRIBUTE_STANDARD_TYPE)));

/*
 * Write name
 *
 */

  write(fd, string_->str, string_->len);
  write(fd, "\r\n", 2);

  g_string_free(string_, TRUE);

/*
 * Iterate children
 *
 */

  int level_ = level + 1;
  g_node_children_foreach
  (&(node->node_),
   G_TRAVERSE_ALL,
   (GNodeForeachFunc)
   print_entries_inner,
   &level_);
}

static void
print_entries(EnumNode* node) {
  int level = 0;
  print_entries_inner(node, &level);
}

static void
aks_file_fixture_test_enumerator(AksFileFixture* fixture,
                                 gconstpointer user_data)
{
  g_object_set
  (fixture->file,
   "filename", "/",
   NULL);

  GError* tmp_err = NULL;

/*
 * Get root info
 *
 */
  GFileInfo* info =
  g_file_query_info
  (G_FILE(fixture->file),
       G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME
   "," G_FILE_ATTRIBUTE_STANDARD_NAME
   "," G_FILE_ATTRIBUTE_STANDARD_SIZE
   "," G_FILE_ATTRIBUTE_STANDARD_TYPE,
   G_FILE_QUERY_INFO_NONE,
   NULL,
   &tmp_err);

  g_assert_no_error(tmp_err);

/*
 * Create root
 *
 */
  EnumNodeData* data =
  g_slice_new(EnumNodeData);
  EnumNode* node =
  (gpointer)
  g_node_new(data);

  data->file =
  g_object_ref
  (G_FILE
   (fixture->file));
  data->info = info;

/*
 * Load tree
 *
 */
  load_tree
  (G_FILE(fixture->file),
   node,
   NULL,
   &tmp_err);

  g_assert_no_error(tmp_err);

/*
 * Display tree
 *
 */
  print_entries(node);

/*
 * Clean tree
 *
 */
  dispose_node(node);
  g_node_destroy(&(node->node_));
}

int main(int argc, char* argv[]) {
  g_test_init(&argc, &argv, NULL);

/*
 * Test file read
 *
 */
#if 0
  g_test_add
  ("/libakashic/aks_file/cache_level_none",
   AksFileFixture,
   (gpointer) AKS_CACHE_LEVEL_NONE,
   aks_file_fixture_set_up,
   aks_file_fixture_test_read,
   aks_file_fixture_tear_down);

  g_test_add
  ("/libakashic/aks_file/cache_level_otf",
   AksFileFixture,
   (gpointer) AKS_CACHE_LEVEL_OTF,
   aks_file_fixture_set_up,
   aks_file_fixture_test_read,
   aks_file_fixture_tear_down);

  g_test_add
  ("/libakashic/aks_file/cache_level_full",
   AksFileFixture,
   (gpointer) AKS_CACHE_LEVEL_FULL,
   aks_file_fixture_set_up,
   aks_file_fixture_test_read,
   aks_file_fixture_tear_down);
#endif // 0
/*
 * Test file info
 *
 */

  g_test_add
  ("/libakashic/aks_file/children_enumerator",
   AksFileFixture,
   (gpointer) AKS_CACHE_LEVEL_OTF,
   aks_file_fixture_set_up,
   aks_file_fixture_test_enumerator,
   aks_file_fixture_tear_down);
return g_test_run();
}
