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
#ifndef __LIBAKASHIC_AKS_ENUMS__
#define __LIBAKASHIC_AKS_ENUMS__
#include <glib-object.h>

typedef enum {
  AKS_CACHE_LEVEL_NONE,
  AKS_CACHE_LEVEL_OTF,
  AKS_CACHE_LEVEL_FULL,
} AksCacheLevel;

GType
aks_cache_level_get_type();
#define AKS_TYPE_CACHE_LEVEL (aks_cache_level_get_type())

#endif // __LIBAKASHIC_AKS_ENUMS__
