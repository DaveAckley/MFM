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
#include "BitVector.h"

namespace MFM
{
  template <class EC> class UlamContext; // FORWARD

  template <class EC>
  struct BitsRef
  {

    typedef typename EC::ATOM_CONFIG AC;
    enum { BITS_PER_ATOM = AC::BITS_PER_ATOM };
    typedef typename AC::ATOM_TYPE T;
    typedef BitVector<T::BITS> BV;

    u32 mask1;  // mask for unit1 bits left-shifted by shift1
    u32 mask2;  // mask for unit2 bits (if any) left-shifted by shift2
    u8 idx1;    // index of unit1
    u8 len1;    // number of bits for unit1
    u8 shift1;  // distance to left shift right-justified unit1 bits to reach dest
    u8 idx2;    // index of unit2, or 0 if there is no unit2
    u8 len2;    // number of bits for unit2, or 0 if there is no unit2
    u8 shift2;  // distance to left shift right-justified unit2 bits, or 0 if there is no unit2
    u8 pos, len;

    BitsRef(u32 p, u32 l) ;

    u32 Read(const T & stg) const ;

    void Write(T & stg, u32 val) const;

    u64 ReadLong(const T & stg) const ;

    void WriteLong(T & stg, u64 val) const;

    u32 GetPos() const { return pos; }
 
    u32 GetLen() const { return len; }
  };
} //MFM


namespace MFM {
  /**
   * A UlamRef is a base class for ulam reference variables
   */
  template <class EC>
  class UlamRef
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    const UlamClass<EC> * m_effSelf;
    T & m_stg;
    BitsRef<EC> m_ref;

  public:

    /**
       Construct an UlamRef 'from scratch'
     */
    UlamRef(u32 pos, u32 len, T& stg, UlamClass<EC> * effself) ;

    /**
       Construct an UlamRef that's relative to an existing UlamRef.
       The 'pos' supplied here will be relative to the the existing
       pos, and this pos + len must fit within the len supplied to the
       existing UlamRef.
     */
    UlamRef(UlamRef & existing, u32 pos, u32 len, UlamClass<EC> * effself) ;

    u32 Read() const { return m_ref.Read(m_stg); }

    void Write(u32 val) { m_ref.Write(m_stg, val); }

    u64 ReadLong() const { return m_ref.ReadLong(m_stg); }

    void WriteLong(u64 val) { m_ref.WriteLong(m_stg, val); }

    u32 GetPos() const { return m_ref.GetPos(); }
 
    u32 GetLen() const { return m_ref.GetLen(); }

  };

} // MFM

#include "UlamRef.tcc"

#endif /* ULAMREF_H */
