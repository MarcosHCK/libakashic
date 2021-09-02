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

static
void aks_file_fixture_set_up(AksFileFixture* fixture,
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

static
void aks_file_fixture_tear_down(AksFileFixture* fixture,
                                gconstpointer user_data)
{
  g_clear_object(&(fixture->input));
  g_clear_object(&(fixture->file));
}

static
void aks_file_fixture_test_read(AksFileFixture* fixture,
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

int main(int argc, char* argv[]) {
  g_test_init(&argc, &argv, NULL);

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
return g_test_run();
}
