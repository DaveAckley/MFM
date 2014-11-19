/*                                              -*- mode:C++ -*-
  P0Atom.h Simplest of the MFM Atoms
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
  \file P0Atom.h Simplest of the MFM Atoms
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef P0ATOM_H
#define P0ATOM_H

#include <stdio.h>
#include "itype.h"
#include "Point.h"
#include "BitField.h"
#include "Atom.h"
#include "Element.h"
#include "CoreConfig.h"
#include "ParamConfig.h"

#define P0ATOM_HEADER_LENGTH_SIZE 2
#define P0ATOM_TYPE_WIDTH_INCREMENT 12
#define P0ATOM_STATE_SIZE 48

namespace MFM
{

  template <class PC>
  class P0Atom : public Atom< CoreConfig< P0Atom<PC>, PC> >
  {
  public:
    enum
    {
      ATOM_CATEGORY = 0,
      ATOM_FIRST_STATE_BIT = 0
    };
  private:

    typedef CoreConfig< P0Atom<PC>, PC> CC;
    typedef typename CC::PARAM_CONFIG P;
    enum { BPA = P::BITS_PER_ATOM };

    enum
    {
      BITS = 64,
      // For now we insist on exact match.  Possibly longer could be
      // supported relatively easily.
      CONFIGURED_BITS_PER_ATOM_IS_INCOMPATIBLE_WITH_P0ATOM = 1/((PC::BITS_PER_ATOM==BITS)?1:0)
    };

    enum
    {
      TYPE0_LENGTH = 0*P0ATOM_TYPE_WIDTH_INCREMENT,
      TYPE1_LENGTH = 1*P0ATOM_TYPE_WIDTH_INCREMENT,
      TYPE2_LENGTH = 2*P0ATOM_TYPE_WIDTH_INCREMENT,
      TYPE3_LENGTH = 28
    };
    // Type info at end of atom so state bits start at a compile-time known bit position (0)
    typedef BitField<BitVector<BITS>,VD::U32,P0ATOM_HEADER_LENGTH_SIZE,BITS - P0ATOM_HEADER_LENGTH_SIZE> AFTypeLengthCode;

    // These BitFields represent just the extended type information, exclusive of the length code
    typedef BitField<BitVector<BITS>,VD::U32,TYPE0_LENGTH, AFTypeLengthCode::START - TYPE0_LENGTH> AFTypeLength0;
    typedef BitField<BitVector<BITS>,VD::U32,TYPE1_LENGTH, AFTypeLengthCode::START - TYPE1_LENGTH> AFTypeLength1;
    typedef BitField<BitVector<BITS>,VD::U32,TYPE2_LENGTH, AFTypeLengthCode::START - TYPE2_LENGTH> AFTypeLength2;
    typedef BitField<BitVector<BITS>,VD::U32,TYPE3_LENGTH, AFTypeLengthCode::START - TYPE3_LENGTH> AFTypeLength3;

    // These BitFields represent the entire type information, including the length code
    typedef BitField<BitVector<BITS>,VD::U32,TYPE0_LENGTH + P0ATOM_HEADER_LENGTH_SIZE, AFTypeLength0::START> AFType0;
    typedef BitField<BitVector<BITS>,VD::U32,TYPE1_LENGTH + P0ATOM_HEADER_LENGTH_SIZE, AFTypeLength1::START> AFType1;
    typedef BitField<BitVector<BITS>,VD::U32,TYPE2_LENGTH + P0ATOM_HEADER_LENGTH_SIZE, AFTypeLength2::START> AFType2;
    typedef BitField<BitVector<BITS>,VD::U32,TYPE3_LENGTH + P0ATOM_HEADER_LENGTH_SIZE, AFTypeLength3::START> AFType3;

  protected:

    /* We really don't want to allow the public to change the type of a
       P0Atom, since the type doesn't mean much without the atomic
       header as well */

    void SetType(u32 lengthCode, u32 type)
    {
      AFTypeLengthCode::Write(this->m_bits,lengthCode);
      switch (lengthCode)
      {
      case 0: AFTypeLength0::Write(this->m_bits,type); break;
      case 1: AFTypeLength1::Write(this->m_bits,type); break;
      case 2: AFTypeLength2::Write(this->m_bits,type); break;
      case 3: AFTypeLength3::Write(this->m_bits,type); break;
      default:
        FAIL(ILLEGAL_ARGUMENT);
      }
    }

    u32 GetLengthCodeForType(u32 type)
    {
      if ((1 << TYPE0_LENGTH) > type) return 0;
      if ((1 << TYPE1_LENGTH) > type) return 1;
      if ((1 << TYPE2_LENGTH) > type) return 2;
      if ((1 << TYPE3_LENGTH) > type) return 3;
      FAIL(ILLEGAL_ARGUMENT);
    }

    /**
     * Index of first bit that isn't a state bit.
     */
    u32 EndStateBit() const
    {
      return EndStateBit(AFTypeLengthCode::Read(this->m_bits));
    }

    static u32 EndStateBit(u32 lengthCode)
    {
      switch(lengthCode)
      {
      case 0:
	return AFTypeLength0::START;
      case 1:
	return AFTypeLength1::START;
      case 2:
	return AFTypeLength2::START;
      case 3:
	return AFTypeLength3::START;
      default:
	FAIL(ILLEGAL_ARGUMENT);
      }
    }

  public:

    bool IsSaneImpl() const
    {
      // P0Atom has no error detection abilities
      return true;
    }

    bool HasBeenRepairedImpl()
    {
      // If somebody thinks it's bad, we can't fix it.
      return false;
    }

    u32 GetTypeImpl() const
    {
      u32 lengthCode = AFTypeLengthCode::Read(this->m_bits);
      switch (lengthCode) {
      case 0: return AFTypeLength0::Read(this->m_bits);
      case 1: return AFTypeLength1::Read(this->m_bits);
      case 2: return AFTypeLength2::Read(this->m_bits);
      case 3: return AFTypeLength3::Read(this->m_bits);
      default:
        FAIL(UNREACHABLE_CODE);
      }
    }

    /**
     * Read stateWidth state bits starting at stateIndex, which counts
     * toward the right with 0 meaning the leftmost state bit.
     */
    u32 GetStateField(u32 stateIndex, u32 stateWidth) const
    {
      const u32 lastState = EndStateBit();
      const u32 firstState = ATOM_FIRST_STATE_BIT + stateIndex;

      if (firstState + stateWidth >= lastState)
        FAIL(ILLEGAL_ARGUMENT);

      return this->m_bits.Read(firstState, stateWidth);
    }

    /**
     * Store value into stateWidth state bits starting at stateIndex,
     * which counts toward the right with 0 meaning the leftmost state
     * bit.
     */
    void SetStateField(u32 stateIndex, u32 stateWidth, u32 value)
    {
      const u32 lastState = EndStateBit();
      const u32 firstState = ATOM_FIRST_STATE_BIT + stateIndex;

      if (firstState + stateWidth >= lastState)
        FAIL(ILLEGAL_ARGUMENT);

      return this->m_bits.Write(firstState, stateWidth, value);
    }

    void WriteStateBitsImpl(ByteSink& ostream) const
    {
      u32 endStateBit = EndStateBit();

      for(u32 i = ATOM_FIRST_STATE_BIT; i < endStateBit; i++)
      {
	ostream.Printf("%d", this->m_bits.ReadBit(i) ? 1 : 0);
      }
    }

    void ReadStateBitsImpl(const char* stateStr)
    {
      u32 endStateBit = EndStateBit();

      for(u32 i = ATOM_FIRST_STATE_BIT; i < endStateBit; i++)
      {
	this->m_bits.WriteBit(i, stateStr[i] == '0' ? 0 : 1);
      }
    }

    void ReadStateBitsImpl(const BitVector<BITS> & bv)
    {
      u32 endStateBit = EndStateBit();

      for(u32 i = ATOM_FIRST_STATE_BIT; i < endStateBit; i++)
      {
	this->m_bits.WriteBit(i, bv.ReadBit(i));
      }
    }

    static u32 GetMaxStateSize(u32 lengthCode) {
      return EndStateBit(lengthCode);
    }

    P0Atom(u32 type = 0, u32 longc = 0, u32 shortc = 0, u32 statec = 0)
    {
      InitAtom(type,longc,shortc,statec);
    }

    void InitAtom(u32 type, u32 longc, u32 shortc, u32 statec)
    {
      if (longc != 0)
        FAIL(ILLEGAL_ARGUMENT);
      if (shortc != 0)
        FAIL(ILLEGAL_ARGUMENT);

      u32 lengthCode =
        GetLengthCodeForType(type);
      u32 maxState =
        GetMaxStateSize(lengthCode);

      if (statec > maxState)
        FAIL(ILLEGAL_ARGUMENT);

      SetType(lengthCode, type);

    }

    void PrintBits(ByteSink & ostream) const
    { this->m_bits.Print(ostream); }

    void PrintImpl(ByteSink & ostream) const
    {
      u32 lengthCode = AFTypeLengthCode::Read();
      u32 type = this->GetType();
      ostream.Printf("P0[%x/",type);
      u32 length = GetMaxStateSize(lengthCode);
      for (int i = 0; i < length; i += 4) {
        u32 nyb = this->m_bits.Read(ATOM_FIRST_STATE_BIT+i,4);
        ostream.Printf("%x",nyb);
      }
      ostream.Printf("]");
    }

    P0Atom& operator=(const P0Atom & rhs);
  };
} /* namespace MFM */

#include "P0Atom.tcc"

#endif /*P0ATOM_H*/
