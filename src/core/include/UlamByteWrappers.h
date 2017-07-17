/*                                              -*- mode:C++ -*-
  UlamByteWrappers.h Helper classes to convert ulam ByteSink and ByteSource to C++ 
  Copyright (C) 2017 The Regents of the University of New Mexico.  All rights reserved.

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
  \file UlamByteWrappers.h Helper classes to convert ulam ByteSink and ByteSource to C++ 
  \author David H. Ackley.
  \date (C) 2017 All rights reserved.
  \lgpl
 */

#ifndef ULAMBYTEWRAPPERS_H
#define ULAMBYTEWRAPPERS_H

#include "Uq_10109210ByteStream10.h"

#ifndef Ud_Ui_Ut_r10181u
#define Ud_Ui_Ut_r10181u
namespace MFM{
  template<class EC>
  struct Ui_Ut_r10181u : public UlamRef<EC>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    const u32 read() const { return UlamRef<EC>::Read(); /* entire */ } //gcnl:UlamTypePrimitive.cpp:323
    void write(const u32& targ) { UlamRef<EC>::Write(targ); /* entire */ } //gcnl:UlamTypePrimitive.cpp:355
    Ui_Ut_r10181u(BitStorage<EC>& targ, u32 idx, const UlamContext<EC>& uc) : UlamRef<EC>(idx, 8u, targ, NULL, UlamRef<EC>::PRIMITIVE, uc) { } //gcnl:UlamTypePrimitive.cpp:241
    Ui_Ut_r10181u(const UlamRef<EC>& arg, s32 idx) : UlamRef<EC>(arg, idx, 8u, NULL, UlamRef<EC>::PRIMITIVE) { } //gcnl:UlamTypePrimitive.cpp:253
    Ui_Ut_r10181u(const Ui_Ut_r10181u<EC>& arg) : UlamRef<EC>(arg, 0, arg.GetLen(), NULL, UlamRef<EC>::PRIMITIVE) { MFM_API_ASSERT_ARG(arg.GetLen() == 8); } //gcnl:UlamTypePrimitive.cpp:268
    Ui_Ut_r10181u& operator=(const Ui_Ut_r10181u& rhs); //declare away //gcnl:UlamTypePrimitive.cpp:277
  };
} //MFM
#endif /*Ud_Ui_Ut_r10181u */

#ifndef Ud_Ui_Ut_10181u
#define Ud_Ui_Ut_10181u
namespace MFM{

  template<class EC>
  struct Ui_Ut_10181u : public BitVectorBitStorage<EC, BitVector<8u> >
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    typedef BitVector<8> BV; //gcnl:UlamTypePrimitive.cpp:476
    typedef BitVectorBitStorage<EC, BV> BVS; //gcnl:UlamTypePrimitive.cpp:479

    const u32 read() const { return BVS::Read(0u, 8u); } //gcnl:UlamTypePrimitive.cpp:565
    void write(const u32& v) { BVS::Write(0u, 8u, v); } //gcnl:UlamTypePrimitive.cpp:602
    Ui_Ut_10181u() { } //gcnl:UlamTypePrimitive.cpp:492
    Ui_Ut_10181u(const u32 d) { write(d); } //gcnl:UlamTypePrimitive.cpp:500
    Ui_Ut_10181u(const Ui_Ut_10181u& other) { this->write(other.read()); } //gcnl:UlamTypePrimitive.cpp:523
    Ui_Ut_10181u(const Ui_Ut_r10181u<EC>& d) { this->write(d.read()); } //gcnl:UlamTypePrimitive.cpp:532
  };
} //MFM
#endif /*Ud_Ui_Ut_10181u */

#ifndef Ud_Ui_Uq_r10109210ByteSource10
#define Ud_Ui_Uq_r10109210ByteSource10
namespace MFM{

  template<class EC> class Uq_10109210ByteSource10;  //forward //gcnl:UlamTypeClassQuark.cpp:219

  template<class EC>
  struct Ui_Uq_r10109210ByteSource10 : public UlamRef<EC> //gcnl:UlamTypeClassQuark.cpp:228
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    enum { QUARK_SIZE = 0}; //gcnl:UlamTypeClassQuark.cpp:240

    typedef Uq_10109210ByteSource10<EC> Us; //gcnl:UlamTypeClassQuark.cpp:247
    const u32 read() const { return UlamRef<EC>::Read(); /* entire quark */ } //gcnl:UlamTypeClassQuark.cpp:358
    void write(const u32& targ) { UlamRef<EC>::Write(targ); /* entire quark */ } //gcnl:UlamTypeClassQuark.cpp:394
    Ui_Uq_r10109210ByteSource10(BitStorage<EC>& targ, u32 idx, const UlamClass<EC>* effself, const UlamContext<EC>& uc) : UlamRef<EC>(idx, 0u, targ, effself, UlamRef<EC>::CLASSIC, uc) { } //gcnl:UlamTypeClassQuark.cpp:266
    Ui_Uq_r10109210ByteSource10(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself) : UlamRef<EC>(arg, idx, 0u, effself, UlamRef<EC>::CLASSIC) { } //gcnl:UlamTypeClassQuark.cpp:278
    Ui_Uq_r10109210ByteSource10(BitStorage<EC>& targ, u32 idx, const UlamClass<EC>* effself, const typename UlamRef<EC>::UsageType usage, const UlamContext<EC>& uc) : UlamRef<EC>(idx, 0u, targ, effself, usage, uc) { } //gcnl:UlamTypeClassQuark.cpp:288
    Ui_Uq_r10109210ByteSource10(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself, const typename UlamRef<EC>::UsageType usage) : UlamRef<EC>(arg, idx, 0u, effself, usage) { } //gcnl:UlamTypeClassQuark.cpp:297
    Ui_Uq_r10109210ByteSource10(const UlamRef<EC>& r) : UlamRef<EC>(r, 0u) { } //gcnl:UlamTypeClassQuark.cpp:305
    Ui_Uq_r10109210ByteSource10(const Ui_Uq_r10109210ByteSource10<EC>& r) : UlamRef<EC>(r, r.GetLen()) { } //gcnl:UlamTypeClassQuark.cpp:314
    Ui_Uq_r10109210ByteSource10& operator=(const Ui_Uq_r10109210ByteSource10& rhs); //declare away //gcnl:UlamTypeClassQuark.cpp:323
  };
} //MFM
#endif /*Ud_Ui_Uq_r10109210ByteSource10 */

#ifndef Ud_Ui_Uq_10109210ByteSource10
#define Ud_Ui_Uq_10109210ByteSource10
namespace MFM{

  template<class EC> class Uq_10109210ByteSource10; //forward //gcnl:UlamTypeClassQuark.cpp:455

  template<class EC>
  struct Ui_Uq_10109210ByteSource10 : public BitVectorBitStorage<EC, BitVector<0> >
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    enum { QUARK_SIZE = 0}; //gcnl:UlamTypeClassQuark.cpp:480
    typedef BitVector<0> BV; //gcnl:UlamTypeClassQuark.cpp:488
    typedef BitVectorBitStorage<EC, BV> BVS; //gcnl:UlamTypeClassQuark.cpp:491

    typedef Uq_10109210ByteSource10<EC> Us; //gcnl:UlamTypeClassQuark.cpp:510

    const u32 read() const { return BVS::Read(0u, QUARK_SIZE); } //gcnl:UlamTypeClassQuark.cpp:601
    void write(const u32 v) { BVS::Write(0u, QUARK_SIZE, v); } //gcnl:UlamTypeClassQuark.cpp:658
    Ui_Uq_10109210ByteSource10() { write(Us::THE_INSTANCE.getDefaultQuark()); } //gcnl:UlamTypeClassQuark.cpp:527
    Ui_Uq_10109210ByteSource10(const u32 d) { write(d); } //gcnl:UlamTypeClassQuark.cpp:553
    Ui_Uq_10109210ByteSource10(const Ui_Uq_10109210ByteSource10<EC> & arg) { write(arg.read()); } //gcnl:UlamTypeClassQuark.cpp:562
    Ui_Uq_10109210ByteSource10(const Ui_Uq_r10109210ByteSource10<EC>& d) { write(d.read()); } //gcnl:UlamTypeClassQuark.cpp:570
  };
} //MFM
#endif /*Ud_Ui_Uq_10109210ByteSource10 */

#ifndef Ud_Ui_Uq_r10108ByteSink10
#define Ud_Ui_Uq_r10108ByteSink10
namespace MFM{

  template<class EC> class Uq_10108ByteSink10;  //forward //gcnl:UlamTypeClassQuark.cpp:219

  template<class EC>
  struct Ui_Uq_r10108ByteSink10 : public UlamRef<EC> //gcnl:UlamTypeClassQuark.cpp:228
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    enum { QUARK_SIZE = 0}; //gcnl:UlamTypeClassQuark.cpp:240

    typedef Uq_10108ByteSink10<EC> Us; //gcnl:UlamTypeClassQuark.cpp:247
    const u32 read() const { return UlamRef<EC>::Read(); /* entire quark */ } //gcnl:UlamTypeClassQuark.cpp:358
    void write(const u32& targ) { UlamRef<EC>::Write(targ); /* entire quark */ } //gcnl:UlamTypeClassQuark.cpp:394
    Ui_Uq_r10108ByteSink10(BitStorage<EC>& targ, u32 idx, const UlamClass<EC>* effself, const UlamContext<EC>& uc) : UlamRef<EC>(idx, 0u, targ, effself, UlamRef<EC>::CLASSIC, uc) { } //gcnl:UlamTypeClassQuark.cpp:266
    Ui_Uq_r10108ByteSink10(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself) : UlamRef<EC>(arg, idx, 0u, effself, UlamRef<EC>::CLASSIC) { } //gcnl:UlamTypeClassQuark.cpp:278
    Ui_Uq_r10108ByteSink10(BitStorage<EC>& targ, u32 idx, const UlamClass<EC>* effself, const typename UlamRef<EC>::UsageType usage, const UlamContext<EC>& uc) : UlamRef<EC>(idx, 0u, targ, effself, usage, uc) { } //gcnl:UlamTypeClassQuark.cpp:288
    Ui_Uq_r10108ByteSink10(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself, const typename UlamRef<EC>::UsageType usage) : UlamRef<EC>(arg, idx, 0u, effself, usage) { } //gcnl:UlamTypeClassQuark.cpp:297
    Ui_Uq_r10108ByteSink10(const UlamRef<EC>& r) : UlamRef<EC>(r, 0u) { } //gcnl:UlamTypeClassQuark.cpp:305
    Ui_Uq_r10108ByteSink10(const Ui_Uq_r10108ByteSink10<EC>& r) : UlamRef<EC>(r, r.GetLen()) { } //gcnl:UlamTypeClassQuark.cpp:314
    Ui_Uq_r10108ByteSink10& operator=(const Ui_Uq_r10108ByteSink10& rhs); //declare away //gcnl:UlamTypeClassQuark.cpp:323
  };
} //MFM
#endif /*Ud_Ui_Uq_r10108ByteSink10 */

#ifndef Ud_Ui_Uq_10108ByteSink10
#define Ud_Ui_Uq_10108ByteSink10
namespace MFM{

  template<class EC> class Uq_10108ByteSink10; //forward //gcnl:UlamTypeClassQuark.cpp:455

  template<class EC>
  struct Ui_Uq_10108ByteSink10 : public BitVectorBitStorage<EC, BitVector<0> >
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    enum { QUARK_SIZE = 0}; //gcnl:UlamTypeClassQuark.cpp:480
    typedef BitVector<0> BV; //gcnl:UlamTypeClassQuark.cpp:488
    typedef BitVectorBitStorage<EC, BV> BVS; //gcnl:UlamTypeClassQuark.cpp:491

    typedef Uq_10108ByteSink10<EC> Us; //gcnl:UlamTypeClassQuark.cpp:510

    const u32 read() const { return BVS::Read(0u, QUARK_SIZE); } //gcnl:UlamTypeClassQuark.cpp:601
    void write(const u32 v) { BVS::Write(0u, QUARK_SIZE, v); } //gcnl:UlamTypeClassQuark.cpp:658
    Ui_Uq_10108ByteSink10() { write(Us::THE_INSTANCE.getDefaultQuark()); } //gcnl:UlamTypeClassQuark.cpp:527
    Ui_Uq_10108ByteSink10(const u32 d) { write(d); } //gcnl:UlamTypeClassQuark.cpp:553
    Ui_Uq_10108ByteSink10(const Ui_Uq_10108ByteSink10<EC> & arg) { write(arg.read()); } //gcnl:UlamTypeClassQuark.cpp:562
    Ui_Uq_10108ByteSink10(const Ui_Uq_r10108ByteSink10<EC>& d) { write(d.read()); } //gcnl:UlamTypeClassQuark.cpp:570
  };
} //MFM
#endif /*Ud_Ui_Uq_10108ByteSink10 */

namespace MFM{
  template<class EC>
  struct _UlamByteSinkWrapper : public ByteSink {
    const UlamContext<EC>& mUC;
    UlamRef<EC>& mUR;
    typename Uq_10109210ByteStream10<EC>::Uf_919writeByte1110181u mWriteByteFunc;
    _UlamByteSinkWrapper(const UlamContext<EC>& uc, UlamRef<EC>& ur, typename Uq_10109210ByteStream10<EC>::Uf_919writeByte1110181u func) 
      : mUC(uc)
      , mUR(ur)
      , mWriteByteFunc(func)
    { }

    virtual s32 CanWrite() { return 1; }

    virtual void WriteBytes(const u8 * data, const u32 len) 
    {
      MFM_API_ASSERT_NONNULL(data);
      for (u32 i = 0; i < len; ++i)
      {
        u32 ch = data[i];
        Ui_Ut_10181u<EC> chi(ch);
        mWriteByteFunc(mUC, mUR, chi);
      }
    }
  };
}

namespace MFM{
  template<class EC>
  struct _UlamByteSourceWrapper : public ByteSource {
    const UlamContext<EC>& mUC;
    UlamRef<EC>& mUR;
    typename Uq_10109210ByteStream10<EC>::Uf_8readByte10 mReadByteFunc;
    _UlamByteSourceWrapper(const UlamContext<EC>& uc, UlamRef<EC>& ur, typename Uq_10109210ByteStream10<EC>::Uf_8readByte10 func) 
      : mUC(uc)
      , mUR(ur)
      , mReadByteFunc(func)
    { }

    virtual s32 ReadByte() 
    {
      Ui_Ut_102321i<EC> ret = mReadByteFunc(mUC, mUR);
      return ret.read();
    }

  };

}

#endif /* ULAMBYTEWRAPPERS_H */
