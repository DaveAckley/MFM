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

#define WALL_VERSION 1

  template <class CC>
  class Element_Wall : public Element<CC>
  {
    // Short names for params
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  public:

    static Element_Wall THE_INSTANCE;
    static const u32 TYPE() {
      return THE_INSTANCE.GetType();
    }

    Element_Wall() : Element<CC>(MFM_UUID_FOR("Wall", WALL_VERSION)) { }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,0);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xffffffff;
    }

    virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const {
      return nowAt.Equals(maybeAt)?Element<CC>::COMPLETE_DIFFUSABILITY:0;
    }

    virtual u32 PercentMovable(const T& you,
			       const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {}
  };

  template <class CC>
  Element_Wall<CC> Element_Wall<CC>::THE_INSTANCE;
}

#endif /* ELEMENT_WALL_H */
