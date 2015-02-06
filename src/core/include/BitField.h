/*                                              -*- mode:C++ -*-
  BitField.h Section of a BitVector
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
  \file BitField.h Section of a BitVector
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef BITFIELD_H
#define BITFIELD_H

#include "BitVector.h"
#include "VD.h"
#include "Atom.h"

namespace MFM {

  /**
   * A field of up to 32 contiguous bits (64 for VD::BITS) of a
   * BitVector, interpreted as some type VD:::Type
   */
  template <class BV, VD::Type VT, u32 LEN, u32 IDX>
  class BitField
  {
  public:
    typedef typename VTypeToType<VT>::TYPE VTYPE;

    enum
    {
      BITS = BV::BITS,
      START = IDX,
      LENGTH = LEN,
      END = START + LENGTH
    };

    enum
    {
      BITFIELD_ERROR_OVERFLOWED_BITS = (END+1)/((BITS+1)/(END+1))
    };

    enum
    {
      BITFIELD_ERROR_FIELD_TOO_LONG = (LENGTH+1)/((64+1)/(LENGTH+1))
    };

    /**
     * Reads the contents of a window (which is represented by this
     * BitField) from a specified BV into a u32.
     *
     * @param bv The BV to read bits from.
     *
     * @returns the bits inside the window specified by this BitField
     *          which reside in bv.
     */
    static u32 Read(const BV & bv)
    {
      return bv.Read(START,LENGTH);
    }

    /**
     * writes the contents of a u32 into a window (which is represented by this
     * BitField) of a specified BV.
     *
     * @param bv The BV to write bits to.
     */
    static void Write(BV & bv, u32 val)
    {
      bv.Write(START,LENGTH,val);
    }

    /**
     * Reads the contents of a window (which is represented by this
     * BitField) from a specified BV into a u64.
     *
     * @param bv The BV to read bits from.
     *
     * @returns the bits inside the window specified by this BitField
     *          which reside in bv.
     */
    static u64 ReadLong(const BV & bv)
    {
      return bv.ReadLong(START,LENGTH);
    }

    /**
     * Read an element of an array, interpreting the entire bitfield
     * as a contiguous sequence of items each of which is size \c
     * unitsize.  Both the unit size and the array index are specified
     * at runtime.
     *
     * @param bv The BV to read bits from.
     *
     * @param index The array index to read
     *
     * @param unitsize The number of bits per array item
     *
     * @returns the \c unitsize bits from \bv, starting at position \c
     *          index * \c unitsize within this BitField.
     */
    static u32 ReadArray(const BV & bv, const u32 index, const u32 unitsize)
    {
      return bv.Read((index * unitsize) + IDX, unitsize);
    }

    /**
     * Write an element of an array, interpreting the entire bitfield
     * as a contiguous sequence of items each of which is size \c
     * unitsize.  Both the unit size and the array index are specified
     * at runtime.
     *
     * @param bv The BV to write bits to.
     *
     * @param val the value to write to \bv (taking only the bottom \c
     * unitsize bits).
     *
     * @param index The array index to write
     *
     * @param unitsize The number of bits per array item.
     *
     */
    static void WriteArray(BV & bv, const u32 val, const u32 index, const u32 unitsize)
    {
      bv.Write((index * unitsize) + IDX, unitsize, val);
    }

    /**
     * Sets bit bitnum within this BitField (with bitnum 0 meaning the
     * leftmost bit of this field).
     *
     * @param bv The BV to alter.
     *
     * @param bitnum The number of the bit to set.
     *
     */
    static void SetBit(BV & bv, u32 bitnum)
    {
      return bv.SetBit(START+bitnum);
    }

    template <class AC>
    static void SetBit(Atom<AC> & atom, u32 bitnum)
    {
      return atom.m_bits.SetBit(START+bitnum);
    }

    /**
     * Clears bit bitnum within this BitField (with bitnum 0 meaning
     * the leftmost bit of this field).
     *
     * @param bv The BV to alter.
     *
     * @param bitnum The number of the bit to clear.
     *
     */
    static void ClearBit(BV & bv, u32 bitnum)
    {
      return bv.ClearBit(START+bitnum);
    }

    template <class AC>
    static void ClearBit(Atom<AC> & atom, u32 bitnum)
    {
      return atom.m_bits.ClearBit(START+bitnum);
    }

    /**
     * Toggles bit bitnum within this BitField (with bitnum 0 meaning
     * the leftmost bit of this field).
     *
     * @param bv The BV to alter.
     *
     * @param bitnum The number of the bit to toggle.
     *
     */
    static void ToggleBit(BV & bv, u32 bitnum)
    {
      return bv.ToggleBit(START+bitnum);
    }

    template <class AC>
    static void ToggleBit(Atom<AC> & atom, u32 bitnum)
    {
      return atom.m_bits.ToggleBit(START+bitnum);
    }


    /**
     * Read bit bitnum within this BitField (with bitnum 0 meaning
     * the leftmost bit of this field).
     *
     * @param bv The BV to examine.
     *
     * @param bitnum The number of the bit to read
     *
     * @returns true if bit bitnum is set in bv
     *
     */
    static bool ReadBit(const BV & bv, u32 bitnum)
    {
      return bv.ReadBit(START+bitnum);
    }

    template <class AC>
    static bool ReadBit(const Atom<AC> & atom, u32 bitnum)
    {
      return atom.m_bits.ReadBit(START+bitnum);
    }

    /**
     * writes the contents of a u64 into a window (which is represented by this
     * BitField) of a specified BV.
     *
     * @param bv The BV to write bits to.
     */
    static void WriteLong(BV & bv, u64 val)
    {
      bv.WriteLong(START,LENGTH,val);
    }

    template <class AC>
    static u32 Read(const Atom<AC> & atom)
    {
      return Read(atom.m_bits);
    }

    template <class AC>
    static void Write(Atom<AC> & atom, u32 val)
    {
      Write(atom.m_bits, val);
    }

    //////
    // Type specific: u32
    template <class AC>
    static void Load(const Atom<AC> & atom, u32 & val)
    {
      val = Read(atom);
    }

    template <class AC>
    static void Store(Atom<AC> & atom, const u32 & val)
    {
      Write(atom,val);
    }

    //////
    // Type specific: s32
    template <class AC>
    static void Load(const Atom<AC> & atom, s32 & val)
    {
      u32 v = Read(atom);
      val = _SignExtend32(v, LENGTH);
    }

    template <class AC>
    static void Store(Atom<AC> & atom, const s32 & val)
    {
      Write(atom,(u32) val);
    }

    //////
    // Type specific: bool
    template <class AC>
    static void Load(const Atom<AC> & atom, bool & val)
    {
      u32 v = Read(atom);
      val = PopCount(v) > LENGTH/2;
    }

    template <class AC>
    static void Store(Atom<AC> & atom, const bool & val)
    {
      Write(atom,val? _GetNOnes32(LENGTH) : 0u);
    }

    //////
    // Type specific: BITS
    template <class AC>
    static void Load(const Atom<AC> & atom, u64 & val)
    {
      val = ReadLong(atom.m_bits);
    }

    template <class AC>
    static void Store(Atom<AC> & atom, const u64 & val)
    {
      WriteLong(atom.m_bits,val);
    }


    template <class AC>
    static VTYPE GetValue(const Atom<AC> & a)
    {
      VTYPE temp;
      Load(a, temp);
      return temp;
    }

    template <class AC>
    static void SetValue(Atom<AC> & a, VTYPE val)
    {
      Store(a, val);
    }

  };
} /* namespace MFM */

#endif /*BITFIELD_H*/
