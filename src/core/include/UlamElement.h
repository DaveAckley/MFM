/*                                              -*- mode:C++ -*-
  UlamElement.h A concrete base class for ULAM elements
  Copyright (C) 2014-2017 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2015-2017 Ackleyshack, LLC.

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
  \file UlamElement.h A concrete base class for ULAM elements
  \author David H. Ackley.
  \author Elena S. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \lgpl
 */
#ifndef ULAMELEMENT_H
#define ULAMELEMENT_H

#include "ElementTable.h"
#include "UlamClass.h"
#include "BitStorage.h"

// Unsigned(32)
#ifndef Ud_Ui_Ut_102321u
#define Ud_Ui_Ut_102321u
namespace MFM{

  template<class EC> class Ui_Ut_r102321u;  //forward //gcnl:UlamTypePrimitive:

  template<class EC>
  struct Ui_Ut_102321u : public BitVectorBitStorage<EC, BitVector<32u> >
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    typedef BitVector<32> BV; //gcnl:UlamTypePrimitive.cpp:464
    typedef BitVectorBitStorage<EC, BV> BVS; //gcnl:UlamTypePrimitive.cpp:467

    const u32 read() const { return BVS::Read(0u, 32u); } //gcnl:UlamTypePrimitive.cpp:556
    void write(const u32& v) { BVS::Write(0u, 32u, v); } //gcnl:UlamTypePrimitive.cpp:606
    Ui_Ut_102321u() { } //gcnl:UlamTypePrimitive.cpp:480
    Ui_Ut_102321u(const u32 d) { write(d); } //gcnl:UlamTypePrimitive.cpp:488
    Ui_Ut_102321u(const Ui_Ut_102321u& other) : BVS() { this->write(other.read()); } //gcnl:UlamTypePrimitive.cpp:511
    Ui_Ut_102321u(const Ui_Ut_r102321u<EC>& d) { this->write(d.read()); } //gcnl:UlamTypePrimitive.cpp:520
    virtual const char * GetUlamTypeMangledName() const { return "Ut_102321u"; } //gcnl:UlamType.cpp:929
  };
} //MFM
#endif /*Ud_Ui_Ut_102321u */

//Unsigned(8) [4] (e.g. t3531 in UrSelf_Types.h)
#ifndef Ud_Ui_Ut_14181u
#define Ud_Ui_Ut_14181u
namespace MFM{

  template<class EC> class Ui_Ut_r14181u;  //forward //gcnl:UlamTypePrimitive:

  template<class EC>
  struct Ui_Ut_14181u : public BitVectorBitStorage<EC, BitVector<32u> >
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    typedef BitVector<32> BV; //gcnl:UlamTypePrimitive.cpp:464
    typedef BitVectorBitStorage<EC, BV> BVS; //gcnl:UlamTypePrimitive.cpp:467

    const u32 read() const { return BVS::Read(0u, 32u); } //reads entire array //gcnl:UlamTypePrimitive.cpp:560
    u32 readArrayItem(const u32 index, const u32 itemlen) const { return this->BVS::Read(index * itemlen, itemlen); } //reads BV array item //gcnl:UlamTypePrimitive.cpp:587
    void write(const u32& v) { BVS::Write(0u, 32u, v); } //writes entire array //gcnl:UlamTypePrimitive.cpp:610
    void writeArrayItem(const u32& v, const u32 index, const u32 itemlen) {this->BVS::Write(index * itemlen, itemlen, v); } //writes BV array item //gcnl:UlamTypePrimitive.cpp:635
    Ui_Ut_14181u() { } //gcnl:UlamTypePrimitive.cpp:480
    Ui_Ut_14181u(const u32 d) { write(d); } //gcnl:UlamTypePrimitive.cpp:488
    Ui_Ut_14181u(const u32 d[1]) : BVS(d) { } //gcnl:UlamTypePrimitive.cpp:499
    Ui_Ut_14181u(const Ui_Ut_14181u& other) : BVS() { this->write(other.read()); } //gcnl:UlamTypePrimitive.cpp:511
    Ui_Ut_14181u(const Ui_Ut_r14181u<EC>& d) { this->write(d.read()); } //gcnl:UlamTypePrimitive.cpp:520
    virtual const char * GetUlamTypeMangledName() const { return "Ut_14181u"; } //gcnl:UlamType.cpp:929
  };
} //MFM
#endif /*Ud_Ui_Ut_14181u */

namespace MFM
{
  template <class EC> class UlamElement; // FORWARD
  template <class EC> class UlamContext; // FORWARD
  template <class EC> class UlamRef; // FORWARD

  template <class EC>
  struct UlamElementInfo
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    virtual const char * GetName() const = 0;
    virtual const char * GetSymbol() const = 0;
    virtual const char * GetSummary() const = 0;
    virtual const char * GetDetails() const = 0;
    virtual const char * GetAuthor() const = 0;
    virtual const char * GetCopyright() const = 0;
    virtual const char * GetLicense() const = 0;
    virtual bool GetPlaceable() const = 0;
    virtual const u32 GetVersion() const = 0;
    virtual const u32 GetElementColor() const = 0;
    virtual const u32 GetEventWindowBoundary() const = 0;
    virtual const u32 GetSymmetry(const UlamContext<EC>& uc) const = 0;

    virtual const u32 GetPercentDiffusability() const
    {
      return 100;
    }

    // No longer needed? Going via existing parameter registry and access routes
    // virtual const u32 GetModelParameterCount() const = 0;

    // virtual UlamTypeInfoModelParameter<EC> & GetModelParameter(u32 index) const = 0;

    UlamElementInfo() { }
    virtual ~UlamElementInfo() { }
  };

} //MFM


namespace MFM {
  /**
   * A UlamElement is a concrete element primarily for use by culam.
   */
  template <class EC>
  class UlamElement : public UlamClass<EC>, public Element<EC>
  {
    typedef Element<EC> Super;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    const UlamElementInfo<EC> * m_info;

  public:

    enum SpecialVirtualVTableIndices {
      BEHAVE_VOWNED_INDEX = 0,  /* ==Uq_10106UrSelf10<EC>::VOWNED_IDX_Uf_6behave10 */
      GETCOLOR_VOWNED_INDEX = 1, /* ==Uq_10106UrSelf10<EC>::VOWNED_IDX_Uf_8getColor11102321u */
      RENDERGRAPHICS_VOWNED_INDEX = 2 /* ==Uq_10106UrSelf10<EC>::VOWNED_IDX_Uf_9214renderGraphics10 */
    };

    UlamElement(const UUID & uuid) : Element<EC>(uuid) { }

    virtual ~UlamElement() { }

    
    bool HasUlamElementInfo() const { return m_info != NULL; }

    u32 GetSymmetry(UlamContext<EC>& uc) const {
      u32 sym = m_info ? m_info->GetSymmetry(uc) : (u32) PSYM_DEG000L;
      return sym;
    }

    const UlamElementInfo<EC> & GetUlamElementInfo() const
    {
      MFM_API_ASSERT_NONNULL(m_info);
      return * m_info;
    }

    /**
       Print the contents of atom to the given ByteSink, including
       various details as specified by flags.
       \sa PrintFlags
     */
    void Print(const UlamClassRegistry<EC> & ucr, ByteSink & bs, const T & atom, u32 flags, u32 basestatepos) const ;

    void SetInfo(const UlamElementInfo<EC> * info) {
      m_info = info;
      this->SetName(m_info->GetName());
      this->SetAtomicSymbol(m_info->GetSymbol());
    }

    virtual void Behavior(EventWindow<EC>& window) const ;

    virtual u32 GetEventWindowBoundary() const ;

    virtual bool GetPlaceable() const
    {
      return m_info?m_info->GetPlaceable() : true;
    }

    /**
       Override AsUlamElement in BOTH Element<EC> AND UlamClass<EC>
     */
    virtual UlamElement* AsUlamElement()
    {
      return this;
    }

    /**
       Override AsUlamElement in BOTH Element<EC> AND UlamClass<EC>
     */
    virtual const UlamElement<EC> * AsUlamElement() const
    {
      return this;
    }

    virtual u32 GetElementColor() const
    {
      if (m_info) {
        return m_info->GetElementColor();
      }
      return 0xffffffff;
    }

    virtual u32 GetAtomColor(const ElementTable<EC> & et, const UlamClassRegistry<EC> & ucr, const T& atom, u32 selector) const ;

    virtual u32 Diffusability(EventWindow<EC> & ew, SPoint nowAt, SPoint maybeAt) const ;
  };

} // MFM

#include "UlamElement.tcc"

#endif /* ULAMELEMENT_H */
