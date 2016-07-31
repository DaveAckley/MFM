/*                                              -*- mode:C++ -*-
  UlamRef.h A base for ulam references
  Copyright (C) 2016 The Regents of the University of New Mexico.  All rights reserved.

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
  \file UlamRef.h A base class for ulam references
  \author David H. Ackley.
  \date (C) 2016 All rights reserved.
  \lgpl
 */
#ifndef ULAMREF_H
#define ULAMREF_H

#include "UlamClass.h"
#include "BitStorage.h"
#include "ByteSink.h"

namespace MFM
{

  /**
   * A UlamRef is a base class for ulam reference variables
   */
  template <class EC>
  class UlamRef
  {
  public:
    enum UsageType {
      PRIMITIVE,  //< Bits used as primitive, type cannot change at runtime, derived URs cannot change usage type, no effself
      ARRAY,      //< Bits used as array, type cannot change at runtime, derived URs can change usage type, no effself
      ATOMIC,     //< Bits used as atom, type may change at runtime, derived URs can change usage type, has effself
      ELEMENTAL,  //< Bits used as element (inside atom), type cannot (?) change at runtime, derived URs can change usage type, has effself
      CLASSIC     //< Bits used as class-based non-atom stg, type cannot change at runtime, derived URs can change usage type, has effself
    };

    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    /**
       Construct an UlamRef 'from scratch'
     */
    UlamRef(u32 pos, u32 len, BitStorage<EC>& stg, const UlamClass<EC> * effself,
            const UsageType usage, const UlamContext<EC> & uc) ;

    /**
       Construct an UlamRef that's relative to an existing UlamRef.
       The 'pos' supplied here will be relative to the the existing
       pos, and this pos + len must fit within the len supplied to the
       existing UlamRef.
     */
    UlamRef(const UlamRef<EC> & existing, s32 pos, u32 len, const UlamClass<EC> * effself, const UsageType usage) ;

    /**
       Construct an UlamRef that's related (not data member) to an existing UlamRef.
       Same 'pos', effective self, and usage; len may change;
       and pos + len must fit within the len supplied to the
       existing UlamRef.
     */
    UlamRef(const UlamRef<EC> & existing, u32 len) ;

    u32 Read() const { return m_stg.Read(m_pos, m_len); }

    void Write(u32 val) { m_stg.Write(m_pos, m_len, val); }

    u64 ReadLong() const { return m_stg.ReadLong(m_pos, m_len); }

    void WriteLong(u64 val) { m_stg.WriteLong(m_pos, m_len, val); }

    T ReadAtom() const
    {
      if (m_usage == ATOMIC) return m_stg.ReadAtom(m_pos);
      if (m_usage == ELEMENTAL) return m_stg.ReadAtom(m_pos - T::ATOM_FIRST_STATE_BIT);
      FAIL(ILLEGAL_STATE);
    }

    void WriteAtom(const T& val)
    {
      if (m_usage == ATOMIC)
      {
        m_stg.WriteAtom(m_pos, val);
        UpdateEffectiveSelf();
      }
      else if (m_usage == ELEMENTAL)
      {
        m_stg.WriteAtom(m_pos - T::ATOM_FIRST_STATE_BIT, val);
        CheckEffectiveSelf();
      }
      else FAIL(ILLEGAL_STATE);
    }

    u32 GetPos() const { return m_pos; }

    u32 GetLen() const { return m_len; }

    const UlamClass<EC> * GetEffectiveSelf() const { CheckEffectiveSelf(); return m_effSelf; }

    BitStorage<EC> & GetStorage() { return m_stg; }

    u32 GetType() const ;

    T CreateAtom() const ;

    void Print(const UlamClassRegistry<EC>&, ByteSink&, u32 printFlags) const;

  private:
    // Declare away copy ctor
    UlamRef(const UlamRef<EC> & existing) ;

    void UpdateEffectiveSelf() ;

    void CheckEffectiveSelf() const ;

    const UlamClass<EC>* LookupUlamElementTypeFromAtom() const ;

    // DATA MEMBERS
    const UlamContext<EC> & m_uc;
    const UlamClass<EC> * m_effSelf;
    BitStorage<EC> & m_stg;
    u32 m_pos;
    u32 m_len;
    UsageType m_usage;

  }; //UlamRef

  template <class EC, u32 POS, u32 LEN>
  struct UlamRefFixed : public UlamRef<EC>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    //omit BitStorage constructor since doesn't support transients
    UlamRefFixed(const UlamRef<EC>& parent, const UlamClass<EC> * effself)
      : UlamRef<EC>(parent, POS, LEN, effself)
    { }
  }; //UlamRefFixed

} // MFM

#include "UlamRef.tcc"

#endif /* ULAMREF_H */
