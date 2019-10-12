/*                                              -*- mode:C++ -*-
  UlamByteWrappers.h Helper classes to convert ulam ByteSink and ByteSource to C++
  Copyright (C) 2017-2019 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2019 Ackleyshack LLC.

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
  \author Elena S. Ackley.
  \date (C) 2017-2019 All rights reserved.
  \lgpl
 */

#ifndef ULAMBYTEWRAPPERS_H
#define ULAMBYTEWRAPPERS_H

namespace MFM { template <class EC> struct Uq_10109216ByteStreamWriter10; }  //FORWARD
namespace MFM { template <class EC> struct Uq_10109216ByteStreamReader10; }  //FORWARD

#ifndef Ud_Ui_Ut_r10181u
#define Ud_Ui_Ut_r10181u
namespace MFM{
  template<class EC>
  struct Ui_Ut_r10181u : public UlamRef<EC>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    const u32 read() const { return UlamRef<EC>::Read(); /* entire */ } //gcnl:UlamTypePrimitive.cpp:311
    void write(const u32& targ) { UlamRef<EC>::Write(targ); /* entire */ } //gcnl:UlamTypePrimitive.cpp:343
    Ui_Ut_r10181u(BitStorage<EC>& targ, u32 idx, const UlamContext<EC>& uc) : UlamRef<EC>(idx, 8u, targ, NULL, UlamRef<EC>::PRIMITIVE, uc) { } //gcnl:UlamTypePrimitive.cpp:229
    Ui_Ut_r10181u(const UlamRef<EC>& arg, s32 idx) : UlamRef<EC>(arg, idx, 8u, NULL, UlamRef<EC>::PRIMITIVE) { } //gcnl:UlamTypePrimitive.cpp:241
    Ui_Ut_r10181u(const Ui_Ut_r10181u<EC>& arg) : UlamRef<EC>(arg, 0, arg.GetLen(), NULL, UlamRef<EC>::PRIMITIVE) { MFM_API_ASSERT_ARG(arg.GetLen() == 8); } //gcnl:UlamTypePrimitive.cpp:256
    Ui_Ut_r10181u& operator=(const Ui_Ut_r10181u& rhs); //declare away //gcnl:UlamTypePrimitive.cpp:265
  };
} //MFM
#endif /*Ud_Ui_Ut_r10181u */

#ifndef Ud_Ui_Ut_10181u
#define Ud_Ui_Ut_10181u
namespace MFM{

  template<class EC> class Ui_Ut_r10181u;  //forward //gcnl:UlamTypePrimitive:

  template<class EC>
  struct Ui_Ut_10181u : public BitVectorBitStorage<EC, BitVector<8u> >
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    typedef BitVector<8> BV; //gcnl:UlamTypePrimitive.cpp:464
    typedef BitVectorBitStorage<EC, BV> BVS; //gcnl:UlamTypePrimitive.cpp:467

    const u32 read() const { return BVS::Read(0u, 8u); } //gcnl:UlamTypePrimitive.cpp:556
    void write(const u32& v) { BVS::Write(0u, 8u, v); } //gcnl:UlamTypePrimitive.cpp:606
    Ui_Ut_10181u() { } //gcnl:UlamTypePrimitive.cpp:480
    Ui_Ut_10181u(const u32 d) { write(d); } //gcnl:UlamTypePrimitive.cpp:488
    Ui_Ut_10181u(const Ui_Ut_10181u& other) : BVS() { this->write(other.read()); } //gcnl:UlamTypePrimitive.cpp:511
    Ui_Ut_10181u(const Ui_Ut_r10181u<EC>& d) { this->write(d.read()); } //gcnl:UlamTypePrimitive.cpp:520
    virtual const char * GetUlamTypeMangledName() const { return "Ut_10181u"; } //gcnl:UlamType.cpp:929
  };
} //MFM
#endif /*Ud_Ui_Ut_10181u */

#ifndef Ud_Ui_Uq_r10109210ByteSource10
#define Ud_Ui_Uq_r10109210ByteSource10
namespace MFM{

  template<class EC> class Uq_10109210ByteSource10;  //forward //gcnl:UlamTypeClassQuark.cpp:166
  template<class EC> class Ui_Uq_10109210ByteSource10;  //forward //gcnl:UlamTypeClassQuark.cpp:171

  template<class EC>
  struct Ui_Uq_r10109210ByteSource10 : public UlamRef<EC> //gcnl:UlamTypeClassQuark.cpp:180
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    enum { QUARK_SIZE = 0}; //gcnl:UlamTypeClassQuark.cpp:192

    typedef Uq_10109210ByteSource10<EC> Us; //gcnl:UlamTypeClassQuark.cpp:199
    typedef Ui_Uq_10109210ByteSource10<EC> Usi; //gcnl:UlamTypeClassQuark.cpp:205

     u32 read() const { if(&Us::THE_INSTANCE==this->GetEffectiveSelf()) return UlamRef<EC>::Read();/* entire quark */else { return Usi(*this).read(); } } //gcnl:UlamTypeClassQuark.cpp:328
    void write(const u32& targ) { /* noop */ } //gcnl:UlamTypeClassQuark.cpp:423
    Ui_Uq_r10109210ByteSource10(BitStorage<EC>& targ, u32 idx, const UlamClass<EC>* effself, const UlamContext<EC>& uc) : UlamRef<EC>(idx, 0u, targ, effself, UlamRef<EC>::CLASSIC, uc) { } //gcnl:UlamTypeClassQuark.cpp:227
    Ui_Uq_r10109210ByteSource10(BitStorage<EC>& targ, u32 idx, u32 postoeff, const UlamClass<EC>* effself, const typename UlamRef<EC>::UsageType usage, const UlamContext<EC>& uc) : UlamRef<EC>(idx, 0u, postoeff, targ, effself, usage, uc) { } //gcnl:UlamTypeClassQuark.cpp:237
    Ui_Uq_r10109210ByteSource10(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself) : UlamRef<EC>(arg, idx, 0u, effself, UlamRef<EC>::CLASSIC) { } //gcnl:UlamTypeClassQuark.cpp:249
    Ui_Uq_r10109210ByteSource10(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself, const typename UlamRef<EC>::UsageType usage) : UlamRef<EC>(arg, idx, 0u, effself, usage) { } //gcnl:UlamTypeClassQuark.cpp:258
    Ui_Uq_r10109210ByteSource10(const UlamRef<EC>& r) : UlamRef<EC>(r, 0u) { } //gcnl:UlamTypeClassQuark.cpp:266
    Ui_Uq_r10109210ByteSource10(const UlamRef<EC>& r, s32 idx) : UlamRef<EC>(r, idx, 0u, true) { } //gcnl:UlamTypeClassQuark.cpp:274
    Ui_Uq_r10109210ByteSource10(const Ui_Uq_r10109210ByteSource10<EC>& r) : UlamRef<EC>(r, r.GetLen()) { } //gcnl:UlamTypeClassQuark.cpp:283
    Ui_Uq_r10109210ByteSource10& operator=(const Ui_Uq_r10109210ByteSource10& rhs); //declare away //gcnl:UlamTypeClassQuark.cpp:292
  };
} //MFM
#endif /*Ud_Ui_Uq_r10109210ByteSource10 */

#ifndef Ud_Ui_Uq_10109210ByteSource10
#define Ud_Ui_Uq_10109210ByteSource10
namespace MFM{

  template<class EC> class Uq_10109210ByteSource10; //forward //gcnl:UlamTypeClassQuark.cpp:498
  template<class EC> class Ui_Uq_r10109210ByteSource10;

  template<class EC>
  struct Ui_Uq_10109210ByteSource10 : public BitVectorBitStorage<EC, BitVector<0> >
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    enum { QUARK_SIZE = 0}; //gcnl:UlamTypeClassQuark.cpp:523
    typedef BitVector<0> BV; //gcnl:UlamTypeClassQuark.cpp:531
    typedef BitVectorBitStorage<EC, BV> BVS; //gcnl:UlamTypeClassQuark.cpp:534

    typedef Uq_10109210ByteSource10<EC> Us; //gcnl:UlamTypeClassQuark.cpp:553

    const u32 read() const { return BVS::Read(0u, QUARK_SIZE); } //gcnl:UlamTypeClassQuark.cpp:709
    void write(const u32 v) { BVS::Write(0u, QUARK_SIZE, v); } //gcnl:UlamTypeClassQuark.cpp:767
    Ui_Uq_10109210ByteSource10() { write(Us::THE_INSTANCE.getDefaultQuark()); } //gcnl:UlamTypeClassQuark.cpp:571
    Ui_Uq_10109210ByteSource10(const u32 d) { write(d); } //gcnl:UlamTypeClassQuark.cpp:597
Ui_Uq_10109210ByteSource10(const Ui_Uq_10109210ByteSource10<EC> & arg) : BVS() { write(arg.read()); } //gcnl:UlamTypeClassQuark.cpp:606
    Ui_Uq_10109210ByteSource10(const Ui_Uq_r10109210ByteSource10<EC>& d) { if(&Us::THE_INSTANCE==d.GetEffectiveSelf()) write(d.read()); else {} } //gcnl:UlamTypeClassQuark.cpp:672
    virtual const char * GetUlamTypeMangledName() const { return "Uq_10109210ByteSource10"; } //gcnl:UlamType.cpp:929
  };
} //MFM
#endif /*Ud_Ui_Uq_10109210ByteSource10 */

#ifndef Ud_Ui_Uq_r10108ByteSink10
#define Ud_Ui_Uq_r10108ByteSink10
namespace MFM{

  template<class EC> class Uq_10108ByteSink10;  //forward //gcnl:UlamTypeClassQuark.cpp:166
  template<class EC> class Ui_Uq_10108ByteSink10;  //forward //gcnl:UlamTypeClassQuark.cpp:171

  template<class EC>
  struct Ui_Uq_r10108ByteSink10 : public UlamRef<EC> //gcnl:UlamTypeClassQuark.cpp:180
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    enum { QUARK_SIZE = 0}; //gcnl:UlamTypeClassQuark.cpp:192

    typedef Uq_10108ByteSink10<EC> Us; //gcnl:UlamTypeClassQuark.cpp:199
    typedef Ui_Uq_10108ByteSink10<EC> Usi; //gcnl:UlamTypeClassQuark.cpp:205

    const u32 read() const { if(&Us::THE_INSTANCE==this->GetEffectiveSelf()) return UlamRef<EC>::Read();/* entire quark */else { return Usi(*this).read(); } } //gcnl:UlamTypeClassQuark.cpp:328
    void write(const u32& targ) { /* noop */ } //gcnl:UlamTypeClassQuark.cpp:423
    Ui_Uq_r10108ByteSink10(BitStorage<EC>& targ, u32 idx, const UlamClass<EC>* effself, const UlamContext<EC>& uc) : UlamRef<EC>(idx, 0u, targ, effself, UlamRef<EC>::CLASSIC, uc) { } //gcnl:UlamTypeClassQuark.cpp:227
    Ui_Uq_r10108ByteSink10(BitStorage<EC>& targ, u32 idx, u32 postoeff, const UlamClass<EC>* effself, const typename UlamRef<EC>::UsageType usage, const UlamContext<EC>& uc) : UlamRef<EC>(idx, 0u, postoeff, targ, effself, usage, uc) { } //gcnl:UlamTypeClassQuark.cpp:237
    Ui_Uq_r10108ByteSink10(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself) : UlamRef<EC>(arg, idx, 0u, effself, UlamRef<EC>::CLASSIC) { } //gcnl:UlamTypeClassQuark.cpp:249
    Ui_Uq_r10108ByteSink10(const UlamRef<EC>& arg, s32 idx, const UlamClass<EC>* effself, const typename UlamRef<EC>::UsageType usage) : UlamRef<EC>(arg, idx, 0u, effself, usage) { } //gcnl:UlamTypeClassQuark.cpp:258
    Ui_Uq_r10108ByteSink10(const UlamRef<EC>& r) : UlamRef<EC>(r, 0u) { } //gcnl:UlamTypeClassQuark.cpp:266
    Ui_Uq_r10108ByteSink10(const Ui_Uq_r10108ByteSink10<EC>& r) : UlamRef<EC>(r, r.GetLen()) { } //gcnl:UlamTypeClassQuark.cpp:283
    Ui_Uq_r10108ByteSink10& operator=(const Ui_Uq_r10108ByteSink10& rhs); //declare away //gcnl:UlamTypeClassQuark.cpp:292
  };
} //MFM
#endif /*Ud_Ui_Uq_r10108ByteSink10 */

#ifndef Ud_Ui_Uq_10108ByteSink10
#define Ud_Ui_Uq_10108ByteSink10
namespace MFM{

  template<class EC> class Uq_10108ByteSink10; //forward //gcnl:UlamTypeClassQuark.cpp:498
  template<class EC> class Uq_r10108ByteSink10; //forward //gcnl:UlamTypeClassQuark.cpp

  template<class EC>
  struct Ui_Uq_10108ByteSink10 : public BitVectorBitStorage<EC, BitVector<0> >
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    enum { QUARK_SIZE = 0}; //gcnl:UlamTypeClassQuark.cpp:523
    typedef BitVector<0> BV; //gcnl:UlamTypeClassQuark.cpp:531
    typedef BitVectorBitStorage<EC, BV> BVS; //gcnl:UlamTypeClassQuark.cpp:534

    typedef Uq_10108ByteSink10<EC> Us; //gcnl:UlamTypeClassQuark.cpp:553

    const u32 read() const { return BVS::Read(0u, QUARK_SIZE); } //gcnl:UlamTypeClassQuark.cpp:709
    void write(const u32 v) { BVS::Write(0u, QUARK_SIZE, v); } //gcnl:UlamTypeClassQuark.cpp:767
    Ui_Uq_10108ByteSink10() { write(Us::THE_INSTANCE.getDefaultQuark()); } //gcnl:UlamTypeClassQuark.cpp:571
    Ui_Uq_10108ByteSink10(const u32 d) { write(d); } //gcnl:UlamTypeClassQuark.cpp:597
    Ui_Uq_10108ByteSink10(const Ui_Uq_10108ByteSink10<EC> & arg) : BVS() { write(arg.read()); } //gcnl:UlamTypeClassQuark.cpp:606
    Ui_Uq_10108ByteSink10(const Ui_Uq_r10108ByteSink10<EC>& d) { if(&Us::THE_INSTANCE==d.GetEffectiveSelf()) write(d.read()); else {} } //gcnl:UlamTypeClassQuark.cpp:672
    virtual const char * GetUlamTypeMangledName() const { return "Uq_10108ByteSink10"; } //gcnl:UlamType.cpp:929
  };
} //MFM
#endif /*Ud_Ui_Uq_10108ByteSink10 */

namespace MFM{
  template<class EC>
  struct _UlamByteSinkWrapper : public ByteSink {
    const UlamContext<EC>& mUC;
    UlamRef<EC>& mUR;
    typename Uq_10109216ByteStreamWriter10<EC>::Uf_919writeByte1110181u mWriteByteFunc;
    _UlamByteSinkWrapper(const UlamContext<EC>& uc, UlamRef<EC>& ur, typename Uq_10109216ByteStreamWriter10<EC>::Uf_919writeByte1110181u func)
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
    typename Uq_10109216ByteStreamReader10<EC>::Uf_8readByte10 mReadByteFunc;
    _UlamByteSourceWrapper(const UlamContext<EC>& uc, UlamRef<EC>& ur, typename Uq_10109216ByteStreamReader10<EC>::Uf_8readByte10 func)
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
