/*                                              -*- mode:C++ -*-
  UlamRefMutable.h  Temporary holder for ulam reference data
  Copyright (C) 2019-2020 The Regents of the University of New Mexico.  All rights reserved.
  Copyright (C) 2019-2022 ackleyshack LLC.  All rights reserved.
  Copyright (C) 2020-2022 The Living Computation Foundation.  All rights reserved.

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
  \file UlamRefMutable.h  Temporary holder for ulam reference data
  \author David H. Ackley.
  \author Elena S. Ackley.
  \date (C) 2019-2022 All rights reserved.
  \lgpl
 */
#ifndef ULAMREFMUTABLE_H
#define ULAMREFMUTABLE_H

#include "UlamRef.h"
//#include "UlamClass.h"
//#include "BitStorage.h"

namespace MFM
{

  /**
   * A UlamRefMutable is a temporary holder for ulam reference variables
   */
  template <class EC>
  class UlamRefMutable
  {
  public:

/*
    enum UsageType {
      PRIMITIVE,  //< Bits used as primitive, type cannot change at runtime, derived URs cannot change usage type, no effself
      ARRAY,      //< Bits used as array, type cannot change at runtime, derived URs can change usage type, no effself
      ATOMIC,     //< Bits used as atom, type may change at runtime, derived URs can change usage type, has effself
      ELEMENTAL,  //< Bits used as element (inside atom), type cannot (?) change at runtime, derived URs can change usage type, has effself
      CLASSIC     //< Bits used as class-based non-atom stg, type cannot change at runtime, derived URs can change usage type, has effself
    };
*/

    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    typedef typename UlamRef<EC>::UsageType UsageT;

    /**
       Construct an UlamRefMutable 'from scratch'
     */
    UlamRefMutable() ;

    UlamContext<EC> * GetContextPtr() const { return m_ucptr; }
    UlamClass<EC> * GetEffectiveSelfPtr() const { return m_effSelf; }
    BitStorage<EC> * GetBitStoragePtr() const { return m_stgptr; }
    u32 GetPos() const { return m_pos; }
    u32 GetLen() const { return m_len; }
    UsageT GetUsageType() const { return m_usage; }
    u32 GetPosToEffectiveSelf() const { return m_posToEff; }
    u32 GetVTableClassId() const { return m_vtableclassid; }
    u32 GetDataMemberOffsetInStorage() const { return m_dmOffsetInStg;}
    const UlamRef<EC> * GetPreviousUlamRefPtr() const { return m_prevur; }

    UlamRefMutable<EC>& operator=(const UlamRef<EC>& rhs);

  private:

    // DATA MEMBERS
    UlamContext<EC> * m_ucptr; //pointer, not ref
    UlamClass<EC> * m_effSelf; //non-const, may be NULL
    BitStorage<EC> * m_stgptr; //pointer, not ref
    const UlamRef<EC> * m_prevur;
    u16 m_pos;
    u16 m_len;
    UsageT m_usage;
    u16 m_posToEff;
    u16 m_vtableclassid;
    u16 m_dmOffsetInStg;
  }; //UlamRefMutable

} // MFM

#include "UlamRefMutable.tcc"

#endif /* ULAMREFMUTABLE_H */
