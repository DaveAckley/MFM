/* -*- C++ -*- */
#ifndef T2TYPES_H
#define T2TYPES_H

#include "itype.h"
#include "P3Atom.h"

namespace MFM {

  typedef u32 Dir8;
  typedef u32 Dir6;

  typedef u8 EWSlotNum;

  typedef u8 CircuitNum;

  typedef P3Atom OurT2Atom;

  typedef Site<P3AtomConfig> OurT2Site;
}


#endif /* T2TYPES_H */
