/*                                              -*- mode:C++ -*-
  itype.h Short names for sized ints, and limits
  Copyright (C) 2008-2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file itype.h Short names for sized ints, and limits
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ITYPE_H
#define ITYPE_H

#include <inttypes.h>

namespace MFM {

typedef int8_t  s8;                    /**< Signed 8 bit type */
typedef int16_t s16;                   /**< Signed 16 bit type */
typedef int32_t s32;                   /**< Signed 32 bit type */
typedef int64_t s64;                   /**< Signed 64 bit type */
typedef intptr_t sptr;                 /**< Signed type the size of a pointer on the current platform */

typedef const int8_t  sc8;             /**< Signed const 8 bit type */
typedef const int16_t sc16;            /**< Signed const 16 bit type */
typedef const int32_t sc32;            /**< Signed const 32 bit type */
typedef const int64_t sc64;            /**< Signed const 64 bit type */
typedef const intptr_t scptr;          /**< Signed const type the size of a pointer on the current platform */

typedef volatile int8_t  sv8;          /**< Signed volatile 8 bit type */
typedef volatile int16_t sv16;         /**< Signed volatile 16 bit type */
typedef volatile int32_t sv32;         /**< Signed volatile 32 bit type */
typedef volatile int64_t sv64;         /**< Signed volatile 64 bit type */
typedef volatile intptr_t svptr;       /**< Signed volatile type the size of a pointer on the current platform */

typedef volatile const int8_t  svc8;    /**< Signed volatile const 8 bit type */
typedef volatile const int16_t svc16;   /**< Signed volatile const 16 bit type */
typedef volatile const int32_t svc32;   /**< Signed volatile const 32 bit type */
typedef volatile const int64_t svc64;   /**< Signed volatile const 64 bit type */
typedef volatile const intptr_t svcptr; /**< Signed volatile const type the size of a pointer on the current platform */

typedef uint8_t  u8;                   /**< Unsigned 8 bit type */
typedef uint16_t u16;                  /**< Unsigned 16 bit type */
typedef uint32_t u32;                  /**< Unsigned 32 bit type */
typedef uint64_t u64;                  /**< Unsigned 64 bit type */
typedef uintptr_t uptr;                /**< Unsigned type the size of a pointer on the current platform */

typedef const uint8_t  uc8;            /**< Unsigned const 8 bit type */
typedef const uint16_t uc16;           /**< Unsigned const 16 bit type */
typedef const uint32_t uc32;           /**< Unsigned const 32 bit type */
typedef const uint64_t uc64;           /**< Unsigned const 64 bit type */
typedef const uintptr_t ucptr;         /**< Unsigned const type the size of a pointer on the current platform */

typedef volatile uint8_t  uv8;         /**< Unsigned volatile 8 bit type */
typedef volatile uint16_t uv16;        /**< Unsigned volatile 16 bit type */
typedef volatile uint32_t uv32;        /**< Unsigned volatile 32 bit type */
typedef volatile uint64_t uv64;        /**< Unsigned volatile 64 bit type */
typedef volatile uintptr_t uvptr;      /**< Unsigned volatile type the size of a pointer on the current platform */

typedef volatile const uint8_t  uvc8;    /**< Unsigned volatile const 8 bit type */
typedef volatile const uint16_t uvc16;   /**< Unsigned volatile const 16 bit type */
typedef volatile const uint32_t uvc32;   /**< Unsigned volatile const 32 bit type */
typedef volatile const uint64_t uvc64;   /**< Unsigned volatile const 64 bit type */
typedef volatile const uintptr_t uvcptr; /**< Unsigned volatile const type the size of a pointer on the current platform */

#define S8_MAX     ((s8)127)           /**< Maximum value of signed 8 bit */
#define S8_MIN     ((s8)-128)          /**< Minimum value of signed 8 bit */
#define S16_MAX    ((s16)32767)        /**< Maximum value of signed 16 bit */
#define S16_MIN    ((s16)-32768)       /**< Minimum value of signed 16 bit */
#define S32_MAX    ((s32)2147483647)   /**< Maximum value of signed 32 bit */
#define S32_MIN    ((s32)2147483648UL) /**< Minimum value of signed 32 bit */
#define S64_MAX    ((((s64)0x7fffffff)<<32)|0xffffffff)    /**< Maximum value of signed 64 bit */
#define S64_MIN    ((((s64)0x80000000)<<32))   /**< Minimum value of signed 64 bit */

#define U8_MAX     ((u8)255)           /**< Maximum value of unsigned 8 bit */
#define U8_MIN     ((u8)0)             /**< Minimum value of unsigned 8 bit */
#define U16_MAX    ((u16)65535U)       /**< Maximum value of unsigned 16 bit */
#define U16_MIN    ((u16)0)            /**< Minimum value of unsigned 16 bit */
#define U32_MAX    ((u32)4294967295UL) /**< Maximum value of unsigned 32 bit */
#define U32_MIN    ((u32)0)            /**< Minimum value of unsigned 32 bit */
#define U64_MAX    ((((u64)0xffffffff)<<32)|0xffffffff) /**< Maximum value of unsigned 64 bit */ 
                                       /* U64_MAX hacked with an expression to avoid C99ish syntax.. */
#define U64_MIN    ((u64)0LL)          /**< Minimum value of unsigned 64 bit */

} /* namespace MFM */

#endif  /* ITYPE_H */

