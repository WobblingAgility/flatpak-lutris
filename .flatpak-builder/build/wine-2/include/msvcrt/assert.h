/*
 * Assert support
 *
 * Copyright 2011 Alexandre Julliard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <crtdefs.h>

#ifdef  __cplusplus
extern "C" {
#endif

#undef assert
#ifdef NDEBUG
#define assert(_expr) ((void)0)
#else
extern void __cdecl _assert(const char *, const char *, unsigned int);
#define assert(_expr) (void)((!!(_expr)) || (_assert(#_expr, __FILE__, __LINE__), 0))
#endif

#ifdef  __cplusplus
}
#endif
