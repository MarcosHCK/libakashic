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
#ifndef __LIBAKASHIC_AKS_FILE__
#define __LIBAKASHIC_AKS_FILE__
#include <gio/gio.h>

#define AKS_TYPE_FILE             (aks_file_get_type ())
#define AKS_FILE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AKS_TYPE_FILE, AksFile))
#define AKS_FILE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), AKS_TYPE_FILE, AksFileClass))
#define AKS_IS_FILE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AKS_TYPE_FILE))
#define AKS_IS_FILE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), AKS_TYPE_FILE))
#define AKS_FILE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AKS_TYPE_FILE, AksFileClass))

typedef struct _AksFile       AksFile;
typedef struct _AksFileClass  AksFileClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
aks_file_get_type();

struct _AksFileClass
{
  GObjectClass parent_class;
};

#if __cplusplus
}
#endif // __cplusplus

#endif // __LIBAKASHIC_AKS_FILE__
