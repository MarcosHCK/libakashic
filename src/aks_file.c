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
#include <aks_file.h>

/*
 * Object definition
 *
 */

struct _AksFile
{
  GObject parent_instance;
};

G_DEFINE_TYPE
(AksFile,
 aks_file,
 G_TYPE_OBJECT);

static
void aks_file_class_init(AksFileClass* klass) {
}

static
void aks_file_init(AksFile* self) {
}

/*
 * Object methods
 *
 */

AksFile*
aks_file_new(GInputStream  *base_stream,
             GCancellable  *cancellable,
             GError       **error)
{
  return g_initable_new
  (AKS_TYPE_FILE,
   cancellable,
   error,
   "base-stream", base_stream,
   NULL);
}