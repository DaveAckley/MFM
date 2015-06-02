/* -*- mode:C++ -*- */
/**
  CastOps.h Primitive casting and ALU operations
  Copyright (C) 2014 The Regents of the University of New Mexico.
  Copyright (C) 2014 Ackleyshack LLC.

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
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef CASTOPS_H
#define CASTOPS_H

#include "itype.h"
#include "Util.h"
#include "Fail.h"

namespace MFM {

  //Casts
  inline s32 _Int32ToInt32(s32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    const s32 maxdestval = _GetNOnes32(destbitwidth-1);  //positive
    const s32 mindestval = ~maxdestval;
    return CLAMP<s32>(mindestval, maxdestval, val);
  }

  inline s64 _Int64ToInt64(s64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    const s64 maxdestval = _GetNOnes64(destbitwidth-1);
    const s64 mindestval = ~maxdestval;
    return CLAMP<s64>(mindestval, maxdestval, val);
  }

  inline s32 _Unsigned32ToInt32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u32 maxdestval = _GetNOnes32(destbitwidth - 1);
    return (s32) MIN<u32>(val, maxdestval);
  }

  inline s64 _Unsigned64ToInt64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    u64 maxdestval = _GetNOnes64(destbitwidth - 1);
    return (s64) MIN<u64>(val, maxdestval);
  }

  inline s32 _Bool32ToInt32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 count1s = PopCount(val & _GetNOnes32(srcbitwidth));
    return (s32) ((count1s > (s32) (srcbitwidth - count1s)) ? 1 : 0);
  }

  inline s64 _Bool64ToInt64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 count1s = PopCount(val & _GetNOnes64(srcbitwidth));
    return (s64) ((count1s > (s32) (srcbitwidth - count1s)) ? 1 : 0);
  }

  inline s32 _Unary32ToInt32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return PopCount(val & _GetNOnes32(srcbitwidth));
  }

  inline s64 _Unary64ToInt64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (s64) PopCount(val & _GetNOnes64(srcbitwidth));
  }

  inline s32 _Bits32ToInt32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (s32) (val & _GetNOnes32(srcbitwidth)); // no sign extend
  }

  inline s64 _Bits64ToInt64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (s64) (val & _GetNOnes64(srcbitwidth)); // no sign extend
  }

//To BOOL:
  inline u32 _Bool32ToBool32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 count1s = PopCount(val & _GetNOnes32(srcbitwidth));
    // == when even number bits is ignored (warning at def)
    return (u32) ((count1s > (s32) (srcbitwidth - count1s)) ? _GetNOnes32(destbitwidth) : 0);
  }

  inline u32 _Bool64ToBool64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 count1s = PopCount(val & _GetNOnes64(srcbitwidth));
    // == when even number bits is ignored (warning at def)
    return (u64) ((count1s > (s32) (srcbitwidth - count1s)) ? _GetNOnes64(destbitwidth) : 0);
  }

  inline u32 _Int32ToBool32(s32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return  (u32) (((val & _GetNOnes32(srcbitwidth)) != 0) ? _GetNOnes32(destbitwidth) : 0);
  }

  inline u64 _Int64ToBool64(s64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return  (u64) (((val & _GetNOnes64(srcbitwidth)) != 0) ? _GetNOnes64(destbitwidth) : 0);
  }

  inline u32 _Unsigned32ToBool32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return  (u32) (((val & _GetNOnes32(srcbitwidth)) != 0) ? _GetNOnes32(destbitwidth) : 0);
  }

  inline u64 _Unsigned64ToBool64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return  (u64) (((val & _GetNOnes64(srcbitwidth)) != 0) ? _GetNOnes64(destbitwidth) : 0);
  }

  inline u32 _Unary32ToBool32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return  (u32) (((val & _GetNOnes32(srcbitwidth)) != 0) ? _GetNOnes32(destbitwidth) : 0);
  }

  inline u64 _Unary64ToBool64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return  (u64) (((val & _GetNOnes64(srcbitwidth)) != 0) ? _GetNOnes64(destbitwidth) : 0);
  }

  inline u32 _Bits32ToBool32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (val & _GetNOnes32(MIN<u32>(srcbitwidth, destbitwidth))); //no change to Bit data
  }

  inline u64 _Bits64ToBool64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (val & _GetNOnes64(MIN<u32>(srcbitwidth, destbitwidth))); //no change to Bit data
  }

  inline bool _Bool32ToCbool(u32 val, const u32 srcbitwidth)
  {
    s32 count1s = PopCount(val & _GetNOnes32(srcbitwidth));
    return (count1s > (s32) (srcbitwidth - count1s));  // == when even number bits is ignored (warning at def)
  }

  inline bool _Bool64ToCbool(u64 val, const u32 srcbitwidth)
  {
    s32 count1s = PopCount(val & _GetNOnes64(srcbitwidth));
    return (count1s > (s32) (srcbitwidth - count1s));  // == when even number bits is ignored (warning at def)
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
      return _GetNOnes32(destbitwidth); //saturate
    return 0;
  }

  //To UNSIGNED:
  inline u32 _Int32ToUnsigned32(s32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    if(val <= 0)
      return 0;

    const u32 maxdestval = _GetNOnes32(destbitwidth);
    const u32 mindestval = 0;
    return CLAMP<u32>(mindestval, maxdestval, (u32) val);
  }

  inline u64 _Int64ToUnsigned64(s64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    if(val <= 0)
      return 0;

    const u64 maxdestval = _GetNOnes64(destbitwidth);
    const u64 mindestval = 0;
    return CLAMP<u64>(mindestval, maxdestval, (u64) val);
  }

  inline u32 _Unsigned32ToUnsigned32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    const u32 maxdestval = _GetNOnes32(destbitwidth);
    const u32 mindestval = 0;
    return CLAMP<u32>(mindestval, maxdestval, val);
  }

  inline u64 _Unsigned64ToUnsigned64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    const u64 maxdestval = _GetNOnes64(destbitwidth);
    const u64 mindestval = 0;
    return CLAMP<u64>(mindestval, maxdestval, val);
  }

  inline u32 _Bool32ToUnsigned32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 count1s = PopCount(val & _GetNOnes32(srcbitwidth));
    return ((count1s > (s32) (srcbitwidth - count1s)) ? 1 : 0);
  }

  inline u64 _Bool64ToUnsigned64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 count1s = PopCount(val & _GetNOnes64(srcbitwidth));
    return ((count1s > (s32) (srcbitwidth - count1s)) ? 1 : 0);
  }

  inline u32 _Unary32ToUnsigned32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return PopCount(val & _GetNOnes32(srcbitwidth));
  }

  inline u64 _Unary64ToUnsigned64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return PopCount(val & _GetNOnes64(srcbitwidth));
  }

  inline u32 _Bits32ToUnsigned32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (val & _GetNOnes32(srcbitwidth));
  }

  inline u64 _Bits64ToUnsigned64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (val & _GetNOnes64(srcbitwidth));
  }

  //To BITS:
  inline u32 _Int32ToBits32(s32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (u32) (val & _GetNOnes32(srcbitwidth));
  }

  inline u64 _Int64ToBits64(s64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (u64) (val & _GetNOnes64(srcbitwidth));
  }

  inline u32 _Unsigned32ToBits32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return val & _GetNOnes32(srcbitwidth);
  }

  inline u64 _Unsigned64ToBits64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return val & _GetNOnes64(srcbitwidth);
  }

  inline u32 _Bool32ToBits32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return val & _GetNOnes32(srcbitwidth);
  }

  inline u64 _Bool64ToBits64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return val & _GetNOnes64(srcbitwidth);
  }

  inline u32 _Unary32ToBits32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return val & _GetNOnes32(srcbitwidth);
  }

  inline u64 _Unary64ToBits64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return val & _GetNOnes64(srcbitwidth);
  }

  inline u32 _Bits32ToBits32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return val & _GetNOnes32(srcbitwidth);
  }

  inline u64 _Bits64ToBits64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return val & _GetNOnes64(srcbitwidth);
  }

  //To UNARY:
  inline u32 _Int32ToUnary32(s32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    const s32 maxdestval = destbitwidth;
    const s32 mindestval = 0;
    return (u32) _GetNOnes32(CLAMP<s32>(mindestval, maxdestval, val));
  }

  inline u64 _Int64ToUnary64(s64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    const s64 maxdestval = destbitwidth;
    const s64 mindestval = 0;
    return (u64) _GetNOnes64(CLAMP<s64>(mindestval, maxdestval, val));
  }

  inline u32 _Unsigned32ToUnary32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    const u32 maxdestval = destbitwidth;
    const u32 mindestval = 0;
    return _GetNOnes32(CLAMP<u32>(mindestval, maxdestval, val));
  }

  inline u64 _Unsigned64ToUnary64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    const u64 maxdestval = destbitwidth;
    const u64 mindestval = 0;
    return _GetNOnes64(CLAMP<u64>(mindestval, maxdestval, val));
  }

  inline u32 _Bool32ToUnary32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 count1s = PopCount(val & _GetNOnes32(srcbitwidth));
    return ((count1s > (s32) (srcbitwidth - count1s)) ? 1 : 0);
  }

  inline u64 _Bool64ToUnary64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    s32 count1s = PopCount(val & _GetNOnes64(srcbitwidth));
    return ((count1s > (s32) (srcbitwidth - count1s)) ? 1 : 0);
  }

  inline u32 _Unary32ToUnary32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    const u32 maxdestval = destbitwidth;
    const u32 mindestval = 0;
    u32 count1s = PopCount(val & _GetNOnes32(srcbitwidth));
    return _GetNOnes32(CLAMP<u32>(mindestval, maxdestval, count1s));
  }

  inline u64 _Unary64ToUnary64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    const u32 maxdestval = destbitwidth;
    const u32 mindestval = 0;
    u32 count1s = PopCount(val & _GetNOnes64(srcbitwidth));
    return _GetNOnes64(CLAMP<u32>(mindestval, maxdestval, count1s));
  }

  inline u32 _Bits32ToUnary32(u32 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (val & _GetNOnes32(MIN<u32>(srcbitwidth, destbitwidth)));
  }

  inline u64 _Bits64ToUnary64(u64 val, const u32 srcbitwidth, const u32 destbitwidth)
  {
    return (val & _GetNOnes64(MIN<u32>(srcbitwidth, destbitwidth)));
  }

  //'ALU' ops
  enum { BITS_PER_BOOL = 1 };

  //LOGICAL BANG:
  inline u32 _LogicalBangBool32(u32 val, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    s32 count1s = PopCount(val & mask);
    return ((count1s > (s32) (bitwidth - count1s)) ? 0 : mask);  // == when even number bits is ignored (warning at def)
  }

  inline u64 _LogicalBangBool64(u64 val, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    s32 count1s = PopCount(val & mask);
    return ((count1s > (s32) (bitwidth - count1s)) ? 0 : mask);  // == when even number bits is ignored (warning at def)
  }


  inline s32 _UnaryMinusInt32(s32 val, u32 bitwidth)
  {
    return -val;  //assumes sign extended ???
  }

  inline s64 _UnaryMinusInt64(s64 val, u32 bitwidth)
  {
    return -val;  //assumes sign extended ???
  }

  //Bitwise Binary Ops:
  inline u32 _BitwiseOrUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    u32 maska = _GetNOnes32(PopCount(vala & mask));
    u32 maskb = _GetNOnes32(PopCount(valb & mask));
    return maska | maskb;  // "max"
  }

  inline u64 _BitwiseOrUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    u64 maska = _GetNOnes64(PopCount(vala & mask));
    u64 maskb = _GetNOnes64(PopCount(valb & mask));
    return maska | maskb;  //"max"
  }

  inline u32 _BitwiseAndUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    u32 maska = _GetNOnes32(PopCount(vala & mask));
    u32 maskb = _GetNOnes32(PopCount(valb & mask));
    return maska & maskb;  //"min"
  }

  inline u64 _BitwiseAndUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    u64 maska = _GetNOnes64(PopCount(vala & mask));
    u64 maskb = _GetNOnes64(PopCount(valb & mask));
    return maska & maskb;  //"min"
  }

  inline u32 _BitwiseXorUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    u32 counta = PopCount(vala & mask);
    u32 countb = PopCount(valb & mask);
    return _GetNOnes32(MAX<u32>(counta, countb) - MIN<u32>(counta, countb)); //right-justified ^
  }

  inline u64 _BitwiseXorUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    u32 counta = PopCount(vala & mask);
    u32 countb = PopCount(valb & mask);
    return _GetNOnes64(MAX<u32>(counta, countb) - MIN<u32>(counta, countb)); //right-justified ^
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

  inline s32 _BitwiseOrInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return  ( (vala | valb) & mask) ;  // "at least max"
  }

  inline s64 _BitwiseOrInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala | valb) & mask);  //"at least max"
  }

  inline s32 _BitwiseAndInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ( (vala & valb) & mask);  //"at most min"
  }

  inline s64 _BitwiseAndInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala & valb) & mask);  //"at most min"
  }

  inline s32 _BitwiseXorInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ( (vala ^ valb) & mask);
  }

  inline s64 _BitwiseXorInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala ^ valb) & mask);
  }

  inline u32 _BitwiseOrUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return  ( (vala | valb) & mask) ;  // "at least max"
  }

  inline u64 _BitwiseOrUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala | valb) & mask);  //"at least max"
  }

  inline u32 _BitwiseAndUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ( (vala & valb) & mask);  //"at most min"
  }

  inline u64 _BitwiseAndUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala & valb) & mask);  //"at most min"
  }

  inline u32 _BitwiseXorUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ( (vala ^ valb) & mask);
  }

  inline u64 _BitwiseXorUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ( (vala ^ valb) & mask);
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

  // Ariths On INT:
  inline s32 _BinOpAddInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _Int32ToInt32(extvala + extvalb, 32, bitwidth);
  }

  inline s64 _BinOpAddInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _Int64ToInt64(extvala + extvalb, 64, bitwidth);
  }

  inline s32 _BinOpSubtractInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _Int32ToInt32(extvala - extvalb, 32, bitwidth);
  }

  inline s64 _BinOpSubtractInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _Int64ToInt64(extvala - extvalb, 64, bitwidth);
  }

  inline s32 _BinOpMultiplyInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _Int32ToInt32(extvala * extvalb, 32, bitwidth);
  }

  inline s64 _BinOpMultiplyInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _Int64ToInt64(extvala * extvalb, 64, bitwidth);
  }

  inline s32 _BinOpDivideInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _Int32ToInt32(extvala / extvalb, 32, bitwidth);
  }

  inline s64 _BinOpDivideInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _Int64ToInt64(extvala / extvalb, 64, bitwidth);
  }

  inline s32 _BinOpModInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _Int32ToInt32(extvala % extvalb, 32, bitwidth);
  }

  inline s64 _BinOpModInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _Int64ToInt64(extvala % extvalb, 64, bitwidth);
  }

  // Ariths On UNSIGNED:
  inline u32 _BinOpAddUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return _Unsigned32ToUnsigned32((vala & mask) + (valb & mask), bitwidth, bitwidth);
  }

  inline u64 _BinOpAddUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return _Unsigned64ToUnsigned64((vala & mask) + (valb & mask), bitwidth, bitwidth);
  }

  inline u32 _BinOpSubtractUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return _Unsigned32ToUnsigned32((vala & mask) - (valb & mask), bitwidth, bitwidth);
  }

  inline u64 _BinOpSubtractUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return _Unsigned64ToUnsigned64((vala & mask) - (valb & mask), bitwidth, bitwidth);
  }

  inline u32 _BinOpMultiplyUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return _Unsigned32ToUnsigned32((vala & mask) * (valb & mask), bitwidth, bitwidth);
  }

  inline u64 _BinOpMultiplyUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return _Unsigned64ToUnsigned64((vala & mask) * (valb & mask), bitwidth, bitwidth);
  }

  inline u32 _BinOpDivideUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 mask = _GetNOnes32(bitwidth);
    return _Unsigned32ToUnsigned32((vala & mask) / (valb & mask), bitwidth, bitwidth);
  }

  inline u64 _BinOpDivideUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 mask = _GetNOnes64(bitwidth);
    return _Unsigned64ToUnsigned64((vala & mask) / (valb & mask), bitwidth, bitwidth);
  }

  inline u32 _BinOpModUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u32 mask = _GetNOnes32(bitwidth);
    return _Unsigned32ToUnsigned32((vala & mask) % (valb & mask), bitwidth, bitwidth);
  }

  inline u64 _BinOpModUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    u64 mask = _GetNOnes64(bitwidth);
    return _Unsigned64ToUnsigned64((vala & mask) % (valb & mask), bitwidth, bitwidth);
  }

  //Bin Op Arith on Unary (e.g. op equals)
  //convert to binary before the operation; then convert back to unary
  inline u32 _BinOpAddUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 binvala = _Unary32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Unary32ToInt32(valb, bitwidth, 32);
    return _Int32ToUnary32(binvala + binvalb, 32, bitwidth);
  }

  inline u64 _BinOpAddUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 binvala = _Unary64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Unary64ToInt64(valb, bitwidth, 64);
    return _Int64ToUnary64(binvala + binvalb, 64, bitwidth);
  }

  inline u32 _BinOpSubtractUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 binvala = _Unary32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Unary32ToInt32(valb, bitwidth, 32);
    return _Int32ToUnary32(binvala - binvalb, 32, bitwidth);
  }

  inline u64 _BinOpSubtractUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 binvala = _Unary64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Unary64ToInt64(valb, bitwidth, 64);
    return _Int64ToUnary64(binvala - binvalb, 64, bitwidth);
  }

  inline u32 _BinOpMultiplyUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 binvala = _Unary32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Unary32ToInt32(valb, bitwidth, 32);
    return _Int32ToUnary32(binvala * binvalb, 32, bitwidth);
  }

  inline u64 _BinOpMultiplyUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 binvala = _Unary64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Unary64ToInt64(valb, bitwidth, 64);
    return _Int64ToUnary64(binvala * binvalb, 64, bitwidth);
  }

  inline u32 _BinOpDivideUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s32 binvala = _Unary32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Unary32ToInt32(valb, bitwidth, 32);
    return _Int32ToUnary32(binvala / binvalb, 32, bitwidth);
  }

  inline u64 _BinOpDivideUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s64 binvala = _Unary64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Unary64ToInt64(valb, bitwidth, 64);
    return _Int64ToUnary64(binvala / binvalb, 64, bitwidth);
  }

  inline u32 _BinOpModUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s32 binvala = _Unary32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Unary32ToInt32(valb, bitwidth, 32);
    return _Int32ToUnary32(binvala % binvalb, 32, bitwidth);
  }

  inline u64 _BinOpModUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s64 binvala = _Unary64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Unary64ToInt64(valb, bitwidth, 64);
    return _Int64ToUnary64(binvala % binvalb, 64, bitwidth);
  }


  //Bin Op Arith on Bool (e.g. op equals)
  //convert to binary before the operation; then convert back to bool
  inline u32 _BinOpAddBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 binvala = _Bool32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Bool32ToInt32(valb, bitwidth, 32);
    return _Int32ToBool32(binvala + binvalb, 32, bitwidth);
  }

  inline u64 _BinOpAddBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 binvala = _Bool64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Bool64ToInt64(valb, bitwidth, 64);
    return _Int64ToBool64(binvala + binvalb, 64, bitwidth);
  }

  inline u32 _BinOpSubtractBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 binvala = _Bool32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Bool32ToInt32(valb, bitwidth, 32);
    return _Int32ToBool32(binvala - binvalb, 32, bitwidth);
  }

  inline u64 _BinOpSubtractBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 binvala = _Bool64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Bool64ToInt64(valb, bitwidth, 64);
    return _Int64ToBool64(binvala - binvalb, 64, bitwidth);
  }

  inline u32 _BinOpMultiplyBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    s32 binvala = _Bool32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Bool32ToInt32(valb, bitwidth, 32);
    return _Int32ToBool32(binvala * binvalb, 32, bitwidth);
  }

  inline u64 _BinOpMultiplyBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    s64 binvala = _Bool64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Bool64ToInt64(valb, bitwidth, 64);
    return _Int64ToBool64(binvala * binvalb, 64, bitwidth);
  }

  inline u32 _BinOpDivideBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s32 binvala = _Bool32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Bool32ToInt32(valb, bitwidth, 32);
    return _Int32ToBool32(binvala / binvalb, 32, bitwidth);
  }

  inline u64 _BinOpDivideBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s64 binvala = _Bool64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Bool64ToInt64(valb, bitwidth, 64);
    return _Int64ToBool64(binvala / binvalb, 64, bitwidth);
  }

  inline u32 _BinOpModBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s32 binvala = _Bool32ToInt32(vala, bitwidth, 32);
    s32 binvalb = _Bool32ToInt32(valb, bitwidth, 32);
    return _Int32ToBool32(binvala % binvalb, 32, bitwidth);
  }

  inline u64 _BinOpModBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    MFM_API_ASSERT_NONZERO(valb);
    s64 binvala = _Bool64ToInt64(vala, bitwidth, 64);
    s64 binvalb = _Bool64ToInt64(valb, bitwidth, 64);
    return _Int64ToBool64(binvala % binvalb, 64, bitwidth);
  }

  //COMPARISONS

  //CompOps on INTS:
  inline u32 _BinOpCompareEqEqInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _CboolToBool32(extvala == extvalb, BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareEqEqInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _CboolToBool64(extvala == extvalb, BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareNotEqInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _CboolToBool32(extvala != extvalb, BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareNotEqInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _CboolToBool64(extvala != extvalb, BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessThanInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _CboolToBool32(extvala < extvalb, BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessThanInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _CboolToBool64(extvala < extvalb, BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterThanInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _CboolToBool32(extvala > extvalb, BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterThanInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _CboolToBool64(extvala > extvalb, BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessEqualInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _CboolToBool32(extvala <= extvalb, BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessEqualInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _CboolToBool64(extvala <= extvalb, BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterEqualInt32(s32 vala, s32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    s32 extvala = _SignExtend32(vala & mask, bitwidth);
    s32	extvalb = _SignExtend32(valb & mask, bitwidth);
    return _CboolToBool32(extvala >= extvalb, BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterEqualInt64(s64 vala, s64 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes64(bitwidth);
    s64 extvala = _SignExtend64(vala & mask, bitwidth);
    s64	extvalb = _SignExtend64(valb & mask, bitwidth);
    return _CboolToBool64(extvala >= extvalb, BITS_PER_BOOL);
  }

  //CompOps on UNSIGNED
  inline u32 _BinOpCompareEqEqUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return _CboolToBool32(((vala & mask) == (valb & mask)), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareEqEqUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return _CboolToBool64(((vala & mask) == (valb & mask)), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareNotEqUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return _CboolToBool32(((vala & mask) != (valb & mask)), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareNotEqUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return _CboolToBool64(((vala & mask) != (valb & mask)), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessThanUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return _CboolToBool32(((vala & mask) < (valb & mask)), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessThanUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return _CboolToBool64(((vala & mask) < (valb & mask)), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterThanUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return _CboolToBool32(((vala & mask) > (valb & mask)), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterThanUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return _CboolToBool64(((vala & mask) > (valb & mask)), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessEqualUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return _CboolToBool32(((vala & mask) <= (valb & mask)), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessEqualUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return _CboolToBool64(((vala & mask) <= (valb & mask)), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterEqualUnsigned32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return _CboolToBool32(((vala & mask) >= (valb & mask)), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterEqualUnsigned64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return _CboolToBool64(((vala & mask) >= (valb & mask)), BITS_PER_BOOL);
  }

  //CompOps on BOOL
  inline u32 _BinOpCompareEqEqBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToUnsigned32(vala, bitwidth, 32);
    u32 binvalb = _Bool32ToUnsigned32(valb, bitwidth, 32);
    return _CboolToBool32((binvala == binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareEqEqBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToUnsigned64(vala, bitwidth, 32);
    u64 binvalb = _Bool64ToUnsigned64(valb, bitwidth, 32);
    return _CboolToBool64((binvala == binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareNotEqBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToUnsigned32(vala, bitwidth, 32);
    u32 binvalb = _Bool32ToUnsigned32(valb, bitwidth, 32);
    return _CboolToBool32((binvala != binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareNotEqBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToUnsigned64(vala, bitwidth, 32);
    u64 binvalb = _Bool64ToUnsigned64(valb, bitwidth, 32);
    return _CboolToBool64((binvala != binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessThanBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToUnsigned32(vala, bitwidth, 32);
    u32 binvalb = _Bool32ToUnsigned32(valb, bitwidth, 32);
    return _CboolToBool32((binvala < binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessThanBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToUnsigned64(vala, bitwidth, 32);
    u64 binvalb = _Bool64ToUnsigned64(valb, bitwidth, 32);
    return _CboolToBool64((binvala < binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterThanBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToUnsigned32(vala, bitwidth, 32);
    u32 binvalb = _Bool32ToUnsigned32(valb, bitwidth, 32);
    return _CboolToBool32((binvala > binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterThanBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToUnsigned64(vala, bitwidth, 32);
    u64 binvalb = _Bool64ToUnsigned64(valb, bitwidth, 32);
    return _CboolToBool64((binvala > binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessEqualBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToUnsigned32(vala, bitwidth, 32);
    u32 binvalb = _Bool32ToUnsigned32(valb, bitwidth, 32);
    return _CboolToBool32((binvala <= binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessEqualBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToUnsigned64(vala, bitwidth, 32);
    u64 binvalb = _Bool64ToUnsigned64(valb, bitwidth, 32);
    return _CboolToBool64((binvala <= binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterEqualBool32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Bool32ToUnsigned32(vala, bitwidth, 32);
    u32 binvalb = _Bool32ToUnsigned32(valb, bitwidth, 32);
    return _CboolToBool32((binvala >= binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterEqualBool64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Bool64ToUnsigned64(vala, bitwidth, 32);
    u64 binvalb = _Bool64ToUnsigned64(valb, bitwidth, 32);
    return _CboolToBool64((binvala >= binvalb), BITS_PER_BOOL);
  }

  //CompOps on UNARY
  inline u32 _BinOpCompareEqEqUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Unary32ToUnsigned32(vala, bitwidth, 32);
    u32 binvalb = _Unary32ToUnsigned32(valb, bitwidth, 32);
    return _CboolToBool32((binvala == binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareEqEqUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Unary64ToUnsigned64(vala, bitwidth, 32);
    u64 binvalb = _Unary64ToUnsigned64(valb, bitwidth, 32);
    return _CboolToBool64((binvala == binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareNotEqUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Unary32ToUnsigned32(vala, bitwidth, 32);
    u32 binvalb = _Unary32ToUnsigned32(valb, bitwidth, 32);
    return _CboolToBool32((binvala != binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareNotEqUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Unary64ToUnsigned64(vala, bitwidth, 32);
    u64 binvalb = _Unary64ToUnsigned64(valb, bitwidth, 32);
    return _CboolToBool64((binvala != binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessThanUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Unary32ToUnsigned32(vala, bitwidth, 32);
    u32 binvalb = _Unary32ToUnsigned32(valb, bitwidth, 32);
    return _CboolToBool32((binvala < binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessThanUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Unary64ToUnsigned64(vala, bitwidth, 32);
    u64 binvalb = _Unary64ToUnsigned64(valb, bitwidth, 32);
    return _CboolToBool64((binvala < binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterThanUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Unary32ToUnsigned32(vala, bitwidth, 32);
    u32 binvalb = _Unary32ToUnsigned32(valb, bitwidth, 32);
    return _CboolToBool32((binvala > binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterThanUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Unary64ToUnsigned64(vala, bitwidth, 32);
    u64 binvalb = _Unary64ToUnsigned64(valb, bitwidth, 32);
    return _CboolToBool64((binvala > binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareLessEqualUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Unary32ToUnsigned32(vala, bitwidth, 32);
    u32 binvalb = _Unary32ToUnsigned32(valb, bitwidth, 32);
    return _CboolToBool32((binvala <= binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareLessEqualUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Unary64ToUnsigned64(vala, bitwidth, 32);
    u64 binvalb = _Unary64ToUnsigned64(valb, bitwidth, 32);
    return _CboolToBool64((binvala <= binvalb), BITS_PER_BOOL);
  }

  inline u32 _BinOpCompareGreaterEqualUnary32(u32 vala, u32 valb, u32 bitwidth)
  {
    u32 binvala = _Unary32ToUnsigned32(vala, bitwidth, 32);
    u32 binvalb = _Unary32ToUnsigned32(valb, bitwidth, 32);
    return _CboolToBool32((binvala >= binvalb), BITS_PER_BOOL);
  }

  inline u64 _BinOpCompareGreaterEqualUnary64(u64 vala, u64 valb, u32 bitwidth)
  {
    u64 binvala = _Unary64ToUnsigned64(vala, bitwidth, 32);
    u64 binvalb = _Unary64ToUnsigned64(valb, bitwidth, 32);
    return _CboolToBool64((binvala >= binvalb), BITS_PER_BOOL);
  }

  //SHIFT OPS
  //Shift INTS:
  inline s32 _ShiftOpRightInt32(s32 vala, u32 shft, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ((vala >> shft) & mask);   //sign extension???
  }

  inline s64 _ShiftOpRightInt64(s64 vala, u32 shft, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ((vala >> shft) & mask);   //sign extension???
  }

  inline s32 _ShiftOpLeftInt32(s32 vala, u32 shft, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ((vala << shft) & mask);
  }

  inline s64 _ShiftOpLeftInt64(s64 vala, u32 shft, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ((vala << shft) & mask);
  }

  //Shift UNSIGNED:
  inline u32 _ShiftOpRightUnsigned32(u32 vala, u32 shft, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ((vala >> shft) & mask);
  }

  inline u64 _ShiftOpRightUnsigned64(u64 vala, u32 shft, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ((vala >> shft) & mask);
  }

  inline u32 _ShiftOpLeftUnsigned32(u32 vala, u32 shft, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ((vala << shft) & mask);
  }

  inline u64 _ShiftOpLeftUnsigned64(u64 vala, u32 shft, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ((vala << shft) & mask);
  }

  //Shift BOOL:
  inline u32 _ShiftOpRightBool32(u32 vala, u32 shft, u32 bitwidth)
  {
    u32 binvala = _Bool32ToBits32(vala, bitwidth, bitwidth);
    return _Bits32ToBool32((binvala >> shft), bitwidth, bitwidth);
  }

  inline u64 _ShiftOpRightBool64(u64 vala, u32 shft, u32 bitwidth)
  {
    u32 binvala = _Bool64ToBits64(vala, bitwidth, bitwidth);
    return _Bits64ToBool64((binvala >> shft), bitwidth, bitwidth);
  }

  inline u32 _ShiftOpLeftBool32(u32 vala, u32 shft, u32 bitwidth)
  {
    u32 binvala = _Bool32ToBits32(vala, bitwidth, bitwidth);
    return _Bits32ToBool32((binvala << shft), bitwidth, bitwidth);
  }

  inline u64 _ShiftOpLeftBool64(u64 vala, u32 shft, u32 bitwidth)
  {
    u32 binvala = _Bool64ToBits64(vala, bitwidth, bitwidth);
    return _Bits64ToBool64((binvala >> shft), bitwidth, bitwidth);
  }

  //Shift UNARY:
  inline u32 _ShiftOpRightUnary32(u32 vala, u32 shft, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    u32 binvala = PopCount(vala & mask);     //in binary
    return _GetNOnes32(binvala >> shft) & mask;
  }

  inline u64 _ShiftOpRightUnary64(u64 vala, u32 shft, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    u32 binvala = PopCount(vala & mask);     //in binary
    return _GetNOnes64(binvala >> shft) & mask;
  }

  inline u32 _ShiftOpLeftUnary32(u32 vala, u32 shft, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    u32 binvala = PopCount(vala & mask);     //in binary
    return _GetNOnes32(binvala << shft) & mask;
  }

  inline u64 _ShiftOpLeftUnary64(u64 vala, u32 shft, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    u32 binvala = PopCount(vala & mask);     //in binary
    return _GetNOnes64(binvala << shft) & mask;
  }

  //Shift BITS:
  inline u32 _ShiftOpRightBits32(u32 vala, u32 shft, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ((vala >> shft) & mask);
  }

  inline u64 _ShiftOpRightBits64(u64 vala, u32 shft, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ((vala >> shft) & mask);
  }

  inline u32 _ShiftOpLeftBits32(u32 vala, u32 shft, u32 bitwidth)
  {
    u32 mask = _GetNOnes32(bitwidth);
    return ((vala << shft) & mask);
  }

  inline u64 _ShiftOpLeftBits64(u64 vala, u32 shft, u32 bitwidth)
  {
    u64 mask = _GetNOnes64(bitwidth);
    return ((vala << shft) & mask);
  }

}

#endif /* CASTOPS_H */
