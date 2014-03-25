#ifndef ELEMENT_EMPTY_H
#define ELEMENT_EMPTY_H

#include "Element.h"       /* -*- C++ -*- */
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{

  template <class CC>
  class Element_Empty : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;

  public:
    const char* GetName() const { return "Empty"; }

    static Element_Empty THE_INSTANCE;
    static const u32 TYPE = 0x0;             // We get the best type of all

    Element_Empty() { }

    virtual const T & GetDefaultAtom() const 
    {
      static T defaultAtom;                 // And the default ctor makes us!
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0x00000000;                    // Not black.. transparent
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
    }
  };

  template <class CC>
  Element_Empty<CC> Element_Empty<CC>::THE_INSTANCE;
}

#endif /* ELEMENT_EMPTY_H */
