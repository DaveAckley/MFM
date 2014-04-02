/* -*- C++ -*- */

#ifndef ELEMENT_WALL_H
#define ELEMENT_WALL_H

#include "Element.h"   
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{
  template <class CC>
  class Element_Wall : public Element<CC>
  {
    // Short names for params
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  public:
    const  char* GetName() const { return "Wall";}

    static Element_Wall THE_INSTANCE;
    static const u32 TYPE = 0x00f;  // Oof. A wall.
    
    Element_Wall() { }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE,0,0,0);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xffffffff;
    }
    
    virtual void Behavior(EventWindow<CC>& window) const
    {}

    static void Needed();

  };

  template <class CC>
  Element_Wall<CC> Element_Wall<CC>::THE_INSTANCE;
}

#endif /* ELEMENT_WALL_H */
