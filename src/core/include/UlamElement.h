/*                                              -*- mode:C++ -*-
  UlamElement.h A concrete base class for ULAM elements
  Copyright (C) 2014-2015 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2015 Ackleyshack, LLC.

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
  \date (C) 2014-2015 All rights reserved.
  \lgpl
 */
#ifndef ULAMELEMENT_H
#define ULAMELEMENT_H

#include "UlamClass.h"
#include "UlamRef.h"

// Unsigned(32)
#ifndef Ud_Ui_Ut_r102321u
#define Ud_Ui_Ut_r102321u
namespace MFM{
  template<class EC, u32 POS>
  struct Ui_Ut_r102321u : public AutoRefBase<EC>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    Ui_Ut_r102321u(T& targ, u32 idx) : AutoRefBase<EC>(targ, idx) { }
    Ui_Ut_r102321u(AutoRefBase<EC>& arg, u32 idx) : AutoRefBase<EC>(arg, idx) { }
    const u32 read() const { return AutoRefBase<EC>::read(32u); }
    void write(const u32 v) { AutoRefBase<EC>::write(v, 32u); }
  };
} //MFM
#endif /*Ud_Ui_Ut_r102321u */

#ifndef Ud_Ui_Ut_102321u
#define Ud_Ui_Ut_102321u
namespace MFM{
  template<class EC>
  struct Ui_Ut_102321u : public Ui_Ut_r102321u<EC, 64u>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    typedef BitField<BitVector<BPA>, VD::U32, 32, 64> BF;
    T m_stg;  //storage here!

    Ui_Ut_102321u() : Ui_Ut_r102321u<EC, 64u>(m_stg, 39u), m_stg(T::ATOM_UNDEFINED_TYPE) { }
    Ui_Ut_102321u(const u32 d) : Ui_Ut_r102321u<EC, 64u>(m_stg, 39u), m_stg(T::ATOM_UNDEFINED_TYPE) { Ui_Ut_r102321u<EC, 64u>::write(d); }
    Ui_Ut_102321u(const Ui_Ut_102321u& other) : Ui_Ut_r102321u<EC, 64u>(m_stg, 39u), m_stg(other.m_stg) { }
    ~Ui_Ut_102321u() {}
    const u32 read() const { return BF::Read(AutoRefBase<EC>::getBits()); }
    void write(const u32 v) { BF::Write(AutoRefBase<EC>::getBits(), v); }
  };
} //MFM
#endif /*Ud_Ui_Ut_102321u */

// Unsigned(8) [4] -- for ARGB colors
#ifndef Ud_Ui_Ut_r14181u
#define Ud_Ui_Ut_r14181u
namespace MFM{
  template<class EC, u32 POS>
  struct Ui_Ut_r14181u : public AutoRefBase<EC>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    Ui_Ut_r14181u(T& targ, u32 idx) : AutoRefBase<EC>(targ, idx) { }
    Ui_Ut_r14181u(AutoRefBase<EC>& arg, u32 idx) : AutoRefBase<EC>(arg, idx) { }
    const u32 read() const { return AutoRefBase<EC>::read(32u); } //reads entire array
    const u32 readArrayItem(const u32 index, const u32 itemlen) const { return AutoRefBase<EC>::readArrayItem(index, itemlen); }
    void write(const u32 v) { AutoRefBase<EC>::write(v, 32u); } //writes entire array
    void writeArrayItem(const u32 v, const u32 index, const u32 itemlen) { AutoRefBase<EC>::writeArrayItem(v, index, itemlen); }
  };
} //MFM
#endif /*Ud_Ui_Ut_r14181u */

#ifndef Ud_Ui_Ut_14181u
#define Ud_Ui_Ut_14181u
namespace MFM{

  template<class EC>
  struct Ui_Ut_14181u : public Ui_Ut_r14181u<EC, 64u>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    typedef BitField<BitVector<BPA>, VD::BITS, 32, 64> BF;
    T m_stg;  //storage here!

    Ui_Ut_14181u() : Ui_Ut_r14181u<EC, 64u>(m_stg, 39u), m_stg(T::ATOM_UNDEFINED_TYPE) { }
    Ui_Ut_14181u(const u32 d) : Ui_Ut_r14181u<EC, 64u>(m_stg, 39u), m_stg(T::ATOM_UNDEFINED_TYPE) { Ui_Ut_r14181u<EC, 64u>::write(d); }
    Ui_Ut_14181u(const Ui_Ut_14181u& other) : Ui_Ut_r14181u<EC, 64u>(m_stg, 39u), m_stg(other.m_stg) { }
    ~Ui_Ut_14181u() {}
    const u32 read() const { return BF::Read(AutoRefBase<EC>::getBits()); }   //reads entire array
    const u32 readArrayItem(const u32 index, const u32 itemlen) const { return AutoRefBase<EC>::readArrayItem(index, itemlen); }
    void write(const u32 v) { BF::Write(AutoRefBase<EC>::getBits(), v); }   //writes entire array
    void writeArrayItem(const u32 v, const u32 index, const u32 itemlen) { AutoRefBase<EC>::writeArrayItem(v, index, itemlen); }
  };
} //MFM
#endif

namespace MFM
{
  template <class EC> class UlamElement; // FORWARD
  template <class EC> class UlamContext; // FORWARD

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
  class UlamElement : public Element<EC>, public UlamClass<EC>
  {
    typedef Element<EC> Super;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    const UlamElementInfo<EC> * m_info;

  public:

    UlamElement(const UUID & uuid) : Element<EC>(uuid) { }

    virtual ~UlamElement() { }

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
    void Print(const UlamClassRegistry<EC> & ucr, ByteSink & bs, const T & atom, u32 flags) const ;

    void SetInfo(const UlamElementInfo<EC> * info) {
      m_info = info;
      this->SetName(m_info->GetName());
      this->SetAtomicSymbol(m_info->GetSymbol());
    }

    virtual void Behavior(EventWindow<EC>& window) const ;

    /**
       Ulam elements that define 'Void behave()' will override this
       method, and it will be called on events!
     */
    virtual void Uf_6behave(const UlamContext<EC> & uc, UlamRef<EC>& self) const
    {
      // Empty by default
    }

    /**
       Ulam elements defining 'Unsigned getColor(Unsigned selector)'
       will override this method, and it will be called during
       graphics rendering!

       Note the Uv_4self in this method IS A COPY of the atom being
       rendered -- any changes made to Uv_4self during this method
       will vanish when it returns.

       Note also that THERE IS NO EVENT IN PROGRESS when this method
       is called!  Any attempt to access event services during this
       method will fail!  That includes event window accesses AND
       random numbers!

       This base class implementation, if not overridden, yields the
       element color for all atoms and selectors.
     */
    virtual Ui_Ut_14181u<EC> Uf_8getColor(const UlamContext<EC>& uc,
					  T& Uv_4self,
					  Ui_Ut_102321u<EC> Uv_8selector) const
    {
      return Ui_Ut_14181u<EC>(this->GetElementColor());
    }

    virtual bool GetPlaceable() const
    {
      return m_info?m_info->GetPlaceable() : true;
    }

    virtual UlamElement* AsUlamElement()
    {
      return this;
    }

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

    virtual u32 GetAtomColor(const T& atom, u32 selector) const
    {
      if (selector == 0)
        return GetElementColor();

      const UlamContext<EC> uc;
      T temp(atom);
      Ui_Ut_102321u<EC> sel(selector);
      Ui_Ut_14181u<EC> dynColor = Uf_8getColor(uc, temp, sel);
      return dynColor.read();
    }

    virtual u32 Diffusability(EventWindow<EC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      if (nowAt == maybeAt || !m_info) return COMPLETE_DIFFUSABILITY;
      return
        COMPLETE_DIFFUSABILITY * m_info->GetPercentDiffusability() / 100;
    }
  };

} // MFM

#include "UlamElement.tcc"

#endif /* ULAMELEMENT_H */
