/*                                              -*- mode:C++ -*-
  P1Atom.h A partial reimplementation, now deprecated, of the original MFM atom
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
  \file P1Atom.h A partial reimplementation, now deprecated, of the original MFM atom
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef P1ATOM_H
#define P1ATOM_H

#include <stdio.h>
#include "itype.h"
#include "Point.h"
#include "BitVector.h"
#include "MDist.h"
#include "Atom.h"
#include "Element.h"
#include "Element_Empty.h"
#include "CoreConfig.h"
#include "ParamConfig.h"

#define P1ATOM_HEADER_SIZE 16
#define P1ATOM_LONG_BOND_SIZE 8
#define P1ATOM_SHORT_BOND_SIZE 4


namespace MFM {

  template <class PC>
  class P1Atom : public Atom< CoreConfig< P1Atom<PC>, PC> >
  {
  public: enum { ATOM_CATEGORY = 1 };
  private:

    enum {
      BITS = 64,
      // For now we insist on exact match.  Possibly longer could be
      // supported relatively easily.
      CONFIGURED_BITS_PER_ATOM_IS_INCOMPATIBLE_WITH_P1ATOM = 1/((PC::BITS_PER_ATOM==BITS)?1:0)
    };

  protected:

    typedef MDist<4> MDist4;
    typedef CoreConfig< P1Atom<PC>, PC> CC;

    /* We really don't want to allow the public to change the type of a
       p1atom, since the type doesn't mean much without the atomic
       header as well */

    void SetType(u32 type, u32 width)
    {
      this->m_bits.Write(BITS-width, width, type);
    }

  public:

    bool IsSane() const
    {
      return GetBitsAllocated() <= BITS;
    }

    bool HasBeenRepaired()
    {
      // We can't fix an insane atom.
      return false;
    }


    u32 GetBitCount() const {
      return BITS;
    }

    u32 GetType() const {
      s32 bitsUsed = GetBitsAllocated();
      if (bitsUsed > BITS)
        FAIL(ILLEGAL_STATE);

      // The up-to 31 bits left over after everything else are the type bits
      u32 typeBits = BITS-bitsUsed;
      if (typeBits > 31) typeBits = 31;
      return this->m_bits.Read(BITS-typeBits,typeBits);
    }

    void WriteStateBits(ByteSink& ostream) const
    {
      s32 bitsUsed = GetBitsAllocated();

      u32 typeBits = BITS - bitsUsed;
      if(typeBits > 31) typeBits = 31;

      u32 leftoverBits = BITS - typeBits;

      for(u32 i = 0; i < leftoverBits; i++)
      {
	ostream.Printf("%d", this->m_bits.ReadBit(i) ? 1 : 0);
      }
    }

    void ReadStateBits(const char* stateStr)
    {
      u32 len = strlen(stateStr);

      for(u32 i = 0; i < len; i++)
      {
	this->m_bits.WriteBit(i, stateStr[i] == '0' ? 0 : 1);
      }
    }

    void ReadStateBits(const BitVector<BITS> & bv)
    {
      s32 bitsUsed = GetBitsAllocated();

      u32 typeBits = BITS - bitsUsed;
      if(typeBits > 31) typeBits = 31;

      u32 leftoverBits = BITS - typeBits;

      for(u32 i = 0; i < leftoverBits; i++)
      {
	this->m_bits.WriteBit(i, bv.ReadBit(i));
      }
    }


    P1Atom()
    {
      InitAtom(0,0,0,32);
    }

    P1Atom(u32 type, u32 longc, u32 shortc, u32 statec)
    {
      InitAtom(type,longc,shortc,statec);
    }

    void InitAtom(u32 type, u32 longc, u32 shortc, u32 statec)
    {
      FAIL(DEPRECATED);

      SetLongBondCount(longc);
      SetShortBondCount(shortc);
      SetStateBitCount(statec);
      u32 used = GetBitsAllocated();

      if (used > BITS)
        FAIL(ILLEGAL_ARGUMENT);

      u32 avail = BITS - used;
      if (avail > 31) avail = 31;

      if (type >= 1u<<avail)
        FAIL(ILLEGAL_ARGUMENT);
      SetType(type, avail);

      if (statec > 32) {
        SetStateField(32,statec-32,0);
        statec = 32;
      }
      SetStateField(0,statec,0);

    }

    u32 GetBitsAllocated() const
    {
      return
        P1ATOM_HEADER_SIZE+
        GetLongBondCount()*P1ATOM_LONG_BOND_SIZE
        + GetShortBondCount()*P1ATOM_SHORT_BOND_SIZE
        + GetStateBitCount();
    }

    u32 GetBitsAllocatedExceptState() const
    {
      return
        P1ATOM_HEADER_SIZE+
        GetLongBondCount()*P1ATOM_LONG_BOND_SIZE
        + GetShortBondCount()*P1ATOM_SHORT_BOND_SIZE;
    }

    u32 GetStateBitCount() const
    { return this->m_bits.Read(2, 6); }

    void SetStateBitCount(u32 state)
    { this->m_bits.Write(2, 6, state); }

    /**
     * Read stateWidth state bits starting at stateIndex, which counts
     * toward the right with 0 meaning the leftmost state bit.
     */
    u32 GetStateField(u32 stateIndex, u32 stateWidth) const
    {
      // The state bits end at the end of the 'allocated bits'
      u32 firstState = GetBitsAllocatedExceptState();
      return this->m_bits.Read(firstState+stateIndex, stateWidth);
    }

    /**
     * Store value into stateWidth state bits starting at stateIndex,
     * which counts toward the right with 0 meaning the leftmost state
     * bit.
     */
    void SetStateField(u32 stateIndex, u32 stateWidth, u32 value)
    {
      u32 firstState = GetBitsAllocatedExceptState();
      return this->m_bits.Write(firstState+stateIndex, stateWidth, value);
    }

    u32 GetLongBondCount() const
    { return this->m_bits.Read(9, 3); }

    void SetLongBondCount(u32 count)
    { this->m_bits.Write(9, 3, count); }

    u32 GetShortBondCount() const
    { return this->m_bits.Read(12, 4); }

    void SetShortBondCount(u32 count)
    { this->m_bits.Write(12, 4, count); }

    u32 GetLongBond(u32 index) const {
      if (index>=GetLongBondCount()) return 0;
      return this->m_bits.Read(16+index*8,8);
    }
    void SetLongBond(u32 index, u8 value) {
      if (index>=GetLongBondCount()) return;
      return this->m_bits.Write(16+index*8,8,(u32) value);
    }

    u32 GetShortBond(u32 index) const {
      if (index>=GetShortBondCount()) return 0;
      u32 start = GetLongBondCount()*8+16;
      return this->m_bits.Read(start+index*4,4);
    }
    void SetShortBond(u32 index, u8 value) {
      if (index>=GetLongBondCount()) return;
      u32 start = GetLongBondCount()*8+16;
      return this->m_bits.Write(start+index*4,4,(u32) value);
    }

    void ReadVariableBodyInto(u32* arr) const
    {
      arr[0] = this->m_bits.Read(16, 16);
      arr[1] = this->m_bits.Read(32, 32);
    }

    void WriteVariableBodyFrom(u32* arr)
    {
      this->m_bits.Write(16, 16, arr[0]);
      this->m_bits.Write(32, 32, arr[1]);
    }

    void PrintBits(ByteSink & ostream) const
    { this->m_bits.Print(ostream); }

    void Print(ByteSink & ostream) const
    {
      u32 type = GetType();
      u32 lbc = GetLongBondCount();
      u32 sbc = GetShortBondCount();
      u32 stc = GetStateBitCount();
      u32 state = GetStateField(0,stc>32?32:stc);
      ostream.Printf("P1[%x/%d%d/%d:%x]", type,lbc,sbc,stc,state);
    }

#if 0
    /* Adds a long bond. Returns its index. */
    u32 AddLongBond(const SPoint& offset);

    u32 AddShortBond(const SPoint& offset);
#endif

    /**
     * Fills pt with the long bond location in index and returns true,
     * if atom has at least index+1 long bonds.  Otherwise returns false
     * and pt is unchanged */
    bool GetLongBond(u32 index, SPoint& pt) const;

    /**
     * Fills pt with the short bond location in index and returns true,
     * if atom has at least index+1 short bonds.  Otherwise returns
     * false and pt is unchanged */
    bool GetShortBond(u32 index, SPoint& pt) const;

    /**
     * Stores pt in the long bond location index and returns true, if
     * atom has at least index+1 long bonds, and pt is representable as
     * a long bond.  Otherwise returns false and *this is unchanged */
    bool SetLongBond(u32 index, const SPoint& pt);

    /**
     * Stores pt in the short bond location in index and returns true,
     * if atom has at least index+1 short bonds, and pt is representable
     * as a short bond.  Otherwise returns false and *this is unchanged */
    bool SetShortBond(u32 index, const SPoint& pt);

    /*
     * Removes a long bond. Be careful; if a
     * bond is removed, the bonds ahead of it
     * (if they exist) will be pushed downwards.
     * The indices of these bonds will need to
     * be updated again afterwards.
     */
    void RemoveLongBond(u32 index);

    void RemoveShortBond(u32 index);

    P1Atom& operator=(const P1Atom & rhs);
  };
} /* namespace MFM */

#include "P1Atom.tcc"

#endif /*P1ATOM_H*/
