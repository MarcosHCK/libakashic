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
  aks_file_new(stream, AKS_CACHE_LEVEL_OTF, NULL, &tmp_err);

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
void aks_file_fixture_test1(AksFileFixture* fixture,
                            gconstpointer user_data)
{
  const gchar* path = g_file_peek_path(G_FILE(fixture->file));
  g_assert_cmpstr(path, ==, "/");
}

int main(int argc, char* argv[]) {
  g_test_init(&argc, &argv, NULL);

  g_test_add
  ("/libakashic/aks_file",
   AksFileFixture,
   NULL,
   aks_file_fixture_set_up,
   aks_file_fixture_test1,
   aks_file_fixture_tear_down);
return g_test_run();
}
