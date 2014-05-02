#ifndef ATOM_H      /* -*- C++ -*- */
#define ATOM_H

#include "itype.h"
#include "BitVector.h"
#include "Random.h"
#include <stdio.h>  /* For FILE */

namespace MFM
{

  template <class CC> class Element; // Forward declaration

  /**
   * Base class for the Atom type, which represents an instance of an
   * Element in the Movable Feast Machine.
   */
  template <class CC>
  class Atom
  {
    // Extract short names for parameter types
    /**
       The kind of Atom, or sub-class of this Atom, that will be used
       during compilation.
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

    /**
       The width of every tile, in sites.
     */
    enum { W = P::TILE_WIDTH };

    /**
       The number of bits required to represent an Element Table.
     */
    enum { B = P::ELEMENT_TABLE_BITS };

  public:
    /**
       The state of this Atom, in its entirety. All fields of this
       atom are contained in this BitVector.
     */
    BitVector<BPA> m_bits;

  public:
    /**
     * Checks to see if a specified Atom is of a particular type.
     *
     * @param atom The atom to check the type of.
     *
     * @param type The Atomic type to check atom for membership of.
     *
     * @returns true if atom is of type type.
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
     */
    virtual bool IsSane() const = 0;

    /**
     * Gets the type of this Atom.
     *
     * @returns The type of this Atom.
     *
     * @remarks This is to be defined only by a subclass of Atom.
     */
    u32 GetType() const;

    /**
     * Prints this Atom in a semi-readable way to a FILE*.
     *
     * @remarks This is to be defined only by a subclass of Atom.
     */
    void Print(FILE* ostream) const;

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
      for(u32 i = 0; i < BPA >> 1; i++)
      {
	if(rand.OneIn(bitOdds))
	{
	  m_bits.ToggleBit(i);
	  printf("Bit %d xray'd\n", i);
	}
      }
    }
  };
} /* namespace MFM */

#endif /*ATOM_H*/
