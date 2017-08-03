/* -*- mode:C++ -*- */
/**
  CastOps.h Primitive casting and ALU operations
  Copyright (C) 2014-2015 The Regents of the University of New Mexico.
  Copyright (C) 2014-2015 Ackleyshack LLC.

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
  \file CastOps.h Primitive casting and ALU operations
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \lgpl
 */
#ifndef CASTOPS_H
#define CASTOPS_H

#include "itype.h"
#include "Util.h"
#include "Fail.h"

namespace MFM {
  //Between C and Ulam-native

  //C-INT
  inline s32 _Int32ToCs32(u32 val, const u32 srcbitwidth)
  {
    return _SignExtend32(val, srcbitwidth);
  }

  inline s64 _Int64ToCs64(u64 val, const u32 srcbitwidth)
  {
    return _SignExtend64(val, srcbitwidth);
  }

  inline u32 _Cs32ToInt32(s32 val, const u32 destbitwidth)
  {
    const s32 maxdestval = _GetNOnes32(destbitwidth-1);  //positive
    const s32 mindestval = ~maxdestval;
    return CLAMP<s32>(mindestval, maxdestval, val);
  }

  inline u64 _Cs64ToInt64(s64 val, const u32 destbitwidth)
  {
    const s64 maxdestval = _GetNOnes64(destbitwidth-1);
    const s64 mindestval = ~maxdestval;
    return CLAMP<s64>(mindestval, maxdestval, val);
  }

  //C-UNSIGNED
  inline u32 _Unsigned32ToCu32(u32 val, const u32 srcbitwidth)
  {
    return val;
  }

  inline u64 _Unsigned64ToCu64(u64 val, const u32 srcbitwidth)
  {
    return val;
  }

  inline u32 _Cu32ToUnsigned32(u32 val, const u32 destbitwidth)
  {
    const u32 maxdestval = _GetNOnes32(destbitwidth);
    return CLAMP<u32>(0, maxdestval, val);
  }

  inline u64 _Cu64ToUnsigned64(u64 val, const u32 destbitwidth)
  {
    const u64 maxdestval = _GetNOnes64(destbitwidth);
    return CLAMP<u64>(0, maxdestval, val);
  }

  inline s32 _Unsigned32ToCs32(u32 val, const u32 srcbitwidth)
  {
    u32 maxdestval = _GetNOnes32(32 - 1);
    return (s32) MIN<u32>(val, maxdestval);
  }

  inline s64 _Unsigned64ToCs64(u64 val, const u32 srcbitwidth)
  {
    u64 maxdestval = _GetNOnes64(64 - 1);
    return (s64) MIN<u64>(val, maxdestval);
  }

  inline u32 _Cs32ToUnsigned32(s32 val, const u32 destbitwidth)
  {
    if(val <= 0)
      return 0;

    const u32 maxdestval = _GetNOnes32(destbitwidth);
    return CLAMP<u32>(0, maxdestval, (u32) val);
  }

  inline u64 _Cs64ToUnsigned64(s64 val, const u32 destbitwidth)
  {
    if(val <= 0)
      return 0;

    const u64 maxdestval = _GetNOnes64(destbitwidth);
    return CLAMP<u64>(0, maxdestval, (u64) val);
  }

  //C-BOOL
  inline bool _Bool32ToCbool(u32 val, const u32 srcbitwidth)
  {
    // == when even number bits is ignored (warning at def)
    s32 count1s = PopCount(val & _GetNOnes32(srcbitwidth));
    return (count1s > (s32) (srcbitwidth - count1s));
  }

  inline bool _Bool64ToCbool(u64 val, const u32 srcbitwidth)
  {
    // == when even number bits is ignored (warning at def)
    s32 count1s = PopCount64(val & _GetNOnes64(srcbitwidth));
    return (count1s > (s32) (srcbitwidth - count1s));
  }

  inline u32 _CboolToBool32(bool val, const u32 destbitwidth)
  {
    if(val)
      return _GetNOnes32(destbitwidth); //saturate
    return 0;
  }

  inline u64 _CboolToBool64(bool val, const u32 destbitwidth)
  {
    if(val)
      return _GetNOnes64(destbitwidth); //saturate
    return 0;
  }

  inline s32 _Bool32ToCs32(u32 val, const u32 srcbitwidth)
  {
    bool b = _Bool32ToCbool(val, srcbitwidth);
    return (b ? 1 : 0);
  }

  inline s64 _Bool64ToCs64(u64 val, const u32 srcbitwidth)
  {
    bool b = _Bool64ToCbool(val, srcbitwidth);
    return (b ? 1 : 0);
  }

  inline u32 _Bool32ToCu32(u32 val, const u32 srcbitwidth)
  {
    bool b = _Bool32ToCbool(val, srcbitwidth);
    return (b ? 1 : 0);
  }

  inline u64 _Bool64ToCu64(u64 val, const u32 srcbitwidth)
  {
    bool b = _Bool64ToCbool(val, srcbitwidth);
    return (b ? 1 : 0);
  }

  // C-unsigned and UNARY
  inline u32 _Unary32ToCu32(u32 val, const u32 srcbitwidth)
  {
    return PopCount(val & _GetNOnes32(srcbitwidth));
  }

  inline u64 _Unary64ToCu64(u64 val, const u32 srcbitwidth)
  {
    return PopCount64(val & _GetNOnes64(srcbitwidth));
  }

  inline u32 _Cu32ToUnary32(u32 val, const u32 destbitwidth)
  {
    const u32 maxdestval = destbitwidth;
    return _GetNOnes32(CLAMP<u32>(0, maxdestval, val));
  }

  inline u64 _Cu64ToUnary64(u64 val, const u32 destbitwidth)
  {
    const u64 maxdestval = destbitwidth;
    return _GetNOnes64(CLAMP<u64>(0, maxdestval, val));
  }

  // C-signed and UNARY
  inline s32 _Unary32ToCs32(u32 val, const u32 srcbitwidth)
  {
    return (s32) PopCount(val & _GetNOnes32(srcbitwidth));
  }

  inline s64 _Unary64ToCs64(u64 val, const u32 srcbitwidth)
  {
    return (s64) PopCount64(val & _GetNOnes64(srcbitwidth));
  }

  inline u32 _Cs32ToUnary32(s32 val, const u32 destbitwidth)
  {
    const s32 maxdestval = destbitwidth;
    return (u32) _GetNOnes32(CLAMP<s32>(0, maxdestval, val));
  }

  inline u64 _Cs64ToUnary64(s64 val, const u32 destbitwidth)
  {
    const s64 maxdestval = destbitwidth;
    return (u64) _GetNOnes64(CLAMP<s64>(0, maxdestval, val));
  }


  // C- and BITS
  inline s32 _Bits32ToCs32(u32 val, const u32 srcbitwidth)
  {
    return (s32) val & _GetNOnes32(srcbitwidth); //no sign extend
  }

  inline s64 _Bits64ToCs64(u64 val, const u32 srcbitwidth)
  {
    return (s64) val & _GetNOnes64(srcbitwidth); //no sign extend
  }

  inline u32 _Bits32ToCu32(u32 val, const u32 srcbitwidth)
  {
    return val & _GetNOnes32(srcbitwidth);
  }

  inline u64 _Bits64ToCu64(u64 val, const u32 srcbitwidth)
  {
    return val & _GetNOnes64(srcbitwidth);
  }

  inline u32 _Cu32ToBits32(u32 val, const u32 destbitwidth)
  {
    return val & _GetNOnes32(destbitwidth); //no saturate
  }

  inline u64 _Cu64ToBits64(u64 val, const u32 destbitwidth)
  {
    return val & _GetNOnes64(destbitwidth); //no saturate
  }

  //Casts: ULAM native to ULAM native
  inline u32 _Int32ToInt32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 cval = _Int32ToCs32(val, srcbitwidth);
    return _Cs32ToInt32(cval, destbitwidth);
  }

  inline u64 _Int64ToInt64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s64 cval = _Int64ToCs64(val, srcbitwidth);
    return _Cs64ToInt64(cval, destbitwidth);
  }

  inline u32 _Unsigned32ToInt32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 cval = _Unsigned32ToCs32(val, srcbitwidth);
    return _Cs32ToInt32(cval, destbitwidth); //positive
  }

  inline u64 _Unsigned64ToInt64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s64 cval = _Unsigned64ToCs64(val, srcbitwidth);
    return _Cs64ToInt64(cval, destbitwidth); //positive
  }

  inline u32 _Bool32ToInt32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 cval = _Bool32ToCs32(val, srcbitwidth);
    return _Cs32ToInt32(cval, destbitwidth);
  }

  inline u64 _Bool64ToInt64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s64 cval = _Bool64ToCs64(val, srcbitwidth);
    return _Cs64ToInt64(cval, destbitwidth);
  }

  inline u32 _Unary32ToInt32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 cval = _Unary32ToCs32(val, srcbitwidth);
    return _Cs32ToInt32(cval, destbitwidth); // >=0
  }

  inline u64 _Unary64ToInt64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s64 cval = _Unary64ToCs64(val, srcbitwidth);
    return _Cs64ToInt64(cval, destbitwidth); // >=0
  }

  inline u32 _Bits32ToInt32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 cval = _Bits32ToCs32(val, srcbitwidth); //no sign extend
    return _Cs32ToInt32(cval, destbitwidth); //positive
  }

  inline u64 _Bits64ToInt64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s64 cval = _Bits64ToCs64(val, srcbitwidth); //no sign extend
    return _Cs64ToInt64(cval, destbitwidth); //positive
  }

  //To BOOL:
  inline u32 _Bool32ToBool32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    bool b = _Bool32ToCbool(val, srcbitwidth);
    return (b ? _GetNOnes32(destbitwidth) : 0);
  }

  inline u32 _Bool64ToBool64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    bool b = _Bool64ToCbool(val, srcbitwidth);
    return (b ? _GetNOnes64(destbitwidth) : 0);
  }

  inline u32 _Int32ToBool32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 cval = _Int32ToCs32(val, srcbitwidth);
    return _CboolToBool32((cval != 0), destbitwidth);
  }

  inline u64 _Int64ToBool64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s64 cval = _Int64ToCs64(val, srcbitwidth);
    return _CboolToBool64((cval != 0), destbitwidth);
  }

  inline u32 _Unsigned32ToBool32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u32 cval = _Unsigned32ToCu32(val, srcbitwidth);
    return _CboolToBool32((cval != 0), destbitwidth);
  }

  inline u64 _Unsigned64ToBool64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u64 cval = _Unsigned64ToCu64(val, srcbitwidth);
    return _CboolToBool64((cval != 0), destbitwidth);
  }

  inline u32 _Unary32ToBool32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u32 cval = _Unary32ToCu32(val, srcbitwidth);
    return _CboolToBool32((cval != 0), destbitwidth);
  }

  inline u64 _Unary64ToBool64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u64 cval = _Unary64ToCu64(val, srcbitwidth);
    return _CboolToBool64((cval != 0), destbitwidth);
  }

  inline u32 _Bits32ToBool32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (val & _GetNOnes32(MIN<u32>(srcbitwidth, destbitwidth))); //no change to Bit data
  }

  inline u64 _Bits64ToBool64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (val & _GetNOnes64(MIN<u64>(srcbitwidth, destbitwidth))); //no change to Bit data
  }


  //To UNSIGNED:
  inline u32 _Int32ToUnsigned32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 cval = _Int32ToCs32(val, srcbitwidth);
    return _Cs32ToUnsigned32(cval, destbitwidth);
  }

  inline u64 _Int64ToUnsigned64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s64 cval = _Int64ToCs64(val, srcbitwidth);
    return _Cs32ToUnsigned32(cval, destbitwidth);
  }

  inline u32 _Unsigned32ToUnsigned32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u32 cval = _Unsigned32ToCu32(val, srcbitwidth);
    return _Cu32ToUnsigned32(cval, destbitwidth);
  }

  inline u64 _Unsigned64ToUnsigned64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u64 cval = _Unsigned64ToCu64(val, srcbitwidth);
    return _Cu64ToUnsigned64(cval, destbitwidth);
  }

  inline u32 _Bool32ToUnsigned32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    bool b = _Bool32ToCbool(val, srcbitwidth);
    return (b ? 1 : 0);
  }

  inline u64 _Bool64ToUnsigned64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    bool b = _Bool64ToCbool(val, srcbitwidth);
    return (b ? 1 : 0);
  }

  inline u32 _Unary32ToUnsigned32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u32 cval = _Unary32ToCu32(val, srcbitwidth);
    return _Cu32ToUnsigned32(cval, destbitwidth);
  }

  inline u64 _Unary64ToUnsigned64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u64 cval = _Unary64ToCu64(val, srcbitwidth);
    return _Cu64ToUnsigned64(cval, destbitwidth);
  }

  inline u32 _Bits32ToUnsigned32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u32 cval = _Bits32ToCu32(val, srcbitwidth);
    return _Cu32ToUnsigned32(cval, destbitwidth);
  }

  inline u64 _Bits64ToUnsigned64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u64 cval = _Bits64ToCu64(val, srcbitwidth);
    return _Cu64ToUnsigned64(cval, destbitwidth);
  }

  //To BITS:
  inline u32 _Int32ToBits32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u32 maskedval = val & _GetNOnes32(srcbitwidth);
    u32 maskdestval = val & _GetNOnes32(destbitwidth);
    return MIN<u32>(maskedval, maskdestval); //no sign extend
  }

  inline u64 _Int64ToBits64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u64 maskedval = val & _GetNOnes64(srcbitwidth);
    u64 maskdestval = val & _GetNOnes64(destbitwidth);
    return MIN<u64>(maskedval, maskdestval); //no sign extend
  }

  inline u32 _Unsigned32ToBits32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u32 maskedval = val & _GetNOnes32(srcbitwidth);
    u32 maskdestval = val & _GetNOnes32(destbitwidth);
    return MIN<u32>(maskedval, maskdestval);
  }

  inline u64 _Unsigned64ToBits64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u64 maskedval = val & _GetNOnes64(srcbitwidth);
    u64 maskdestval = val & _GetNOnes64(destbitwidth);
    return MIN<u64>(maskedval, maskdestval);
  }

  inline u32 _Bool32ToBits32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u32 maskedval = val & _GetNOnes32(srcbitwidth);
    u32 maskdestval = val & _GetNOnes32(destbitwidth);
    return MIN<u32>(maskedval, maskdestval);
  }

  inline u64 _Bool64ToBits64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u64 maskedval = val & _GetNOnes64(srcbitwidth);
    u64 maskdestval = val & _GetNOnes64(destbitwidth);
    return MIN<u64>(maskedval, maskdestval);
  }

  inline u32 _Unary32ToBits32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u32 maskedval = val & _GetNOnes32(srcbitwidth);
    u32 maskdestval = val & _GetNOnes32(destbitwidth);
    return MIN<u32>(maskedval, maskdestval);
  }

  inline u64 _Unary64ToBits64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u64 maskedval = val & _GetNOnes64(srcbitwidth);
    u64 maskdestval = val & _GetNOnes64(destbitwidth);
    return MIN<u64>(maskedval, maskdestval);
  }

  inline u32 _Bits32ToBits32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u32 maskedval = val & _GetNOnes32(srcbitwidth);
    u32 maskdestval = val & _GetNOnes32(destbitwidth);
    return MIN<u32>(maskedval, maskdestval);
  }

  inline u64 _Bits64ToBits64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u64 maskedval = val & _GetNOnes64(srcbitwidth);
    u64 maskdestval = val & _GetNOnes64(destbitwidth);
    return MIN<u64>(maskedval, maskdestval);
  }

  //To UNARY:
  inline u32 _Int32ToUnary32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 cval = _Int32ToCs32(val, srcbitwidth);
    return _Cs32ToUnary32(cval, destbitwidth);
  }

  inline u64 _Int64ToUnary64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s64 cval = _Int64ToCs64(val, srcbitwidth);
    return _Cs64ToUnary64(cval, destbitwidth);
  }

  inline u32 _Unsigned32ToUnary32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u32 cval = _Unsigned32ToCu32(val, srcbitwidth);
    return _Cu32ToUnary32(cval, destbitwidth);
  }

  inline u64 _Unsigned64ToUnary64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u64 cval = _Unsigned64ToCu64(val, srcbitwidth);
    return _Cu64ToUnary64(cval, destbitwidth);
  }

  inline u32 _Bool32ToUnary32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    bool b = _Bool32ToCbool(val,srcbitwidth);
    return (b ? 1 : 0);
  }

  inline u64 _Bool64ToUnary64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    bool b = _Bool64ToCbool(val,srcbitwidth);
    return (b ? 1 : 0);
  }

  inline u32 _Unary32ToUnary32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u32 cval = _Unary32ToCu32(val, srcbitwidth);
    return _Cu32ToUnary32(cval, destbitwidth);
  }

  inline u64 _Unary64ToUnary64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u64 cval = _Unary64ToCu64(val, srcbitwidth);
    return _Cu64ToUnary64(cval, destbitwidth);
  }

  inline u32 _Bits32ToUnary32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (val & _GetNOnes32(MIN<u32>(srcbitwidth, destbitwidth)));
  }

  inline u64 _Bits64ToUnary64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (val & _GetNOnes64(MIN<u64>(srcbitwidth, destbitwidth)));
  }

  //'ALU' ops
  enum { BITS_PER_BOOL = 1 };

  //LOGICAL BANG:
  inline u32 _LogicalBangBool32(u32 val, u32 bitwidth)
  {
    bool b = _Bool32ToCbool(val, bitwidth);
    return (b ? 0 : _GetNOnes32(bitwidth));
  }

  inline u64 _LogicalBangBool64(u64 val, u32 bitwidth)
  {
    bool b = _Bool64ToCbool(val, bitwidth);
    return (b ? 0 : _GetNOnes64(bitwidth));
  }

  //Unary OP: Minus
  inline u32 _UnaryMinusInt32(u32 val, u32 bitwidth)
  {
    s32 cval = _Int32ToCs32(val, bitwidth);
    if(cval == S32_MIN)
      return S32_MAX; //saturating, closest answer
    return _Cs32ToInt32(-cval, bitwidth);
  }

  inline u64 _UnaryMinusInt64(u64 val, u32 bitwidth)
  {
    s32 cval = _Int64ToCs64(val, bitwidth);
    if(cval == S64_MIN)
      return S64_MAX; //saturating, closest answer
    return _Cs64ToInt64(-cval, bitwidth);
  }

  //Bitwise Binary Ops:
  inline u32 _BitwiseOrUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unary32ToCu32(vala, bitwidth);
    u32 cvalb = _Unary32ToCu32(valb, bitwidth);
    u32 maska = _Cu32ToUnary32(cvala, bitwidth); //right-justified
    u32 maskb = _Cu32ToUnary32(cvalb, bitwidth); //right-justified
    return maska | maskb;  //"max"
  }

  inline u64 _BitwiseOrUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unary64ToCu64(vala, bitwidth);
    u64 cvalb = _Unary64ToCu64(valb, bitwidth);
    u64 maska = _Cu64ToUnary64(cvala, bitwidth); //right-justified
    u64 maskb = _Cu64ToUnary64(cvalb, bitwidth); //right-justified
    return maska | maskb;  //"max"
  }

  inline u32 _BitwiseAndUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unary32ToCu32(vala, bitwidth);
    u32 cvalb = _Unary32ToCu32(valb, bitwidth);
    u32 maska = _Cu32ToUnary32(cvala, bitwidth); //right-justified
    u32 maskb = _Cu32ToUnary32(cvalb, bitwidth); //right-justified
    return maska & maskb;  //"min"
  }

  inline u64 _BitwiseAndUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unary64ToCu64(vala, bitwidth);
    u64 cvalb = _Unary64ToCu64(valb, bitwidth);
    u64 maska = _Cu64ToUnary64(cvala, bitwidth); //right-justified
    u64 maskb = _Cu64ToUnary64(cvalb, bitwidth); //right-justified
    return maska & maskb;  //"min"
  }

  inline u32 _BitwiseXorUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unary32ToCu32(vala, bitwidth);
    u32 cvalb = _Unary32ToCu32(valb, bitwidth);
    return _GetNOnes32(MAX<u32>(cvala, cvalb) - MIN<u32>(cvala, cvalb)); //right-justified ^
  }

  inline u64 _BitwiseXorUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unary64ToCu64(vala, bitwidth);
    u64 cvalb = _Unary64ToCu64(valb, bitwidth);
    return _GetNOnes64(MAX<u64>(cvala, cvalb) - MIN<u64>(cvala, cvalb)); //right-justified ^
  }

  inline u32 _BitwiseOrBits32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return  ( (vala | valb) & mask) ;  // "at least max"
  }

  inline u64 _BitwiseOrBits64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala | valb) & mask);  //"at least max"
  }

  inline u32 _BitwiseAndBits32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ( (vala & valb) & mask);  //"at most min"
  }

  inline u64 _BitwiseAndBits64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala & valb) & mask);  //"at most min"
  }


  inline u32 _BitwiseXorBits32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ( (vala ^ valb) & mask);
  }

  inline u64 _BitwiseXorBits64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala ^ valb) & mask);
  }

  inline u32 _BitwiseOrInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32 cvalb = _Int32ToCs32(valb, bitwidth);
    return _Cs32ToInt32((cvala | cvalb), bitwidth); // "at least max"
  }

  inline u64 _BitwiseOrInt64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64 cvalb = _Int64ToCs64(valb, bitwidth);
    return _Cs64ToInt64((cvala | cvalb), bitwidth); // "at least max"
  }

  inline s32 _BitwiseAndInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32 cvalb = _Int32ToCs32(valb, bitwidth);
    return _Cs32ToInt32((cvala & cvalb), bitwidth); //"at most min"
  }

  inline s64 _BitwiseAndInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64 cvalb = _Int64ToCs64(valb, bitwidth);
    return _Cs64ToInt64((cvala & cvalb), bitwidth); //"at most min"
  }

  inline u32 _BitwiseXorInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32 cvalb = _Int32ToCs32(valb, bitwidth);
    return _Cs32ToInt32((cvala ^ cvalb), bitwidth);
  }

  inline s64 _BitwiseXorInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64 cvalb = _Int64ToCs64(valb, bitwidth);
    return _Cs64ToInt64((cvala ^ cvalb), bitwidth);
  }

  inline u32 _BitwiseOrUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32 cvalb = _Unsigned32ToCu32(valb, bitwidth);
    return _Cu32ToUnsigned32((cvala | cvalb), bitwidth); // "at least max"
  }

  inline u64 _BitwiseOrUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64 cvalb = _Unsigned64ToCu64(valb, bitwidth);
    return _Cu64ToUnsigned64((cvala | cvalb), bitwidth); // "at least max"
  }

  inline u32 _BitwiseAndUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32 cvalb = _Unsigned32ToCu32(valb, bitwidth);
    return _Cu32ToUnsigned32((cvala & cvalb), bitwidth); //"at most min"
  }

  inline u64 _BitwiseAndUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64 cvalb = _Unsigned64ToCu64(valb, bitwidth);
    return _Cu64ToUnsigned64((cvala & cvalb), bitwidth); //"at most min"
  }

  inline u32 _BitwiseXorUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32 cvalb = _Unsigned32ToCu32(valb, bitwidth);
    return _Cu32ToUnsigned32((cvala ^ cvalb), bitwidth);
  }

  inline u64 _BitwiseXorUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64 cvalb = _Unsigned64ToCu64(valb, bitwidth);
    return _Cu64ToUnsigned64((cvala ^ cvalb), bitwidth);
  }

  inline u32 _BitwiseOrBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToBits32(vala, bitwidth, bitwidth);
    u32 binvalb = _Bool32ToBits32(valb, bitwidth, bitwidth);
    return _Bits32ToBool32(binvala | binvalb, bitwidth, bitwidth);
  }

  inline u64 _BitwiseOrBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToBits64(vala, bitwidth, bitwidth);
    u64 binvalb = _Bool64ToBits64(valb, bitwidth, bitwidth);
    return _Bits64ToBool64(binvala | binvalb, bitwidth, bitwidth);
  }

  inline u32 _BitwiseAndBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToBits32(vala, bitwidth, bitwidth);
    u32 binvalb = _Bool32ToBits32(valb, bitwidth, bitwidth);
    return _Bits32ToBool32(binvala & binvalb, bitwidth, bitwidth);
  }

  inline u64 _BitwiseAndBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToBits64(vala, bitwidth, bitwidth);
    u64 binvalb = _Bool64ToBits64(valb, bitwidth, bitwidth);
    return _Bits64ToBool64(binvala & binvalb, bitwidth, bitwidth);
  }

  inline u32 _BitwiseXorBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToBits32(vala, bitwidth, bitwidth);
    u32 binvalb = _Bool32ToBits32(valb, bitwidth, bitwidth);
    return _Bits32ToBool32(binvala ^ binvalb, bitwidth, bitwidth);
  }

  inline u64 _BitwiseXorBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToBits64(vala, bitwidth, bitwidth);
    u64 binvalb = _Bool64ToBits64(valb, bitwidth, bitwidth);
    return _Bits64ToBool64(binvala ^ binvalb, bitwidth, bitwidth);
  }

  //Bounds checks for INT (32,64 bitwidths) arith:
  // based on http://stackoverflow.com/questions/199333/how-to-detect-integer-overflow-in-c-c

  inline u32 _BinOpAddCs32WithBoundsCheck(s32 cvala, s32 cvalb)
  {
    if ((cvalb > 0) && (cvala > S32_MAX - cvalb)) /* `a + b` would overflow */
      return S32_MAX;
    if ((cvalb < 0) && (cvala < S32_MIN - cvalb)) /* `a + b` would underflow */
      return S32_MIN;
    return _Cs32ToInt32((cvala + cvalb), 32);
  }

  inline u64 _BinOpAddCs64WithBoundsCheck(s64 cvala, s64 cvalb)
  {
    if ((cvalb > 0) && (cvala > S64_MAX - cvalb)) /* `a + b` would overflow */
      return S64_MAX;
    if ((cvalb < 0) && (cvala < S64_MIN - cvalb)) /* `a + b` would underflow */
      return S64_MIN;
    return _Cs64ToInt64((cvala + cvalb), 64);
  }

  inline u32 _BinOpSubtractCs32WithBoundsCheck(s32 cvala, s32 cvalb)
  {
    if ((cvalb < 0) && (cvala > S32_MAX + cvalb)) /* `a - b` would overflow */
      return S32_MAX;
    if ((cvalb > 0) && (cvala < S32_MIN + cvalb)) /* `a - b` would underflow */
      return S32_MIN;
    return _Cs32ToInt32((cvala - cvalb), 32);
  }

  inline u64 _BinOpSubtractCs64WithBoundsCheck(s64 cvala, s64 cvalb)
  {
    if ((cvalb < 0) && (cvala > S64_MAX + cvalb)) /* `a - b` would overflow */
      return S64_MAX;
    if ((cvalb > 0) && (cvala < S64_MIN + cvalb)) /* `a - b` would underflow */
      return S64_MIN;
    return _Cs64ToInt64((cvala - cvalb), 64);
  }

  inline u32 _BinOpMultiplyCs32WithBoundsCheck(s32 cvala, s32 cvalb)
  {
    // there may be need to check for -1 for two's complement machines
    if ((cvala == -1) && (cvalb == S32_MIN)) /* `a * b` can overflow */
      return S32_MAX;
    if ((cvala == S32_MIN) && (cvalb == -1)) /* `a * b` (or `a / b`) can overflow */
      return S32_MAX;

    if(cvalb > 0)
      {
	if(cvala > (S32_MAX / cvalb)) /* `a * b` would overflow */
	  return S32_MAX;
	if(cvala < (S32_MIN / cvalb)) /* `a * b` would underflow */
	  return S32_MIN;
      }
    else if(cvalb < 0)
      {
	if(cvala < (S32_MAX / cvalb)) /* `a * b` would overflow */
	  return S32_MAX;
	if((cvalb < -1) && (cvala > (S32_MIN / cvalb))) /* `a * b` would underflow */
	  return S32_MIN;
      }
    //else
    return _Cs32ToInt32((cvala * cvalb), 32);
  }

  inline u64 _BinOpMultiplyCs64WithBoundsCheck(s64 cvala, s64 cvalb)
  {
    // there may be need to check for -1 for two's complement machines
    if ((cvala == -1) && (cvalb == S64_MIN)) /* `a * b` can overflow */
      return S64_MAX;
    if ((cvala == S64_MIN) && (cvalb == -1)) /* `a * b` (or `a / b`) can overflow */
      return S64_MAX;

    if(cvalb > 0)
      {
	if(cvala > (S64_MAX / cvalb)) /* `a * b` would overflow */
	  return S64_MAX;
	if(cvala < (S64_MIN / cvalb)) /* `a * b` would underflow */
	  return S64_MIN;
      }
    else if(cvalb < 0)
      {
	if(cvala < (S64_MAX / cvalb)) /* `a * b` would overflow */
	  return S64_MAX;
	if((cvalb < -1) && (cvala > (S64_MIN / cvalb))) /* `a * b` would underflow */
	  return S64_MIN;
      }
    //else
    return _Cs64ToInt64((cvala * cvalb), 64);
  }

  inline u32 _BinOpDivideCs32WithBoundsCheck(s32 cvala, s32 cvalb)
  {
    if ((cvalb == -1) && (cvala == S32_MIN)) /* `a * b` (or `a / b`) can overflow */
      return S32_MAX;
    return _Cs32ToInt32((cvala / cvalb), 32);
  }

  inline u64 _BinOpDivideCs64WithBoundsCheck(s64 cvala, s64 cvalb)
  {
    if ((cvalb == -1) && (cvala == S64_MIN)) /* `a * b` (or `a / b`) can overflow */
      return S64_MAX;
    return _Cs64ToInt64((cvala / cvalb), 64);
  }

  //Bounds checks for UNSIGNED (32,64 bitwidths) arith (add, multiply only):
  // based on http://stackoverflow.com/questions/199333/how-to-detect-integer-overflow-in-c-c
  inline u32 _BinOpAddCu32WithBoundsCheck(u32 cvala, u32 cvalb)
  {
    if (cvala > (U32_MAX - cvalb)) /* `a + b` would overflow */
      return U32_MAX;
    return _Cu32ToUnsigned32((cvala + cvalb), 32);
  }

  inline u64 _BinOpAddCu64WithBoundsCheck(u64 cvala, u64 cvalb)
  {
    if (cvala > (U64_MAX - cvalb)) /* `a + b` would overflow */
      return U64_MAX;
    return _Cu64ToUnsigned64((cvala + cvalb), 64);
  }

  inline u32 _BinOpMultiplyCu32WithBoundsCheck(u32 cvala, u32 cvalb)
  {
    if((cvalb > 0) && (cvala > (U32_MAX / cvalb))) /* `a * b` would overflow */
      return U32_MAX;
    return _Cu32ToUnsigned32((cvala * cvalb), 32);
  }

  inline u64 _BinOpMultiplyCu64WithBoundsCheck(u64 cvala, u64 cvalb)
  {
    if((cvalb > 0) && (cvala > (U64_MAX / cvalb))) /* `a * b` would overflow */
      return U64_MAX;
    return _Cu64ToUnsigned64((cvala * cvalb), 64);
  }

  // Ariths On INT:

  inline u32 _BinOpAddInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32	cvalb = _Int32ToCs32(valb, bitwidth);
    if(bitwidth == 32)
      {
	//TODO: replace with machine-specific overflow-bit check
	return _BinOpAddCs32WithBoundsCheck(cvala, cvalb);
      }
    return _Cs32ToInt32((cvala + cvalb), bitwidth);
  }

  inline u64 _BinOpAddInt64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64	cvalb = _Int64ToCs64(valb, bitwidth);
    if(bitwidth == 64)
      {
	//TODO: replace with machine-specific overflow-bit check
	return _BinOpAddCs64WithBoundsCheck(cvala, cvalb);
      }
    return _Cs64ToInt64((cvala + cvalb), bitwidth);
  }

  inline u32 _BinOpSubtractInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32	cvalb = _Int32ToCs32(valb, bitwidth);
    if(bitwidth == 32)
      {
	//TODO: replace with machine-specific overflow-bit check
	return _BinOpSubtractCs32WithBoundsCheck(cvala, cvalb);
      }
    return _Cs32ToInt32((cvala - cvalb), bitwidth);
  }

  inline u64 _BinOpSubtractInt64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64	cvalb = _Int64ToCs64(valb, bitwidth);
    if(bitwidth == 64)
      {
	//TODO: replace with machine-specific overflow-bit check
	return _BinOpSubtractCs64WithBoundsCheck(cvala, cvalb);
      }
    return _Cs64ToInt64((cvala - cvalb), bitwidth);
  }

  inline u32 _BinOpMultiplyInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32	cvalb = _Int32ToCs32(valb, bitwidth);
    if(bitwidth == 32)
      {
	//TODO: replace with machine-specific overflow-bit check
	return _BinOpMultiplyCs32WithBoundsCheck(cvala, cvalb);
      }
    return _Cs32ToInt32((cvala * cvalb), bitwidth);
  }

  inline u64 _BinOpMultiplyInt64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64	cvalb = _Int64ToCs64(valb, bitwidth);
    if(bitwidth == 64)
      {
	//TODO: replace with machine-specific overflow-bit check
	return _BinOpMultiplyCs64WithBoundsCheck(cvala, cvalb);
      }
    return _Cs64ToInt64((cvala * cvalb), bitwidth);
  }

  inline u32 _BinOpDivideInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32	cvalb = _Int32ToCs32(valb, bitwidth);
    if(bitwidth == 32)
      {
	//TODO: replace with machine-specific overflow-bit check
	return _BinOpDivideCs32WithBoundsCheck(cvala, cvalb);
      }
    return _Cs32ToInt32((cvala / cvalb), bitwidth);
  }

  inline u64 _BinOpDivideInt64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64	cvalb = _Int64ToCs64(valb, bitwidth);
    if(bitwidth == 64)
      {
	//TODO: replace with machine-specific overflow-bit check
	return _BinOpDivideCs64WithBoundsCheck(cvala, cvalb);
      }
    return _Cs64ToInt64((cvala / cvalb), bitwidth);
  }

  inline u32 _BinOpModInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32	cvalb = _Int32ToCs32(valb, bitwidth);
    return _Cs32ToInt32((cvala % cvalb), bitwidth);
  }

  inline u64 _BinOpModInt64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64	cvalb = _Int64ToCs64(valb, bitwidth);
    return _Cs64ToInt64((cvala % cvalb), bitwidth);
  }

  // Ariths On UNSIGNED:
  inline u32 _BinOpAddUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32 cvalb = _Unsigned32ToCu32(valb, bitwidth);
    if(bitwidth == 32)
      {
	//TODO: replace with machine-specific overflow-bit check
	return _BinOpAddCu32WithBoundsCheck(cvala, cvalb);
      }
    return _Cu32ToUnsigned32((cvala + cvalb), bitwidth);
  }

  inline u64 _BinOpAddUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64 cvalb = _Unsigned64ToCu64(valb, bitwidth);
    if(bitwidth == 64)
      {
	//TODO: replace with machine-specific overflow-bit check
	return _BinOpAddCu64WithBoundsCheck(cvala, cvalb);
      }
    return _Cu64ToUnsigned64((cvala + cvalb), bitwidth);
  }

  inline u32 _BinOpSubtractUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32 cvalb = _Unsigned32ToCu32(valb, bitwidth);
    //no special bounds checking needed for Unsigned subtraction
    if (cvala <= cvalb) return 0;
    return _Cu32ToUnsigned32((cvala - cvalb), bitwidth);
  }

  inline u64 _BinOpSubtractUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64 cvalb = _Unsigned64ToCu64(valb, bitwidth);
    //no special bounds checking needed for Unsigned subtraction
    if (cvala <= cvalb) return 0;
    return _Cu64ToUnsigned64((cvala - cvalb), bitwidth);
  }

  inline u32 _BinOpMultiplyUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32 cvalb = _Unsigned32ToCu32(valb, bitwidth);
    if(bitwidth == 32)
      {
	//TODO: replace with machine-specific overflow-bit check
	return _BinOpMultiplyCu32WithBoundsCheck(cvala, cvalb);
      }
    return _Cu32ToUnsigned32((cvala * cvalb), bitwidth);
  }

  inline u64 _BinOpMultiplyUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64 cvalb = _Unsigned64ToCu64(valb, bitwidth);
    if(bitwidth == 64)
      {
	//TODO: replace with machine-specific overflow-bit check
	return _BinOpMultiplyCu64WithBoundsCheck(cvala, cvalb);
      }
    return _Cu64ToUnsigned64((cvala * cvalb), bitwidth);
  }

  inline u32 _BinOpDivideUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32 cvalb = _Unsigned32ToCu32(valb, bitwidth);
    //no special bounds checking needed for Unsigned division
    return _Cu32ToUnsigned32((cvala / cvalb), bitwidth);
  }

  inline u64 _BinOpDivideUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64 cvalb = _Unsigned64ToCu64(valb, bitwidth);
    //no special bounds checking needed for Unsigned division
    return _Cu64ToUnsigned64((cvala / cvalb), bitwidth);
  }

  inline u32 _BinOpModUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32 cvalb = _Unsigned32ToCu32(valb, bitwidth);
    return _Cu32ToUnsigned32((cvala % cvalb), bitwidth);
  }

  inline u64 _BinOpModUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64 cvalb = _Unsigned64ToCu64(valb, bitwidth);
    return _Cu64ToUnsigned64((cvala % cvalb), bitwidth);
  }

  //Bin Op Arith on Unary (e.g. op equals)
  //convert to binary before the operation; then convert back to unary
  //no special bounds checking needed for Unary arithmetic
  inline u32 _BinOpAddUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Unary32ToCu32(vala, bitwidth);
    u32 binvalb = _Unary32ToCu32(valb, bitwidth);
    return _Cu32ToUnary32(binvala + binvalb, bitwidth);
  }

  inline u64 _BinOpAddUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Unary64ToCu64(vala, bitwidth);
    u64 binvalb = _Unary64ToCu64(valb, bitwidth);
    return _Cu64ToUnary64(binvala + binvalb, bitwidth);
  }

  inline u32 _BinOpSubtractUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 binvala = _Unary32ToCs32(vala, bitwidth);
    s32 binvalb = _Unary32ToCs32(valb, bitwidth);
    return _Cs32ToUnary32((binvala - binvalb), bitwidth);
  }

  inline u64 _BinOpSubtractUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 binvala = _Unary64ToCs64(vala, bitwidth);
    s64 binvalb = _Unary64ToCs64(valb, bitwidth);
    return _Cs64ToUnary64((binvala - binvalb), bitwidth);
  }

  inline u32 _BinOpMultiplyUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Unary32ToCu32(vala, bitwidth);
    u32 binvalb = _Unary32ToCu32(valb, bitwidth);
    return _Cu32ToUnary32(binvala * binvalb, bitwidth);
  }

  inline u64 _BinOpMultiplyUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Unary64ToCu64(vala, bitwidth);
    u64 binvalb = _Unary64ToCu64(valb, bitwidth);
    return _Cu64ToUnary64(binvala * binvalb, bitwidth);
  }

  inline u32 _BinOpDivideUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 binvala = _Unary32ToCu32(vala, bitwidth);
    u32 binvalb = _Unary32ToCu32(valb, bitwidth);
    return _Cu32ToUnary32(binvala / binvalb, bitwidth);
  }

  inline u64 _BinOpDivideUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 binvala = _Unary64ToCu64(vala, bitwidth);
    u64 binvalb = _Unary64ToCu64(valb, bitwidth);
    return _Cu64ToUnary64(binvala / binvalb, bitwidth);
  }

  inline u32 _BinOpModUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 binvala = _Unary32ToCu32(vala, bitwidth);
    u32 binvalb = _Unary32ToCu32(valb, bitwidth);
    return _Cu32ToUnary32(binvala % binvalb, bitwidth);
  }

  inline u64 _BinOpModUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 binvala = _Unary64ToCu64(vala, bitwidth);
    u64 binvalb = _Unary64ToCu64(valb, bitwidth);
    return _Cu64ToUnary64(binvala % binvalb, bitwidth);
  }


  //Bin Op Arith on Bool (e.g. op equals)
  //convert to binary before the operation; then convert back to bool
  inline u32 _BinOpAddBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 destbitwidth = (bitwidth > 1 ? bitwidth : 32);
    u32 binvala = _Bool32ToCu32(vala, bitwidth);
    u32 binvalb = _Bool32ToCu32(valb, bitwidth);
    u32 rtnval = _Cu32ToUnsigned32(binvala + binvalb, destbitwidth);
    return _Unsigned32ToBool32(rtnval, destbitwidth, bitwidth);
  }

  inline u64 _BinOpAddBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u32 destbitwidth = (bitwidth > 1 ? bitwidth : 32);
    u64 binvala = _Bool64ToCu64(vala, bitwidth);
    u64 binvalb = _Bool64ToCu64(valb, bitwidth);
    u64 rtnval = _Cu64ToUnsigned64(binvala + binvalb, destbitwidth);
    return _Unsigned64ToBool64(rtnval, destbitwidth, bitwidth);
  }

  inline u32 _BinOpSubtractBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 destbitwidth = (bitwidth > 1 ? bitwidth : 32);
    s32 binvala = _Bool32ToCs32(vala, bitwidth);
    s32 binvalb = _Bool32ToCs32(valb, bitwidth);
    s32 rtnval = _Cs32ToInt32(binvala - binvalb, destbitwidth);
    return _Int32ToBool32(rtnval, destbitwidth, bitwidth);
  }

  inline u64 _BinOpSubtractBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u32 destbitwidth = (bitwidth > 1 ? bitwidth : 32);
    s64 binvala = _Bool64ToCs64(vala, bitwidth);
    s64 binvalb = _Bool64ToCs64(valb, bitwidth);
    u32 rtnval = _Cs64ToInt64(binvala - binvalb, destbitwidth);
    return _Int64ToBool64(rtnval, destbitwidth, bitwidth);
  }

  inline u32 _BinOpMultiplyBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToCu32(vala, bitwidth);
    u32 binvalb = _Bool32ToCu32(valb, bitwidth);
    u32 rtnval = _Cu32ToUnsigned32(binvala * binvalb, bitwidth);
    return _Unsigned32ToBool32(rtnval, bitwidth, bitwidth);
  }

  inline u64 _BinOpMultiplyBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToCu64(vala, bitwidth);
    u64 binvalb = _Bool64ToCu64(valb, bitwidth);
    u64 rtnval = _Cu64ToUnsigned64(binvala * binvalb, bitwidth);
    return _Unsigned64ToBool64(rtnval, bitwidth, bitwidth);
  }

  inline u32 _BinOpDivideBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 binvala = _Bool32ToCu32(vala, bitwidth);
    u32 binvalb = _Bool32ToCu32(valb, bitwidth);
    u32 rtnval = _Cu32ToUnsigned32(binvala / binvalb, bitwidth);
    return _Unsigned32ToBool32(rtnval, bitwidth, bitwidth);
  }

  inline u64 _BinOpDivideBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 binvala = _Bool64ToCu64(vala, bitwidth);
    u64 binvalb = _Bool64ToCu64(valb, bitwidth);
    u64 rtnval = _Cu64ToUnsigned64(binvala / binvalb, bitwidth);
    return _Unsigned64ToBool64(rtnval, bitwidth, bitwidth);
  }

  inline u32 _BinOpModBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 binvala = _Bool32ToCu32(vala, bitwidth);
    u32 binvalb = _Bool32ToCu32(valb, bitwidth);
    u32 rtnval = _Cu32ToUnsigned32(binvala % binvalb, bitwidth);
    return _Unsigned32ToBool32(rtnval, bitwidth, bitwidth);
  }

  inline u64 _BinOpModBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 binvala = _Bool64ToCu64(vala, bitwidth);
    u64 binvalb = _Bool64ToCu64(valb, bitwidth);
    u64 rtnval = _Cu64ToUnsigned64(binvala % binvalb, bitwidth);
    return _Unsigned64ToBool64(rtnval, bitwidth, bitwidth);
  }

  //COMPARISONS

  //CompOps on INTS:
  inline u32 _BinOpCompareEqEqInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32	cvalb = _Int32ToCs32(valb, bitwidth);
    return _CboolToBool32(cvala == cvalb, BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareEqEqInt64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64	cvalb = _Int64ToCs64(valb, bitwidth);
    return _CboolToBool64(cvala == cvalb, BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareNotEqInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32	cvalb = _Int32ToCs32(valb, bitwidth);
    return _CboolToBool32(cvala != cvalb, BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareNotEqInt64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64	cvalb = _Int64ToCs64(valb, bitwidth);
    return _CboolToBool64(cvala != cvalb, BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessThanInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32	cvalb = _Int32ToCs32(valb, bitwidth);
    return _CboolToBool32(cvala < cvalb, BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessThanInt64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64	cvalb = _Int64ToCs64(valb, bitwidth);
    return _CboolToBool64(cvala < cvalb, BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterThanInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32	cvalb = _Int32ToCs32(valb, bitwidth);
    return _CboolToBool32(cvala > cvalb, BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterThanInt64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64	cvalb = _Int64ToCs64(valb, bitwidth);
    return _CboolToBool64(cvala > cvalb, BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessEqualInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32	cvalb = _Int32ToCs32(valb, bitwidth);
    return _CboolToBool32(cvala <= cvalb, BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessEqualInt64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64	cvalb = _Int64ToCs64(valb, bitwidth);
    return _CboolToBool64(cvala <= cvalb, BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterEqualInt32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    s32	cvalb = _Int32ToCs32(valb, bitwidth);
    return _CboolToBool32(cvala >= cvalb, BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterEqualInt64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    s64	cvalb = _Int64ToCs64(valb, bitwidth);
    return _CboolToBool64(cvala >= cvalb, BITS_PER_BOOL);
  }

  //CompOps on UNSIGNED
  inline u32 _BinOpCompareEqEqUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32	cvalb = _Unsigned32ToCu32(valb, bitwidth);
    return _CboolToBool32((cvala == cvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareEqEqUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64	cvalb = _Unsigned64ToCu64(valb, bitwidth);
    return _CboolToBool64((cvala == cvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareNotEqUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32	cvalb = _Unsigned32ToCu32(valb, bitwidth);
    return _CboolToBool32((cvala != cvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareNotEqUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64	cvalb = _Unsigned64ToCu64(valb, bitwidth);
    return _CboolToBool64((cvala != cvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessThanUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32	cvalb = _Unsigned32ToCu32(valb, bitwidth);
    return _CboolToBool32((cvala < cvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessThanUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64	cvalb = _Unsigned64ToCu64(valb, bitwidth);
    return _CboolToBool64((cvala < cvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterThanUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32	cvalb = _Unsigned32ToCu32(valb, bitwidth);
    return _CboolToBool32((cvala > cvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterThanUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64	cvalb = _Unsigned64ToCu64(valb, bitwidth);
    return _CboolToBool64((cvala > cvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessEqualUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32	cvalb = _Unsigned32ToCu32(valb, bitwidth);
    return _CboolToBool32((cvala <= cvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessEqualUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64	cvalb = _Unsigned64ToCu64(valb, bitwidth);
    return _CboolToBool64((cvala <= cvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterEqualUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    u32	cvalb = _Unsigned32ToCu32(valb, bitwidth);
    return _CboolToBool32((cvala >= cvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterEqualUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    u64	cvalb = _Unsigned64ToCu64(valb, bitwidth);
    return _CboolToBool64((cvala >= cvalb), BITS_PER_BOOL);
  }

  //CompOps on BITS
  inline u32 _BinOpCompareEqEqBits32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Bits32ToCu32(vala, bitwidth);
    u32	cvalb = _Bits32ToCu32(valb, bitwidth);
    return _CboolToBool32((cvala == cvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareEqEqBits64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Bits64ToCu64(vala, bitwidth);
    u64	cvalb = _Bits64ToCu64(valb, bitwidth);
    return _CboolToBool64((cvala == cvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareNotEqBits32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 cvala = _Bits32ToCu32(vala, bitwidth);
    u32	cvalb = _Bits32ToCu32(valb, bitwidth);
    return _CboolToBool32((cvala != cvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareNotEqBits64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 cvala = _Bits64ToCu64(vala, bitwidth);
    u64	cvalb = _Bits64ToCu64(valb, bitwidth);
    return _CboolToBool64((cvala != cvalb), BITS_PER_BOOL);
  }

  //CompOps on BOOL
  inline u32 _BinOpCompareEqEqBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToCu32(vala, bitwidth);
    u32 binvalb = _Bool32ToCu32(valb, bitwidth);
    return _CboolToBool32((binvala == binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareEqEqBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToCu64(vala, bitwidth);
    u64 binvalb = _Bool64ToCu64(valb, bitwidth);
    return _CboolToBool64((binvala == binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareNotEqBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToCu32(vala, bitwidth);
    u32 binvalb = _Bool32ToCu32(valb, bitwidth);
    return _CboolToBool32((binvala != binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareNotEqBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToCu64(vala, bitwidth);
    u64 binvalb = _Bool64ToCu64(valb, bitwidth);
    return _CboolToBool64((binvala != binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessThanBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToCu32(vala, bitwidth);
    u32 binvalb = _Bool32ToCu32(valb, bitwidth);
    return _CboolToBool32((binvala < binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessThanBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToCu64(vala, bitwidth);
    u64 binvalb = _Bool64ToCu64(valb, bitwidth);
    return _CboolToBool64((binvala < binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterThanBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToCu32(vala, bitwidth);
    u32 binvalb = _Bool32ToCu32(valb, bitwidth);
    return _CboolToBool32((binvala > binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterThanBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToCu64(vala, bitwidth);
    u64 binvalb = _Bool64ToCu64(valb, bitwidth);
    return _CboolToBool64((binvala > binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessEqualBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToCu32(vala, bitwidth);
    u32 binvalb = _Bool32ToCu32(valb, bitwidth);
    return _CboolToBool32((binvala <= binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessEqualBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToCu64(vala, bitwidth);
    u64 binvalb = _Bool64ToCu64(valb, bitwidth);
    return _CboolToBool64((binvala <= binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterEqualBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToCu32(vala, bitwidth);
    u32 binvalb = _Bool32ToCu32(valb, bitwidth);
    return _CboolToBool32((binvala >= binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterEqualBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToCu64(vala, bitwidth);
    u64 binvalb = _Bool64ToCu64(valb, bitwidth);
    return _CboolToBool64((binvala >= binvalb), BITS_PER_BOOL);
  }

  //CompOps on UNARY
  inline u32 _BinOpCompareEqEqUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Unary32ToCu32(vala, bitwidth);
    u32 binvalb = _Unary32ToCu32(valb, bitwidth);
    return _CboolToBool32((binvala == binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareEqEqUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Unary64ToCu64(vala, bitwidth);
    u64 binvalb = _Unary64ToCu64(valb, bitwidth);
    return _CboolToBool64((binvala == binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareNotEqUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Unary32ToCu32(vala, bitwidth);
    u32 binvalb = _Unary32ToCu32(valb, bitwidth);
    return _CboolToBool32((binvala != binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareNotEqUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Unary64ToCu64(vala, bitwidth);
    u64 binvalb = _Unary64ToCu64(valb, bitwidth);
    return _CboolToBool64((binvala != binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessThanUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Unary32ToCu32(vala, bitwidth);
    u32 binvalb = _Unary32ToCu32(valb, bitwidth);
    return _CboolToBool32((binvala < binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessThanUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Unary64ToCu64(vala, bitwidth);
    u64 binvalb = _Unary64ToCu64(valb, bitwidth);
    return _CboolToBool64((binvala < binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterThanUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Unary32ToCu32(vala, bitwidth);
    u32 binvalb = _Unary32ToCu32(valb, bitwidth);
    return _CboolToBool32((binvala > binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterThanUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Unary64ToCu64(vala, bitwidth);
    u64 binvalb = _Unary64ToCu64(valb, bitwidth);
    return _CboolToBool64((binvala > binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessEqualUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Unary32ToCu32(vala, bitwidth);
    u32 binvalb = _Unary32ToCu32(valb, bitwidth);
    return _CboolToBool32((binvala <= binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessEqualUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Unary64ToCu64(vala, bitwidth);
    u64 binvalb = _Unary64ToCu64(valb, bitwidth);
    return _CboolToBool64((binvala <= binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterEqualUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Unary32ToCu32(vala, bitwidth);
    u32 binvalb = _Unary32ToCu32(valb, bitwidth);
    return _CboolToBool32((binvala >= binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterEqualUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Unary64ToCu64(vala, bitwidth);
    u64 binvalb = _Unary64ToCu64(valb, bitwidth);
    return _CboolToBool64((binvala >= binvalb), BITS_PER_BOOL);
  }

  //SHIFT OPS
  //Shift INTS:
  inline u32 _ShiftOpRightInt32(u32 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 32) return 0;
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    return _Cs32ToInt32((cvala >> shft), bitwidth);
  }

  inline u64 _ShiftOpRightInt64(u64 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 64) return 0;
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    return _Cs64ToInt64((cvala >> shft), bitwidth);
   }

  inline u32 _ShiftOpLeftInt32(u32 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 32) return 0; //instead of self
    s32 cvala = _Int32ToCs32(vala, bitwidth);
    return _Cs32ToInt32((cvala << shft), bitwidth);
  }

  inline u64 _ShiftOpLeftInt64(u64 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 64) return 0; //instead of self
    s64 cvala = _Int64ToCs64(vala, bitwidth);
    return _Cs64ToInt64((cvala >> shft), bitwidth);
  }

  //Shift UNSIGNED:
  inline u32 _ShiftOpRightUnsigned32(u32 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 32) return 0;
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    return _Cu32ToUnsigned32((cvala >> shft), bitwidth);
  }

  inline u64 _ShiftOpRightUnsigned64(u64 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 64) return 0;
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    return _Cu64ToUnsigned64((cvala >> shft), bitwidth);
  }

  inline u32 _ShiftOpLeftUnsigned32(u32 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 32) return 0; //instead of self
    u32 cvala = _Unsigned32ToCu32(vala, bitwidth);
    return _Cu32ToUnsigned32((cvala << shft), bitwidth);
  }

  inline u64 _ShiftOpLeftUnsigned64(u64 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 64) return 0; //instead of self
    u64 cvala = _Unsigned64ToCu64(vala, bitwidth);
    return _Cu64ToUnsigned64((cvala << shft), bitwidth);
  }

  //Shift BOOL:
  inline u32 _ShiftOpRightBool32(u32 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 32) return 0;
    u32 binvala = _Bool32ToBits32(vala, bitwidth, bitwidth);
    return _Bits32ToBool32((binvala >> shft), bitwidth, bitwidth);
  }

  inline u64 _ShiftOpRightBool64(u64 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 64) return 0;
    u32 binvala = _Bool64ToBits64(vala, bitwidth, bitwidth);
    return _Bits64ToBool64((binvala >> shft), bitwidth, bitwidth);
  }

  inline u32 _ShiftOpLeftBool32(u32 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 32) return 0; //instead of self
    u32 binvala = _Bool32ToBits32(vala, bitwidth, bitwidth);
    return _Bits32ToBool32((binvala << shft), bitwidth, bitwidth);
  }

  inline u64 _ShiftOpLeftBool64(u64 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 64) return 0; //instead of self
    u32 binvala = _Bool64ToBits64(vala, bitwidth, bitwidth);
    return _Bits64ToBool64((binvala >> shft), bitwidth, bitwidth);
  }

  //Shift UNARY:
  inline u32 _ShiftOpRightUnary32(u32 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 32) return 0;
    u32 cvala = _Unary32ToCu32(vala, bitwidth);
    return _Cu32ToUnary32((cvala >> shft), bitwidth);
  }

  inline u64 _ShiftOpRightUnary64(u64 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 64) return 0;
    u64 cvala = _Unary64ToCu64(vala, bitwidth);
    return _Cu64ToUnary64((cvala >> shft), bitwidth);
  }

  inline u32 _ShiftOpLeftUnary32(u32 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 32) return 0; //instead of self
    u32 cvala = _Unary32ToCu32(vala, bitwidth);
    return _Cu32ToUnary32((cvala << shft), bitwidth);
  }

  inline u64 _ShiftOpLeftUnary64(u64 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 64) return 0; //instead of self
    u64 cvala = _Unary64ToCu64(vala, bitwidth);
    return _Cu64ToUnary64((cvala << shft), bitwidth);
  }

  //Shift BITS:
  inline u32 _ShiftOpRightBits32(u32 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 32) return 0;
    u32 cvala = _Bits32ToCu32(vala, bitwidth);
    return _Cu32ToBits32((cvala >> shft), bitwidth);
  }

  inline u64 _ShiftOpRightBits64(u64 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 64) return 0;
    u64 cvala = _Bits64ToCu64(vala, bitwidth);
    return _Cu64ToBits64((cvala >> shft), bitwidth);
  }

  inline u32 _ShiftOpLeftBits32(u32 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 32) return 0; //instead of self
    u32 cvala = _Bits32ToCu32(vala, bitwidth);
    return _Cu32ToBits32((cvala << shft), bitwidth);
  }

  inline u64 _ShiftOpLeftBits64(u64 vala, u32 shft, u32 bitwidth)
  {
    if(shft >= 64) return 0; //instead of self
    u64 cvala = _Bits64ToCu64(vala, bitwidth);
    return _Cu64ToBits64((cvala << shft), bitwidth);
  }

}

#endif /* CASTOPS_H */
