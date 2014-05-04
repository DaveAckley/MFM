#ifndef ATOM_H      /* -*- C++ -*- */
#define ATOM_H

#include "itype.h"
#include "BitVector.h"
#include "Random.h"
#include "Util.h"     /* For REQUIRE */
#include <stdio.h>  /* For FILE */

namespace MFM
{

  template <class CC> class Element; // Forward declaration

  /**
     An Atom is a fixed-size collection of bits, representing the
     contents of a site.  An Atom is the MFM analog to an 'object' in
     an object-oriented programming language, as \sa Element is the
     MFM analog to a 'class'.  Be warned, however, that the analogy is
     very loose!  Atoms are both fixed size and very small, comprising
     dozens or perhaps hundreds of bits only.

     The Atom class is abstract; subclasses are to provide details of
     atomic structure, but note Atom <b>contains \em NO virtual
     methods</b>.  Our goal is to avoid incurring the cost of a vtable
     pointer on every Atom instance, so all Atom accesses must be by
     template type rather than by base class polymorphism.

   */
  template <class CC>
  class Atom
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { BPA = P::BITS_PER_ATOM };
    enum { R = P::EVENT_WINDOW_RADIUS };
    enum { W = P::TILE_WIDTH };
    enum { B = P::ELEMENT_TABLE_BITS };

  protected:
    /**
       The actual and only storage space of an Atom.
     */
    BitVector<BPA> m_bits;

  public:

    Atom() {
      // Create a compilation error if sizeof(Atom)!=sizeof(m_bits)
      COMPILATION_REQUIREMENT<sizeof(Atom)==sizeof(m_bits)>();
    }

    /**
       Return true if the given \a atom is the given \a type.
     */
    static bool IsType(const T& atom, u32 type)
    {
      return atom.GetType()==type;
    }

    static bool IsSameType(const T& atom, const T & other)
    {
      return atom.GetType()==other.GetType();
    }

    // To be defined by subclasses only
    bool IsSane() const;

    // To be defined by subclasses only
    u32 GetType() const;

    // To be defined by subclasses only
    void Print(FILE* ostream) const;

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

#include "Atom.tcc"

#endif /*ATOM_H*/
