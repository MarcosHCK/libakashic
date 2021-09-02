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
#ifndef __LIBAKASHIC_AKS_STREAM__
#define __LIBAKASHIC_AKS_STREAM__
#include <gio/gio.h>

#define AKS_TYPE_STREAM             (aks_stream_get_type ())
#define AKS_STREAM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AKS_TYPE_STREAM, AksStream))
#define AKS_STREAM_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), AKS_TYPE_STREAM, AksStreamClass))
#define AKS_IS_STREAM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AKS_TYPE_STREAM))
#define AKS_IS_STREAM_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), AKS_TYPE_STREAM))
#define AKS_STREAM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AKS_TYPE_STREAM, AksStreamClass))

typedef struct _AksStream       AksStream;
typedef struct _AksStreamClass  AksStreamClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
aks_stream_get_type();

struct _AksStreamClass
{
  GInputStreamClass parent_class;
};

#if __cplusplus
}
#endif // __cplusplus

#endif // __LIBAKASHIC_AKS_STREAM__
