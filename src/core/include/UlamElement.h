/*                                              -*- mode:C++ -*-
  UlamElement.h A concrete base class for ULAM elements
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

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
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ULAMELEMENT_H
#define ULAMELEMENT_H

#include "Element.h"
#include "Tile.h"
#include "EventWindow.h"
#include "UlamContext.h"

#ifndef Ud_Ui_Ut_102328Unsigned
#define Ud_Ui_Ut_102328Unsigned
namespace MFM{
  struct Ui_Ut_102328Unsigned
  {
    typedef BitField<BitVector<32>, VD::U32, 32, 0> BF;
    BitVector<32> m_stg;
    Ui_Ut_102328Unsigned() : m_stg() { }
    Ui_Ut_102328Unsigned(const u32 d) : m_stg(d) {}
    Ui_Ut_102328Unsigned(const Ui_Ut_102328Unsigned& d) : m_stg(d.m_stg) {}
    ~Ui_Ut_102328Unsigned() {}
    const u32 read() const { return BF::Read(m_stg); }
    void write(const u32 v) { BF::Write(m_stg, v); }
  };
} //MFM
#endif /*Ud_Ui_Ut_102328Unsigned */

// Unsigned(8) [4] -- for ARGB colors
#ifndef Ud_Ui_Ut_14188Unsigned
#define Ud_Ui_Ut_14188Unsigned
namespace MFM{
  struct Ui_Ut_14188Unsigned
  {
    typedef BitField<BitVector<32>, VD::BITS, 32, 0> BF;
    BitVector<32> m_stg;
    Ui_Ut_14188Unsigned() : m_stg() { }
    Ui_Ut_14188Unsigned(const u32 d) : m_stg(d) {}
    Ui_Ut_14188Unsigned(const Ui_Ut_14188Unsigned& d) : m_stg(d.m_stg) {}
    ~Ui_Ut_14188Unsigned() {}
    const u32 read() const { return BF::Read(m_stg); }   //reads entire array
    const u32 readArrayItem(const u32 index, const u32 unitsize) const { return BF::ReadArray(m_stg, index, unitsize); }
    void write(const u32 v) { BF::Write(m_stg, v); }   //writes entire array
    void writeArrayItem(const u32 v, const u32 index, const u32 unitsize) { BF::WriteArray(m_stg, v, index, unitsize); }
  };
} //MFM
#endif /*Ud_Ui_Ut_14188Unsigned */

namespace MFM
{
  template <class EC> class UlamElement; // FORWARD

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
    virtual const char * GetLicense() const = 0;
    virtual const u32 GetVersion() const = 0;
    virtual const u32 GetNumColors() const = 0;
    virtual const u32 GetColor(UlamContext<EC>& uc, T atom, u32 colnum) const = 0;
    virtual const u32 GetSymmetry(UlamContext<EC>& uc) const = 0;

    virtual const u32 GetPercentDiffusability() const
    {
      return 100;
    }

    UlamElementInfo() { }
    virtual ~UlamElementInfo() { }
  };

  /**
   * A UlamElement is a concrete element primarily for use by culam.
   */
  template <class EC>
  class UlamElement : public Element<EC>
  {
    typedef Element<EC> Super;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    const UlamElementInfo<EC> * m_info;

  public:
    UlamElement(const UUID & uuid) : Element<EC>(uuid)
    { }

    void SetInfo(const UlamElementInfo<EC> * info) {
      m_info = info;
      this->SetName(m_info->GetName());
      this->SetAtomicSymbol(m_info->GetSymbol());
    }

    /**
     * Destroys this UlamElement.
     */
    virtual ~UlamElement()
    { }

    virtual void Behavior(EventWindow<EC>& window) const
    {
      Tile<EC> & tile = window.GetTile();
      UlamContext<EC> uc;
      uc.SetTile(tile);

      u32 sym = m_info ? m_info->GetSymmetry(uc) : PSYM_DEG000L;
      window.SetSymmetry((PointSymmetry) sym);

      T & me = window.GetCenterAtomSym();
      Uf_6behave(uc, me);
    }

    /**
       Ulam elements that define 'Void behave()' will override this
       method, and it will be called on events!
     */
    virtual void Uf_6behave(UlamContext<EC> & uc, T& self) const
    {
      // Empty by default
    }

    /**
       Ulam elements that define 'Unsigned getColor()' will override
       this method, and it will be called during graphics rendering!

       Note the Uv_4self in this method IS A COPY of the atom being
       rendered -- any changes made to Uv_4self during this method
       will vanish when it returns.

       Note also that THERE IS NO EVENT IN PROGRESS when this method
       is called!  Any attempt to access event services during this
       method will fail!  That includes event window accesses AND
       random numbers!
     */
    virtual Ui_Ut_14188Unsigned Uf_8getColor(UlamContext<EC>& uc,
                                             T& Uv_4self,
                                             Ui_Ut_102328Unsigned Uv_8selector) const
    {
      return Ui_Ut_14188Unsigned(0xffffffff);
    }

    virtual const UlamElement<EC> * AsUlamElement() const
    {
      return this;
    }

    /**
       Find the first position of a data member in this element by the
       name of its type, if any exist.

       \return The smallest bit position of an occurrence of the type
               \c dataMemberTypeName in this element, if any, or -1 to
               indicate the given type is not used as a data member in
               this UlamElement.  A return value of 0 corresponds to
               the ATOM_FIRST_STATE_BIT of the atom.

       \sa T::ATOM_FIRST_STATE_BIT
     */
    virtual s32 PositionOfDataMemberType(const char * dataMemberTypeName) const
    {
      FAIL(ILLEGAL_STATE);  // culam should always have overridden this method
    }

    /**
       Find the first position of a data member, specified by its \c
       dataMemberTypeName, in an UlamElement specified by its \c type
       number, if such as UlamElement exists and has such a data
       member.

       \param type an element type number, hopefully of an UlamElement

       \param dataMemberTypeName the name of the type to search for in
              the data members of the found UlamElement.

       \return The smallest bit position of an occurrence of the type
               \c dataMemberTypeName in this element, if any.  Note
               that a return value of 0 corresponds to the
               ATOM_FIRST_STATE_BIT of the atom.

               A return value of -1 indicates the given \c type is not
               associated with any known type of element.

               A return value of -2 indicates the given \c type is
               associated with an Element that is not an UlamElement,
               so its data members cannot be searched.

               A return value of -3 indicates the given \c type is
               associated with an UlamElement, which did not contain a
               data member of the named type.

       \sa T::ATOM_FIRST_STATE_BIT
       \sa PositionOfDataMemberType
     */
    static s32 PositionOfDataMember(UlamContext<EC>& uc, u32 type, const char * dataMemberTypeName)
    {
      Tile<EC> & tile = uc.GetTile();
      ElementTable<EC> & et = tile.GetElementTable();
      const Element<EC> * eltptr = et.Lookup(type);
      if (!eltptr) return -1;
      const UlamElement<EC> * ueltptr = eltptr->AsUlamElement();
      if (!ueltptr) return -2;
      s32 ret = ueltptr->PositionOfDataMemberType(dataMemberTypeName);
      if (ret < 0) return -3;
      return ret;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      if (m_info && m_info->GetNumColors() > 0) {
        UlamContext<EC> uc;
        return m_info->GetColor(uc, this->GetDefaultAtom(), 0);
      }
      return 0xffffffff;
    }

    virtual u32 LocalPhysicsColor(const T& atom, u32 selector) const
    {
      if (m_info && m_info->GetNumColors() > selector) {
        UlamContext<EC> uc;
        return m_info->GetColor(uc, atom, selector);
      }
      return this->PhysicsColor();
    }

    virtual u32 Diffusability(EventWindow<EC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      if (nowAt == maybeAt || !m_info) return COMPLETE_DIFFUSABILITY;
      return
        COMPLETE_DIFFUSABILITY * m_info->GetPercentDiffusability() / 100;
    }
  };
} // MFM

#endif /* ULAMELEMENT_H */
