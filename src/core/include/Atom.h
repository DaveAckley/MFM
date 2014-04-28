#ifndef ATOM_H      /* -*- C++ -*- */
#define ATOM_H

#include "itype.h"
#include "BitVector.h"
#include "Random.h"
#include <stdio.h>  /* For FILE */

namespace MFM
{

  template <class CC> class Element; // Forward declaration

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
    
  public:
    BitVector<BPA> m_bits;
    
  public:
    static bool IsType(const T& atom, u32 type) 
    {
      return atom.GetType()==type;
    }
    
    static bool IsSameType(const T& atom, const T & other) 
    {
      return atom.GetType()==other.GetType();
    }

    virtual bool IsSane() const = 0;
    
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
