/* -*- C++ -*- */
#ifndef ELEMENT_EMPTY_H
#define ELEMENT_EMPTY_H

#include "Element.h"
#include "EventWindow.h"
#include "itype.h"

namespace MFM
{

#define EMPTY_VERSION 0

  template <class CC>
  class Element_Empty : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;

  public:

    static Element_Empty THE_INSTANCE;
    static u32 TYPE() { return THE_INSTANCE.GetType(); }
    static bool IsType(u32 type) {
      return type == TYPE();
    }

    Element_Empty() : Element<CC>(MFM_UUID_FOR("Empty", EMPTY_VERSION)) { }

    virtual u32 PercentMovable(const T& you,
			       const T& me, const SPoint& offset) const
    {
      return 100;
    }

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
