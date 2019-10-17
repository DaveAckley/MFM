/*                                              -*- mode:C++ -*-
  UlamRef.h A base for ulam references
  Copyright (C) 2016,2019 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2019 ackleyshack LLC.  All rights reserved.

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
  \author Elena S. Ackley.
  \date (C) 2016,2019 All rights reserved.
  \lgpl
 */
#ifndef ULAMREF_H
#define ULAMREF_H

#include "UlamClass.h"
#include "BitStorage.h"
#include "ByteSink.h"
#include "UlamVTableEntry.h"

namespace MFM
{

  template <class EC> class UlamRefMutable; // FORWARD

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
       Construct an UlamRef 'from scratch', when pos isnt effSelf (i.e. base class of effSelf)
     */
    UlamRef(u32 pos, u32 len, u32 postoeff, BitStorage<EC>& stg, const UlamClass<EC> * effself,
            const UsageType usage, const UlamContext<EC> & uc) ;

    /**
       Construct an UlamRef that's relative to an existing UlamRef.
       The 'pos' supplied here will be relative to the the existing
       pos, and this pos + len must fit within the len supplied to the
       existing UlamRef.
     */
    UlamRef(const UlamRef<EC> & existing, s32 posincr, u32 len, const UlamClass<EC> * effself, const UsageType usage) ;

    /**
       Construct an UlamRef that's related (not data member) to an existing UlamRef.
       Same effective self, and usage; pos (ulam-5), and len may change;
       and pos + len must fit within the len supplied to the
       existing UlamRef.
     */
    UlamRef(const UlamRef<EC> & existing, s32 posincr, u32 len) ;

    /**
       Construct an UlamRef that's related (not data member) to an existing UlamRef.
       Same effective self, and usage; pos and len may change;
       and pos + len must fit within the len supplied to the
       existing UlamRef.
       (note: 'applydelta' is a de-ambiguity arg).
     */
    UlamRef(const UlamRef<EC> & existing, s32 effselfoffset, u32 len, bool applydelta) ;

    /**
	Construct an UlamRef for a virtual function call, based on the
	existing EffectiveSelf;
	Returns the VfuncPtr, as well as the UlamRef to use in the vf call;
	Cast the void* VfuncPtr to the vfunc's typedef (see origclass' .h);

	Invarient: the new 'ur' has pos to the Override class found in
	EffectiveSelf's VTtable for this vfunc's VOWNED_IDX +
	originating class start offset;
    */
    UlamRef(const UlamRef<EC> & existing, u32 vownedfuncidx, const UlamClass<EC> & origclass, VfuncPtr & vfuncref) ;

    /**
       Construct an UlamRef for a virtual function call, overloaded to
       take RegistryNumber instead of UlamClass of originating class;
       (note: 'applydelta' is a de-ambiguity arg)
    */
    UlamRef(const UlamRef<EC> & existing, u32 vownedfuncidx, u32 origclassregnum, bool applydelta, VfuncPtr & vfuncref) ;


    /** Construct an UlamRef from a UlamRefMutable (e.g. ?: expression)
     */
    UlamRef(const UlamRefMutable<EC> & muter);

    static BitStorage<EC> * checknonnullstg(BitStorage<EC> * stgptr) {
      MFM_API_ASSERT_NONNULL(stgptr);
      return stgptr;
    }

    static UlamContext<EC> * checknonnulluc(UlamContext<EC> * ucptr) {
      MFM_API_ASSERT_NONNULL(ucptr);
      return ucptr;
    }

    u32 Read() const { return m_stg.Read(m_pos, m_len); }

    void Write(u32 val) { m_stg.Write(m_pos, m_len, val); }

    u64 ReadLong() const { return m_stg.ReadLong(m_pos, m_len); }

    void WriteLong(u64 val) { m_stg.WriteLong(m_pos, m_len, val); }

    T ReadAtom() const
    {
      if (m_usage == ATOMIC) return m_stg.ReadAtom(m_pos);
      if (m_usage == ELEMENTAL) return m_stg.ReadAtom(GetEffectiveSelfPos() - T::ATOM_FIRST_STATE_BIT);
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
        m_stg.WriteAtom(GetEffectiveSelfPos() - T::ATOM_FIRST_STATE_BIT, val);
        CheckEffectiveSelf();
      }
      else FAIL(ILLEGAL_STATE);
    }

    /**
       Copy \c LEN bits from positions \c pos through \c pos + LEN - 1
       of \c this to positions 0 through LEN - 1 of \c rtnbv
    */
    template<u32 LEN>
    void ReadBV(u32 pos, BitVector<LEN>& rtnbv) const
    {
      m_stg.ReadBV<LEN>(pos + m_pos, rtnbv);
    }

    /**
       Copy \c LEN bits from positions 0 through LEN - 1 of \c val
       to positions \c pos through \c pos + LEN - 1 of \c this.
    */
    template<u32 LEN>
    void WriteBV(u32 pos, const BitVector<LEN>& val)
    {
      m_stg.WriteBV<LEN>(pos + m_pos, val);
    }

    u32 GetPos() const { return m_pos; }

    u32 GetLen() const { return m_len; }

    UsageType GetUsage() const { return m_usage; } //for UlamRefMutable


    //return beginning of element state bits;
    s32 GetEffectiveSelfPos() const { return (m_pos - m_posToEff); }

    //return the delta from existing pos to beginning of element state bits;
    s32 GetPosToEffectiveSelf() const { return m_posToEff; }

    const UlamClass<EC> * GetEffectiveSelf() const { CheckEffectiveSelf(); return m_effSelf; }

    UlamClass<EC> * GetEffectiveSelfPointer() const { return const_cast<UlamClass<EC> *> (m_effSelf); } //for UlamRefMutable

    BitStorage<EC> & GetStorage() { return m_stg; }

    BitStorage<EC> & GetStorage() const { return m_stg; }

    UlamContext<EC> * GetContextAsPointer() const { return const_cast<UlamContext<EC> *> (&m_uc); } //for UlamRefMutable

    u32 GetType() const ;

    T CreateAtom() const ;

    void Print(const UlamClassRegistry<EC>&, ByteSink&, u32 printFlags) const;

  private:
    // Declare away copy ctor
    UlamRef(const UlamRef<EC> & existing) ;

    void UpdateEffectiveSelf() ;

    void CheckEffectiveSelf() const ;

    const UlamClass<EC>* LookupUlamElementTypeFromAtom() const ;

    /** helper, creates EffectiveSelf-based UlamRef for virtual func call;
	Invarient: 'ur' of a virtual func points to the override class;
	EffectiveSelf can be a different class, i.e. override is a baseclass;
    */
    void InitUlamRefForVirtualFuncCall(const UlamRef<EC> & ur, u32 vownedfuncidx, u32 origclassregnum, VfuncPtr & vfuncref);

    /** helper, uses existing effselfpos and new effselfoffset to set our
	new m_pos and m_posToEff; m_len is also set;
	m_pos + m_len - m_posToEff must fit in m_stg.
    */
    void ApplyDelta(s32 existingeffselfpos, s32 effselfoffset, u32 len);


    // DATA MEMBERS
    const UlamContext<EC> & m_uc;
    const UlamClass<EC> * m_effSelf;
    BitStorage<EC> & m_stg;
    u32 m_pos;
    u32 m_len;
    UsageType m_usage;
    u32 m_posToEff;

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
