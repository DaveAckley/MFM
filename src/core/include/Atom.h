/*                                              -*- mode:C++ -*-
  Atom.h Instance of MFM Element
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
  \file Atom.h Instance of MFM Element
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ATOM_H
#define ATOM_H

#include "itype.h"
#include "VD.h"
#include "BitVector.h"
#include "Random.h"
#include "Util.h"     /* For REQUIRE */
#include "Logger.h"
#include "ByteSource.h"
#include "OverflowableCharBufferByteSink.h"

namespace MFM
{

  template <class CC> class Element; // Forward declaration
  template <class CC> class AtomSerializer; // Forward declaration
  template <class BV, VD::Type, u32, u32> class BitField;  // Forward declaration

  /**
     An Atom is a fixed-size collection of bits, representing an
     instance of an Element in the Movable Feast Machine.  The
     contents of a site is currently a single Atom.

     In general, an Atom is the MFM analog to an 'object' in an
     object-oriented programming language, as Element is the MFM
     analog to a 'class'.  Be warned, however, that the analogy is
     very loose!  Atoms are both fixed size and very small, comprising
     dozens or perhaps hundreds of bits only.

     The Atom class is abstract; subclasses are to provide details of
     atomic structure, but note Atom <b>contains \em NO virtual
     methods</b>.  Our goal is to avoid incurring the cost of a vtable
     pointer on every Atom instance, so all Atom accesses must be by
     template type rather than by base class polymorphism.

     In addition to the 'virtually virtual' methods that Atom
     subclasses must provide (i.e., the methods documented as 'This is
     to be defined only by a subclass of Atom'), Atom subclasses must
     also provide compile time constants: ATOM_CATEGORY and
     ATOM_FIRST_STATE_BIT.
  */
  template <class CC>
  class Atom
  {
    // Extract short names for parameter types
    /**
       The subclass of this Atom that will be used during compilation.
    */
    typedef typename CC::ATOM_TYPE T;

    /**
       A collection of parameters, which are expanded upon below.
    */
    typedef typename CC::PARAM_CONFIG P;

    /**
       The number of bits that every atom will occupy.
    */
    enum { BPA = P::BITS_PER_ATOM };

    /**
       The radius of all event windows.
    */
    enum { R = P::EVENT_WINDOW_RADIUS };

  protected:
    /**
       The state of this Atom, in its entirety. All fields of this
       atom are contained in this BitVector.
    */
    BitVector<BPA> m_bits;

    friend class VD;  // Value Descriptors can mess with our bits
    friend class Element<CC>;  // Let Element mess with our bits
    friend class AtomSerializer<CC>;  // Ditto AtomSerializer
    template <class BV, VD::Type, u32, u32> friend class BitField;  // Ditto BitField (all instances)

  public:

    Atom()
    {
      // Create a compilation error if sizeof(Atom)!=sizeof(m_bits)
      COMPILATION_REQUIREMENT<sizeof(Atom)==sizeof(m_bits)>();
    }

    /**
     * Checks to see if a specified Atom is of a particular \a type.
     *
     * @param atom The atom to check the type of.
     *
     * @param type The Atomic type to check \a atom for membership of.
     *
     * @returns true if \a atom is of type \a type.
     */
    static bool IsType(const T& atom, u32 type)
    {
      return atom.GetType()==type;
    }

    /**
     *  Checks to see if two atoms are of the same type.
     *
     * @param atom One atom to check the type of
     *
     * @param other The other atom to check the type of
     *
     * @returns true of both Atoms are the same type.
     */
    static bool IsSameType(const T& atom, const T& other)
    {
      return atom.GetType()==other.GetType();
    }

    /**
     * Checks to see if this Atom has a reasonable enough bit pattern
     * within its BitVector to not be considered illegal.
     *
     * @returns true if this Atom is considered to be in a legal
     * state, else false.
     *
     * @remarks This delegates to IsSaneImpl in the subclass of Atom.
     */
    bool IsSane() const
    {
      return static_cast<const T*>(this)->IsSaneImpl();
    }


    /**
     * Assuming IsSane has returned false, this attempts to correct
     * the type of this atom, if it is possible to do.
     *
     * @returns true if this Atom now has a legal type, false if the
     * type is so damaged it should just be destroyed
     *
     * @remarks This delegates to HasBeenRepairedImpl in the subclass of Atom.
     */
    bool HasBeenRepaired()
    {
      return static_cast<T*>(this)->HasBeenRepairedImpl();
    }

    /**
     * Gets the type of this Atom.
     *
     * @returns The type of this Atom.
     *
     * @remarks This delegates to GetTypeImpl in the subclass of Atom.
     */
    u32 GetType() const
    {
      return static_cast<const T*>(this)->GetTypeImpl();
    }

    /**
     * Prints this Atom in a semi-readable way to a ByteSink.
     *
     * @remarks This delegates to PrintImpl in the subclass of Atom.
     */
    void Print(ByteSink & ostream) const
    {
      static_cast<const T*>(this)->PrintImpl(ostream);
    }

    /**
     * Writes all bits of this Atom to a ByteSink. These bits must
     * not include the type bits of this Atom.
     *
     * @remarks This delegates to WriteStateBits in the subclass of Atom.
     */
    void WriteStateBits(ByteSink& ostream) const
    {
      static_cast<const T*>(this)->WriteStateBitsImpl(ostream);
    }

    /**
     * Reads the State bits of this atom from a given hex-encoded char* .
     *
     * @remarks This delegates to ReadStateBits in the subclass of Atom.
     */
    void ReadStateBits(const char* hexStr)
    {
      static_cast<T*>(this)->ReadStateBitsImpl(hexStr);
    }


    /**
     * Reads the State bits of this atom from a BitVector.  Only
     * the non-type bits are copied.
     *
     * @remarks This delegates to ReadStateBits in the subclass of Atom.
     */
    void ReadStateBits(const BitVector<BPA> & bv)
    {
      static_cast<T*>(this)->ReadStateBitsImpl(bv);
    }


    /**
     * Writes the internal bits of this Atom to a ByteSink in hex format.
     *
     * @param ostream The ByteSink to write the bits of this Atom to.
     */
    void WriteBits(ByteSink& ostream) const
    {
      m_bits.Print(ostream);
    }

    /**
     * Reads bits from a ByteSource in hex format, replacing the bits
     * of this Atom with the read bits.
     *
     * @param inStream The ByteSource to read bits from.
     */
    void ReadBits(const char* hexStr)
    {
      u32 len = strlen(hexStr);
      u32 bitLen = len * 4;

      if(bitLen > BPA)
      {
	FAIL(ILLEGAL_ARGUMENT);
      }

      m_bits.Clear();

      u32 bitIdx = 0;
      char hexVal;

      for(s32 i = len - 1; i >= 0; i--)
      {
	hexVal = hexStr[i];
	if(hexVal >= '0' && hexVal <= '9')
	{
	  hexVal -= '0';
	}
	else if(hexVal >= 'a' && hexVal >= 'f')
	{
	  hexVal = hexVal - 'a' + 10;
	}
	else if(hexVal >= 'A' && hexVal <= 'F')
	{
	  hexVal = hexVal - 'A' + 10;
	}
	else
	{
	  FAIL(ILLEGAL_ARGUMENT);
	}

	for(s32 j = 3; j >= 0; j--)
	{
	  bitIdx = i * 4 + j;
	  m_bits.WriteBit(bitIdx, !!(hexVal & (1 << j)));
	}
      }
    }

    /**
     * Randomly flips some of the bits in this Atom's BitVector.
     *
     * @param rand The PRNG used to determine randomness of bit
     * flipping.
     *
     * @param bitOdds The odds (one in these odds) that a particular
     * bit will be flipped.
     */
    void XRay(Random& rand, u32 bitOdds)
    {
      for(u32 i = 0; i < BPA; i++)
      {
        if(rand.OneIn(bitOdds))
        {
          m_bits.ToggleBit(i);
        }
      }
    }

    bool operator==(const Atom & rhs) const
    {
      return m_bits == rhs.m_bits;
    }

    bool operator!=(const Atom & rhs) const
    {
      return !(*this == rhs);
    }
  };


} /* namespace MFM */

#endif /*ATOM_H*/
