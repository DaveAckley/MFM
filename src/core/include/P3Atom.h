/*                                              -*- mode:C++ -*-
  P3Atom.h Atom with built in error correcting
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
  \file P3Atom.h Atom with built in error correcting
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef P3ATOM_H
#define P3ATOM_H

#include <stdio.h>
#include "itype.h"
#include "Point.h"
#include "BitField.h"
#include "Atom.h"
#include "Element.h"
#include "CoreConfig.h"
#include "ParamConfig.h"
#include "Util.h"      /* For COMPILATION_REQUIREMENT */
#include "Parity2D_4x4.h"

namespace MFM {

  template <class PC>
  class P3Atom : public Atom< CoreConfig< P3Atom<PC>, PC> >
  {
  public: enum
  {
    ATOM_CATEGORY = 3
  };
  private:
    typedef CoreConfig< P3Atom<PC>, PC> CC;

  public:
    enum {
      BITS = PC::BITS_PER_ATOM,

      //////
      // P3 header configuration: Header is in low-bits end of the bitvector

      P3_ECC_BITS_POS = 0,
      P3_ECC_BITS_LEN = 9,

      P3_TYPE_BITS_POS = P3_ECC_BITS_POS + P3_ECC_BITS_LEN,
      P3_TYPE_BITS_LEN = 16,

      P3_FIXED_HEADER_POS = P3_ECC_BITS_POS,
      P3_FIXED_HEADER_LEN = P3_ECC_BITS_LEN + P3_TYPE_BITS_LEN,

      P3_STATE_BITS_POS = P3_FIXED_HEADER_POS + P3_FIXED_HEADER_LEN,
      P3_STATE_BITS_LEN = BITS - P3_STATE_BITS_POS,

      //////
      // Declarations required by the Atom contract
      ATOM_FIRST_STATE_BIT = P3_STATE_BITS_POS,

      //////
      // Other constants
      P3_TYPE_COUNT = 1<<P3_TYPE_BITS_LEN

    };

    typedef BitField<BitVector<BITS>,VD::U32,P3_FIXED_HEADER_LEN,P3_FIXED_HEADER_POS> AFFixedHeader;
    typedef BitField<BitVector<BITS>,VD::U32,P3_TYPE_BITS_LEN,P3_TYPE_BITS_POS> AFTypeBits;
    typedef BitField<BitVector<BITS>,VD::U32,P3_ECC_BITS_LEN,P3_ECC_BITS_POS> AFECCBits;

  protected:

    /* We really don't want to allow the public to change the type of a
       P3Atom, since the type doesn't mean much without the atomic
       header as well */

    void SetType(u32 type) {

      if (type >= P3_TYPE_COUNT)
        FAIL(ILLEGAL_ARGUMENT);

      // Generate ECC and store all in header
      AFFixedHeader::Write(this->m_bits,Parity2D_4x4::Add2DParity(type));
    }

  public:

    //    P3Atom(u32 type = Element_Empty<CC>::THE_INSTANCE.GetType(), u32 z1 = 0, u32 z2 = 0, u32 stateBits = 0)
    P3Atom(u32 type = 0, u32 z1 = 0, u32 z2 = 0, u32 stateBits = 0)
    {
      COMPILATION_REQUIREMENT< 32 <= BITS-1 >();

      if (z1 != 0 || z2 != 0)
        FAIL(ILLEGAL_ARGUMENT);

      if (stateBits > P3_STATE_BITS_LEN)
        FAIL(OUT_OF_ROOM);

      SetType(type);
    }

    u32 GetTypeImpl() const {
      return AFTypeBits::Read(this->m_bits);
    }

    bool IsSaneImpl() const
    {
      u32 fixedHeader = AFFixedHeader::Read(this->m_bits);
      return Parity2D_4x4::Check2DParity(fixedHeader);
    }

    bool HasBeenRepairedImpl()
    {
      u32 fixedHeader = AFFixedHeader::Read(this->m_bits);
      u32 repairedHeader =
        Parity2D_4x4::Correct2DParityIfPossible(fixedHeader);

      if (repairedHeader == 0) return false;

      if (fixedHeader != repairedHeader)
      {
        AFFixedHeader::Write(this->m_bits, repairedHeader);
      }

      return true;
    }


    u32 GetMaxStateSize(u32 type) const {
      return P3_STATE_BITS_LEN;
    }

    /**
     * Index of first bit that isn't a state bit.
     */
    u32 EndStateBit() const
    {
      return BITS;
    }

    void WriteStateBitsImpl(ByteSink& ostream) const
    {
      for(u32 i = P3_STATE_BITS_POS; i < P3_STATE_BITS_POS + P3_STATE_BITS_LEN; i++)
      {
	ostream.Printf("%d", this->m_bits.ReadBit(i) ? 1 : 0);
      }
    }

    void ReadStateBitsImpl(const char* stateStr)
    {
      for(u32 i = 0; i < P3_STATE_BITS_LEN; i++)
      {
	this->m_bits.WriteBit(P3_STATE_BITS_POS + i, stateStr[i] == '0' ? 0 : 1);
      }
    }

    void ReadStateBitsImpl(const BitVector<BITS> & bv)
    {
      for(u32 i = 0; i < P3_STATE_BITS_LEN; i++)
      {
        u32 idx = P3_STATE_BITS_POS + i;
	this->m_bits.WriteBit(idx, bv.ReadBit(idx));
      }
    }

    /**
     * Read stateWidth state bits starting at stateIndex, which counts
     * toward the right with 0 meaning the leftmost state bit.
     */
    u32 GetStateField(u32 stateIndex, u32 stateWidth) const
    {
      if (stateWidth > P3_STATE_BITS_LEN)
        FAIL(ILLEGAL_ARGUMENT);
      return this->m_bits.Read(P3_STATE_BITS_POS + stateIndex, stateWidth);
    }

    /**
     * Store value into stateWidth state bits starting at stateIndex,
     * which counts toward the right with 0 meaning the leftmost state
     * bit.
     */
    void SetStateField(u32 stateIndex, u32 stateWidth, u32 value)
    {
      if (stateWidth > P3_STATE_BITS_LEN)
        FAIL(ILLEGAL_ARGUMENT);
      return this->m_bits.Write(P3_STATE_BITS_POS + stateIndex, stateWidth, value);
    }

    void PrintBits(ByteSink & ostream) const
    { this->m_bits.Print(ostream); }

    void PrintImpl(ByteSink & ostream) const
    {
      u32 type = this->GetType();
      ostream.Printf("P3[%x/",type);
      u32 length = GetMaxStateSize(type);
      for (u32 i = 0; i < length; i += 4) {
        u32 nyb = this->GetStateField(i,4);
        ostream.Printf("%x",nyb);
      }
      ostream.Printf("]");
    }

    P3Atom& operator=(const P3Atom & rhs)
    {
      if (this == &rhs) return *this;

      this->m_bits = rhs.m_bits;

      return *this;
    }

  };
} /* namespace MFM */

#endif /*P3ATOM_H*/
