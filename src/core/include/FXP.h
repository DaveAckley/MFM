/* -*- C++ -*- */
/**
 * FXP.h: Simple fixed-point code.
 * Copyright (C) 2014 Regents of the University of New Mexico
 *
 * Rewritten for the MFM library by Dave Ackley, but originally based
 * on 'fixed_class.h', which is licensed as follows:

 Copyright (c) 2007, Markus Trenkwalder

 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 * Neither the name of the library's copyright owner nor the names of its
 contributors may be used to endorse or promote products derived from this
 software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _FXP_H
#define _FXP_H

#ifndef FXP_STDIO
#define FXP_STDIO 1
#endif

#if FXP_STDIO
#include <stdio.h>    /* For FILE */
#endif

#include "itype.h"

namespace MFM {

  // The template argument p in all of the following functions refers to the
  // fixed point precision (e.g. p = 8 gives 24.8 fixed point functions).

  // Perform a fixed point multiplication without a 64-bit intermediate result.
  // This is fast but beware of overflow!
  template <int p>
  inline s32 fixmulf(s32 a, s32 b)
  {
    return (a * b) >> p;
  }

  // Perform a fixed point multiplication using a 64-bit intermediate result to
  // prevent overflow problems.
  template <int p>
  inline s32 fixmul(s32 a, s32 b)
  {
    return (s32)(((int64_t)a * b) >> p);
  }

  // Fixed point division
  template <int p>
  inline int fixdiv(s32 a, s32 b)
  {
#if 1
    return (s32)((((s64)a) << p) / b);
#else
    // The following produces the same results as the above but gcc 4.0.3
    // generates fewer instructions (at least on the ARM processor).
    union {
      s64 a;
      struct {
        s32 l;
        s32 h;
      } halves;
    } x;

    x.halves.l = a << p;
    x.halves.h = a >> (sizeof(s32) * 8 - p);
    return (s32)(x.a / b);
#endif
  }

  namespace detail {
    inline u32 CountLeadingZeros(u32 x)
    {
      u32 exp = 31;

      if (x & 0xffff0000) {
        exp -= 16;
        x >>= 16;
      }

      if (x & 0xff00) {
        exp -= 8;
        x >>= 8;
      }

      if (x & 0xf0) {
        exp -= 4;
        x >>= 4;
      }

      if (x & 0xc) {
        exp -= 2;
        x >>= 2;
      }

      if (x & 0x2) {
        exp -= 1;
      }

      return exp;
    }
  }

  // q is the precision of the input
  // output has 32-q bits of fraction
  template <int q>
  inline int fixinv(s32 a)
  {
    s32 x;

    bool sign = false;

    if (a < 0) {
      sign = true;
      a = -a;
    }

    static const uint16_t rcp_tab[] = {
      0x8000, 0x71c7, 0x6666, 0x5d17, 0x5555, 0x4ec4, 0x4924, 0x4444
    };

    s32 exp = detail::CountLeadingZeros(a);
    x = ((s32)rcp_tab[(a>>(28-exp))&0x7]) << 2;
    exp -= 16;

    if (exp <= 0)
      x >>= -exp;
    else
      x <<= exp;

    /* two iterations of newton-raphson  x = x(2-ax) */
    x = fixmul<(32-q)>(x,((2<<(32-q)) - fixmul<q>(a,x)));
    x = fixmul<(32-q)>(x,((2<<(32-q)) - fixmul<q>(a,x)));

    if (sign)
      return -x;
    else
      return x;
  }

  // Conversion from and to float

  template <int p>
  float fix2float(s32 f)
  {
    return (float)f / (1 << p);
  }

  template <int p>
  s32 float2fix(float f)
  {
    return (s32)(f * (1 << p));
  }

  s32 fixcos16(s32 a);
  s32 fixsin16(s32 a);
  s32 fixrsqrt16(s32 a);
  s32 fixsqrt16(s32 a);

  // The template argument p in all of the following functions refers to the
  // fixed point precision (e.g. p = 8 gives 24.8 fixed point functions).

  template <int p>
  struct FXP {
    s32 intValue;

    FXP() : intValue(0) {}
    /*explicit*/ FXP(u32 i) : intValue(i << p) {}
    /*explicit*/ FXP(s32 i) : intValue(i << p) {}
    /*explicit*/ FXP(float f) : intValue(float2fix<p>(f)) {}
    /*explicit*/ FXP(double f) : intValue(float2fix<p>((float)f)) {}

    FXP& operator += (FXP r) { intValue += r.intValue; return *this; }
    FXP& operator -= (FXP r) { intValue -= r.intValue; return *this; }
    FXP& operator *= (FXP r) { intValue = fixmul<p>(intValue, r.intValue); return *this; }
    FXP& operator /= (FXP r) { intValue = fixdiv<p>(intValue, r.intValue); return *this; }

    FXP& operator ++ () { *this += 1; return *this; }
    FXP operator ++ (int) { const FXP val = *this; ++ *this; return val; }

    FXP& operator -- () { *this -= 1; return *this; }
    FXP operator -- (int) { const FXP val = *this; -- *this; return val; }

    FXP& operator *= (s32 r) { intValue *= r; return *this; }
    FXP& operator /= (s32 r) { intValue /= r; return *this; }

    FXP operator - () const { FXP x; x.intValue = -intValue; return x; }
    FXP operator + (FXP r) const { FXP x = *this; x += r; return x;}
    FXP operator - (FXP r) const { FXP x = *this; x -= r; return x;}
    FXP operator * (FXP r) const { FXP x = *this; x *= r; return x;}
    FXP operator / (FXP r) const { FXP x = *this; x /= r; return x;}

    bool operator == (FXP r) const { return intValue == r.intValue; }
    bool operator != (FXP r) const { return !(*this == r); }
    bool operator <  (FXP r) const { return intValue < r.intValue; }
    bool operator >  (FXP r) const { return intValue > r.intValue; }
    bool operator <= (FXP r) const { return intValue <= r.intValue; }
    bool operator >= (FXP r) const { return intValue >= r.intValue; }

    FXP operator + (s32 r) const { FXP x = *this; x += r; return x;}
    FXP operator - (s32 r) const { FXP x = *this; x -= r; return x;}
    FXP operator * (s32 r) const { FXP x = *this; x *= r; return x;}
    FXP operator / (s32 r) const { FXP x = *this; x /= r; return x;}

    s32 asInt() const { return intValue; }
    float toFloat() const { return fix2float<p>(intValue); }
    double toDouble() const { return (double) fix2float<p>(intValue); }

#if FXP_STDIO
    void Print(FILE * f) {
      u32 val;
      if (intValue < 0) {
        val = -intValue;
        fputc('-',f);
      } else val = intValue;
      fprintf(f,"%d.", val >> p);
      for (int i = 0; i < "011122233344445556667777888999:::"[p]-'0'; ++i) {
        val &= (1<<p)-1;
        val *= 10;
        fputc((val>>p)+'0',f);
      }
    }
#endif

  };

  typedef FXP<16> FXP16;

  // Specializations for use with plain integers
  template <int p>
  inline FXP<p> operator + (s32 a, FXP<p> b)
  { return b + a; }

  template <int p>
  inline FXP<p> operator - (s32 a, FXP<p> b)
  { return -b + a; }

  template <int p>
  inline FXP<p> operator * (s32 a, FXP<p> b)
  { return b * a; }

  template <int p>
  inline FXP<p> operator / (s32 a, FXP<p> b)
  { FXP<p> r(a); r /= b; return r; }

  // math functions
  // no default implementation

  template <int p>
  inline FXP<p> Sin(FXP<p> a);

  template <int p>
  inline FXP<p> Cos(FXP<p> a);

  template <int p>
  inline FXP<p> Sqrt(FXP<p> a);

  template <int p>
  inline FXP<p> Rsqrt(FXP<p> a);

  template <int p>
  inline FXP<p> Inv(FXP<p> a);

  template <int p>
  inline FXP<p> Abs(FXP<p> a)
  {
    FXP<p> r;
    r.intValue = a.intValue > 0 ? a.intValue : -a.intValue;
    return r;
  }

  // specializations for 16.16 format

  template <>
  inline FXP<16> Sin(FXP<16> a)
  {
    FXP<16> r;
    r.intValue = fixsin16(a.intValue);
    return r;
  }

  template <>
  inline FXP<16> Cos(FXP<16> a)
  {
    FXP<16> r;
    r.intValue = fixcos16(a.intValue);
    return r;
  }


  template <>
  inline FXP<16> Sqrt(FXP<16> a)
  {
    FXP<16> r;
    r.intValue = fixsqrt16(a.intValue);
    return r;
  }

  template <>
  inline FXP<16> Rsqrt(FXP<16> a)
  {
    FXP<16> r;
    r.intValue = fixrsqrt16(a.intValue);
    return r;
  }

  template <>
  inline FXP<16> Inv(FXP<16> a)
  {
    FXP<16> r;
    r.intValue = fixinv<16>(a.intValue);
    return r;
  }

  // The multiply accumulate case can be optimized.
  template <int p>
  inline FXP<p> multiply_accumulate(
                                    int count,
                                    const FXP<p> *a,
                                    const FXP<p> *b)
  {
    s64 result = 0;
    for (int i = 0; i < count; ++i)
      result += static_cast<s64>(a[i].intValue) * b[i].intValue;
    FXP<p> r;
    r.intValue = static_cast<s32>(result >> p);
    return r;
  }


} // end namespace MFM

#endif /* _FXP_H */

