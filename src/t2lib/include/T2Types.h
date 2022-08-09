/* -*- C++ -*- */
#ifndef T2TYPES_H
#define T2TYPES_H

#include "itype.h"
#include "P3Atom.h"
#include "Point.h"
#include "Site.h"
#include "AtomSerializer.h"
#include "OverflowableCharBufferByteSink.h"

#include "MDist.h"
#include "ElementRegistry.h"
#include "EventWindow.h"
#include "Tile.h"
#include "UlamElement.h"
#include "UlamRef.h"
#include "UlamClass.h"
#include "UlamClassRegistry.h"

namespace MFM {

  typedef u32 Dir8;
  typedef u32 Dir6;

  typedef u8 EWSlotNum;

  typedef u8 CircuitNum;

  typedef P3Atom OurT2Atom;

  typedef Site<P3AtomConfig> OurT2Site;

  typedef Base<P3AtomConfig> OurT2Base;

  typedef EventConfig<OurT2Site,4> OurT2EventConfig;

  typedef MDist<4> OurMDist;

  typedef ElementRegistry<OurT2EventConfig> OurElementRegistry;

  typedef Element<OurT2EventConfig> OurElement;

  typedef UlamElement<OurT2EventConfig> OurUlamElement;

  typedef UlamElementInfo<OurT2EventConfig> OurUlamElementInfo;

  typedef UlamRef<OurT2EventConfig> OurUlamRef;

  typedef UlamClass<OurT2EventConfig> OurUlamClass;

  typedef UlamClassRegistry<OurT2EventConfig> OurUlamClassRegistry;

  typedef UlamContext<OurT2EventConfig> OurUlamContext;

  typedef UlamContextEvent<OurT2EventConfig> OurUlamContextEvent;

  typedef UlamContextRestricted<OurT2EventConfig> OurUlamContextRestricted;

  typedef EventWindow<OurT2EventConfig> OurEventWindow;

  typedef EventWindowRenderer<OurT2EventConfig> OurEventWindowRenderer;

  typedef Tile<OurT2EventConfig> OurTraditionalTile;

  typedef ElementTable<OurT2EventConfig> OurElementTable;

  typedef AtomBitStorage<OurT2EventConfig> OurAtomBitStorage;

  typedef AtomSerializer<P3AtomConfig> OurT2AtomSerializer;

  typedef BitVector<P3AtomConfig::BITS_PER_ATOM> OurT2AtomBitVector;

  typedef Point<s8> BPoint;

  typedef u8 Bytes32[32];

}


#endif /* T2TYPES_H */
