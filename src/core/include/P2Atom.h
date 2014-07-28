/*                                              -*- mode:C++ -*-
  P2Atom.h Atom with (eventually) built in bond support
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
  \file P2Atom.h Atom with (eventually) built in bond support
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef P2ATOM_H      /* -*- C++ -*- */
#define P2ATOM_H

#include <stdio.h>
#include "itype.h"
#include "Point.h"
#include "BitVector.h"
#include "MDist.h"

#define P2ATOM_SIZE 96
#define P2ATOM_RADIUS 5

#define P2ATOM_HEADER_SIZE 8
#define P2ATOM_LONGBOND_SIZE 8
#define P2ATOM_SHORTBOND_SIZE 4


namespace MFM {

  class P2Atom
  {
  public: enum { ATOM_CATEGORY = 2 };
  private:
    BitVector<P2ATOM_SIZE> m_bits;

    typedef MDist<4> MDist4;

  public:

    static u32 StateFunc(P2Atom* atom)
    {
      return atom->GetState();
    }

    P2Atom() {
      FAIL(DEPRECATED);
    }

    P2Atom(u32 state)
    {
      SetState(state);
      FAIL(DEPRECATED);
    }

    bool IsSane() const
    {
      // P2Atom has no implemented error detection abilities
      return true;
    }

    bool HasBeenRepaired()
    {
      // If somebody thinks it's bad, we can't fix it.
      return false;
    }


    u32 GetState()
    { return m_bits.Read(2, 6); }

    void SetState(u32 state)
    { m_bits.Write(2, 6, state); }

    u32 GetLongBondCount()
    { return m_bits.Read(9, 3); }

    void SetLongBondCount(u32 count)
    { m_bits.Write(9, 3, count); }

    u32 GetShortBondCount()
    { return m_bits.Read(12, 4); }

    void SetShortBondCount(u32 count)
    { m_bits.Write(12, 4, count); }

    void ReadVariableBodyInto(u32* arr)
    {
      arr[0] = m_bits.Read(16, 16);
      arr[1] = m_bits.Read(32, 32);
    }

    void WriteVariableBodyFrom(u32* arr)
    {
      m_bits.Write(16, 16, arr[0]);
      m_bits.Write(32, 32, arr[1]);
    }

    void WriteLowerBits(u32 val)
    {
      m_bits.Write(32, 32, val);
    }

    u32 ReadLowerBits()
    {
      return m_bits.Read(32, 32);
    }

    void PrintBits(ByteSink & ostream)
    { m_bits.Print(ostream); }

    /* Adds a long bond. Returns its index. */
    u32 AddLongBond(const SPoint& offset);

    u32 AddShortBond(const SPoint& offset);

    /* Fills pt with the long bond location in index. */
    void FillLongBond(u32 index, SPoint& pt);

    void FillShortBond(u32 index, SPoint& pt);

    /*
     * Removes a long bond. Be careful; if a
     * bond is removed, the bonds ahead of it
     * (if they exist) will be pushed downwards.
     * The indices of these bonds will need to
     * be updated again afterwards.
     */
    void RemoveLongBond(u32 index);

    void RemoveShortBond(u32 index);

    P2Atom& operator=(P2Atom rhs);
  };
} /* namespace MFM */
#endif /*P2ATOM_H*/
