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
  struct BitRef
  {

    typedef typename EC::ATOM_CONFIG AC;
    enum { BITS_PER_ATOM = AC::BITS_PER_ATOM };
    typedef typename AC::ATOM_TYPE T;
    typedef BitVector<T::BITS> BV;

    u32 mask1;  // mask for unit1 bits left-shifted by shift1
    u32 mask2;  // mask for unit2 bits (if any) left-shifted by shift2
    u8 idx;     // index of unit1
    u8 len1;    // number of bits for unit1
    u8 shift1;  // distance to left shift right-justified unit1 bits to reach dest
    u8 len2;    // number of bits for unit2, or 0 if there is no unit2
    u8 shift2;  // distance to left shift right-justified unit2 bits, or 0 if there is no unit2
    u8 pos;     // absolute starting bit index
    u8 len;     // total bit length.  Note that if len1 + len2 < len
                // (which is only relevant for ReadLong/WriteLong),
                // then three units, not just two, are involved.  In
                // that case, all of unit[idx+1] is used, and len2,
                // shift2, and mask2 apply to unit[idx+2]

    BitRef(u32 p, u32 l) ;

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
     A base class for virtualized (rather than templatized) access to
     bit storage
   */
  template <class EC>
  struct BitStorage {

    virtual ~BitStorage() { }

    virtual u32 Read(u32 pos, u32 len) const = 0;

    virtual void Write(u32 pos, u32 len, u32 val) = 0;

    virtual u64 ReadLong(u32 pos, u32 len) const = 0;

    virtual void WriteLong(u32 pos, u32 len, u64 val) = 0;

    virtual u32 GetBitSize() const = 0;
  };

  /**
     A BitStorage for a single atom
   */
  template <class EC>
  struct AtomBitStorage : public BitStorage<EC> {
    
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    AtomBitStorage(const T & toCopy) : m_stg(toCopy) { }

    AtomBitStorage() { }

    enum { POS_ORIGIN = T::ATOM_FIRST_STATE_BIT };

    T m_stg;

    virtual u32 Read(u32 pos, u32 len) const 
    {
      return m_stg.GetBits().Read(pos + POS_ORIGIN, len);
    }

    virtual void Write(u32 pos, u32 len, u32 val) 
    {
      m_stg.GetBits().Write(pos + POS_ORIGIN, len, val);
    }

    virtual u64 ReadLong(u32 pos, u32 len) const 
    {
      return m_stg.GetBits().ReadLong(pos + POS_ORIGIN, len);
    }

    virtual void WriteLong(u32 pos, u32 len, u64 val) 
    {
      m_stg.GetBits().WriteLong(pos + POS_ORIGIN, len, val);
    }

    virtual u32 GetBitSize() const 
    {
      return T::BPA;
    }

    T ReadAtom() const { return m_stg; } //a copy

    void WriteAtom(const T& tval) { m_stg = tval; }
  };

  /**
     A BitStorage for an arbitrary BitVector BV
   */
  template <class EC, class BV>
  struct BitVectorStorage : public BitStorage<EC> {

    BitVectorStorage(const BV & toCopy) : m_stg(toCopy) { }

    BitVectorStorage() { }

    BV m_stg;

    virtual u32 Read(u32 pos, u32 len) const 
    {
      return m_stg.Read(pos, len);
    }

    virtual void Write(u32 pos, u32 len, u32 val) 
    {
      m_stg.Write(pos, len, val);
    }

    virtual u64 ReadLong(u32 pos, u32 len) const 
    {
      return m_stg.ReadLong(pos, len);
    }

    virtual void WriteLong(u32 pos, u32 len, u64 val) 
    {
      m_stg.WriteLong(pos, len, val);
    }

    virtual u32 GetBitSize() const 
    {
      return BV::BITS;
    }
  };

  /**
   * A UlamRef is a base class for ulam reference variables
   */
  template <class EC>
  class UlamRef
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { POS_ORIGIN = T::ATOM_FIRST_STATE_BIT };

    const UlamClass<EC> * m_effSelf;
    BitStorage<EC> & m_stg;
    u32 m_pos;
    u32 m_len;

  public:

    /**
       Construct an UlamRef 'from scratch'
     */
    UlamRef(u32 pos, u32 len, BitStorage<EC>& stg, const UlamClass<EC> * effself) ;

    /**
       Construct an UlamRef that's relative to an existing UlamRef.
       The 'pos' supplied here will be relative to the the existing
       pos, and this pos + len must fit within the len supplied to the
       existing UlamRef.
     */
    UlamRef(const UlamRef<EC> & existing, u32 pos, u32 len, const UlamClass<EC> * effself) ;

    u32 Read() const { return m_stg.Read(m_pos, m_len); }

    void Write(u32 val) { m_stg.Write(m_pos, m_len, val); }

    u64 ReadLong() const { return m_stg.ReadLong(m_pos, m_len); }

    void WriteLong(u64 val) { m_stg.WriteLong(m_pos, m_len, val); }

    u32 GetPos() const { return m_pos; }

    u32 GetLen() const { return m_len; }

    u32 GetType() const { return m_stg.GetType(); }

    const UlamClass<EC> * GetEffectiveSelf() const { return m_effSelf; }

    BitStorage<EC> & GetStorage() { return m_stg; }
  };

  template <class EC, u32 OFFSET, u32 LEN>
  struct UlamRefFixed : public UlamRef<EC>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    UlamRefFixed(BitStorage<EC>& stg, const UlamClass<EC> * effself)
      : UlamRef<EC>(OFFSET, LEN, stg, effself)
    { }

    UlamRefFixed(const UlamRef<EC>& parent, const UlamClass<EC> * effself)
      : UlamRef<EC>(parent, OFFSET, LEN, effself)
    { }
  };

  template <class EC>
  struct UlamRefAtom : public UlamRefFixed<EC, 0, EC::ATOM_CONFIG::BITS_PER_ATOM - EC::ATOM_CONFIG::ATOM_TYPE::ATOM_FIRST_STATE_BIT>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    typedef UlamRefFixed<EC, 0, BPA - T::ATOM_FIRST_STATE_BIT> Super;

    UlamRefAtom(BitStorage<EC>& stg, const UlamClass<EC> * effself)
      : Super(stg, effself)
    { }

    UlamRefAtom(const UlamRefAtom<EC>& existing, const UlamClass<EC> * effself)
      : Super(existing, effself)
    { }

  };


} // MFM

#include "UlamRef.tcc"

#endif /* ULAMREF_H */
