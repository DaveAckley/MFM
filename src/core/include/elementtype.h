#ifndef ELEMENTTYPE_H /* -*- C++ -*- */
#define ELEMENTTYPE_H

namespace MFM
{
  typedef enum
  {
    ELEMENT_NOTHING  = 0x0,
    ELEMENT_DREG     = 0x1,
    ELEMENT_RES      = 0x2,
    ELEMENT_SORTER   = 0x3,
    ELEMENT_EMITTER  = 0x4,
    ELEMENT_CONSUMER = 0x5,
    ELEMENT_DATA     = 0x6
  }ElementType;
}

#endif /*ELEMENTTYPE_H*/
