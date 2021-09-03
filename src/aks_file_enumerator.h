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
#ifndef __LIBAKASHIC_AKS_FILE_ENUMERATOR__
#define __LIBAKASHIC_AKS_FILE_ENUMERATOR__
#include <aks_file.h>

#define AKS_TYPE_FILE_ENUMERATOR            (aks_file_enumerator_get_type ())
#define AKS_FILE_ENUMERATOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), AKS_TYPE_FILE_ENUMERATOR, AksFileEnumerator))
#define AKS_FILE_ENUMERATOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), AKS_TYPE_FILE_ENUMERATOR, AksFileEnumeratorClass))
#define AKS_IS_FILE_ENUMERATOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AKS_TYPE_FILE_ENUMERATOR))
#define AKS_IS_FILE_ENUMERATOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), AKS_TYPE_FILE_ENUMERATOR))
#define AKS_FILE_ENUMERATOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), AKS_TYPE_FILE_ENUMERATOR, AksFileEnumeratorClass))

typedef struct _AksFileEnumerator       AksFileEnumerator;
typedef struct _AksFileEnumeratorClass  AksFileEnumeratorClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
aks_file_enumerator_get_type();

struct _AksFileEnumeratorClass
{
  GFileEnumeratorClass parent_class;
};

GFileEnumerator*
_aks_file_enumerator_new(AksFile *file,
                         const char *attributes,
                         GFileQueryInfoFlags flags,
                         GCancellable *cancellable,
                         GError **error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __LIBAKASHIC_AKS_FILE_ENUMERATOR__
