/* -*- C++ -*- */
#ifndef T2TYPES_H
#define T2TYPES_H

#include "itype.h"
#include "P3Atom.h"
#include "Site.h"
#include "AtomSerializer.h"
#include "OverflowableCharBufferByteSink.h"

namespace MFM {

  typedef u32 Dir8;
  typedef u32 Dir6;

  typedef u8 EWSlotNum;

  typedef u8 CircuitNum;

  typedef P3Atom OurT2Atom;

  typedef Site<P3AtomConfig> OurT2Site;

  typedef Base<P3AtomConfig> OurT2Base;

  typedef AtomSerializer<P3AtomConfig> OurT2AtomSerializer;

  typedef BitVector<P3AtomConfig::BITS_PER_ATOM> OurT2AtomBitVector;

  
}


#endif /* T2TYPES_H */
