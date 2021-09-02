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

FileNodeData*
_aks_node_data_new() {
  FileNodeData* data =
  g_slice_new0(FileNodeData);
  g_ref_count_init(&(data->refs));
return data;
}

FileNodeData*
_aks_node_data_ref(FileNodeData* data) {
  g_ref_count_inc(&(data->refs));
}

void
_aks_node_data_unref(FileNodeData* data) {
  if(g_ref_count_dec(&(data->refs)))
  {
  /*
   * Free data
   *
   */
    g_clear_pointer(&(data->name), g_free);
    g_clear_pointer(&(data->cache), g_bytes_unref);
    g_clear_pointer(&(data->entry), archive_entry_free);

  /*
   * Free data structure
   *
   */
    g_slice_free(FileNodeData, data);
    g_print("unref\r\n");
  }
}

gboolean
_aks_node_data_equal(FileNodeData* data1,
                     FileNodeData* data2)
{
  return
  (data1 == data2
   && data1->hash_ == data2->hash_
   && g_str_equal(data1->name, data2->name));
}

G_DEFINE_BOXED_TYPE
(FileNodeData,
 _aks_node_data,
 _aks_node_data_ref,
 _aks_node_data_unref);
