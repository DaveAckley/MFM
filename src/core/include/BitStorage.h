/*                                              -*- mode:C++ -*-
  BitStorage.h An interface for accessing bits stored in arbitrary-sized groups
  Copyright (C) 2016-2017 The Regents of the University of New Mexico.  All rights reserved.

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
  \file BitStorage.h An interface for accessing bits stored in arbitrary-sized groups
  \author David H. Ackley.
  \date (C) 2016-2017 All rights reserved.
  \lgpl
 */
#ifndef BITSTORAGE_H
#define BITSTORAGE_H

#include "BitVector.h"

namespace MFM
{

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

    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    BitStorage() { }

    virtual ~BitStorage() { }

    virtual u32 Read(u32 pos, u32 len) const = 0;

    virtual void Write(u32 pos, u32 len, u32 val) = 0;

    virtual u64 ReadLong(u32 pos, u32 len) const = 0;

    virtual void WriteLong(u32 pos, u32 len, u64 val) = 0;

    /**
       ReadBig: read len bits starting at index \c pos; return them
                left-justified at position 0..len-1.  Fails if pos +
                len > size of this storage.  Fails if len > 96.
    */
    virtual BV96 ReadBig(u32 pos, u32 len) const = 0;

    /**
       WriteBig: read len bits starting at position 0 in val; write
                len bits into this storage starting from index \c pos;
                Fails if pos + len > size of this storage.  Fails if
                len > 96.
    */
    virtual void WriteBig(u32 pos, u32 len, const BV96& val) = 0;

    /**
       Copy \c LEN bits from positions \c pos through \c pos + LEN - 1
       of \c this to positions 0 through LEN - 1 of \c rtnbv
    */
    template<u32 LEN>
    void ReadBV(u32 pos, BitVector<LEN>& rtnbv) const
    {
      u32 amt = 32;
      for (u32 i = 0; i < LEN; i += amt)
      {
        if (i + amt > LEN) amt = LEN - i;
        rtnbv.Write(i, amt, this->Read(pos + i, amt));
      }
    }

    /**
       Copy \c LEN bits from positions 0 through LEN - 1 of \c val
       to positions \c pos through \c pos + LEN - 1 of \c this.
    */
    template<u32 LEN>
    void WriteBV(u32 pos, const BitVector<LEN>& val)
    {
      u32 amt = 32;
      for (u32 i = 0; i < LEN; i += amt)
      {
        if (i + amt > LEN) amt = LEN - i;
        this->Write(pos + i, amt, val.Read(i, amt));
      }
    }

    /**
	ReadAtom: returns a formatted atom, length T::BPA
     */
    virtual T ReadAtom(u32 pos) const = 0;

    /**
	WritesAtom: writes a formatted atom, length T::BPA
     */
    virtual void WriteAtom(u32 pos, const T& val) = 0;

    virtual u32 GetBitSize() const = 0;

    virtual const char * GetUlamTypeMangledName() const = 0;
  }; //BitStorage (pure)

  /**
     A BitStorage for an arbitrary BitVector BV
   */
  template <class EC, class BV>
  struct BitVectorBitStorage : public BitStorage<EC> {

    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    BitVectorBitStorage(const BV & toCopy) : m_stg(toCopy) { }

    BitVectorBitStorage(const u32 * const values) : m_stg(values) { }

    BitVectorBitStorage() { }

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

    virtual BV96 ReadBig(u32 pos, u32 len) const
    {
      return m_stg.ReadBig(pos, len);
     }

    virtual void WriteBig(u32 pos, u32 len, const BV96& val)
     {
      m_stg.WriteBig(pos, len, val);
     }

    virtual T ReadAtom(u32 pos) const
    {
      T tmpt;
      tmpt.GetBits() = m_stg.ReadBig(pos, T::BPA);
      return tmpt;
    }

    virtual void WriteAtom(u32 pos, const T& tval)
    {
      m_stg.WriteBig(pos, T::BPA, tval.GetBits());
    }

    virtual u32 GetBitSize() const
    {
      return BV::BITS;
    }

    virtual const char * GetUlamTypeMangledName() const
    {
      FAIL(ILLEGAL_STATE);
      return NULL;
    }

  }; //BitVectorBitStorage

  /**
     A BitStorage for single existing T&
   */
  template <class EC>
  struct AtomRefBitStorage : public BitStorage<EC> {

    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    explicit AtomRefBitStorage(T & toModify) : m_stg(toModify) { }

  private:
    AtomRefBitStorage(const AtomRefBitStorage<EC> & toCopy) ;  // Declare away

  public:

    T& m_stg;

    virtual u32 Read(u32 pos, u32 len) const
    {
      return m_stg.GetBits().Read(pos, len);
    }

    virtual void Write(u32 pos, u32 len, u32 val)
    {
      m_stg.GetBits().Write(pos, len, val);
    }

    virtual u64 ReadLong(u32 pos, u32 len) const
    {
      return m_stg.GetBits().ReadLong(pos, len);
    }

    virtual void WriteLong(u32 pos, u32 len, u64 val)
    {
      m_stg.GetBits().WriteLong(pos, len, val);
    }

    virtual BV96 ReadBig(u32 pos, u32 len) const
    {
      return m_stg.GetBits().ReadBig(pos, len);
    }

    virtual void WriteBig(u32 pos, u32 len, const BV96& val)
    {
      m_stg.GetBits().WriteBig(pos, len, val);
    }

    virtual T ReadAtom(u32 pos) const
    {
      return ReadAtom();
    }

    virtual void WriteAtom(u32 pos, const T& tval)
    {
      WriteAtom(tval);
    }

    virtual u32 GetBitSize() const
    {
      return T::BPA;
    }

    u32 GetType() { return m_stg.GetType(); }

    T ReadAtom() const { return m_stg; } //a copy

    void WriteAtom(const T& tval) { m_stg = tval; }

    virtual const char * GetUlamTypeMangledName() const
    {
      FAIL(ILLEGAL_STATE);
      return NULL;
    }

  }; //AtomRefBitStorage

  /**
     A BitStorage for a single atom
   */
  template <class EC>
  struct AtomBitStorage : public AtomRefBitStorage<EC> {

    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    AtomBitStorage(const T & toCopy) : AtomRefBitStorage<EC>(m_atom), m_atom(toCopy) { }

    AtomBitStorage() : AtomRefBitStorage<EC>(m_atom) { }

    T m_atom;

    const T& GetAtom() const { return m_atom; }

  private:
    AtomBitStorage(const AtomBitStorage<EC> & toCopyABS); //declare away

  }; //AtomBitStorage

} // MFM

#include "BitStorage.tcc"

#endif /* BITSTORAGE_H */
