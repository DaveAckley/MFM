#ifndef ELEMENT_EMPTY_H
#define ELEMENT_EMPTY_H

#include "element.h"       /* -*- C++ -*- */
#include "eventwindow.h"
#include "elementtype.h"
#include "elementtable.h"
#include "itype.h"
#include "p1atom.h"
#include "element_res.h"  /* For Element_Res::TYPE */

namespace MFM
{

  template <class T, u32 R>
  class Element_Empty : public Element<T,R>
  {

  public:
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

    virtual void Behavior(EventWindow<T,R>& window) const
    {
    }
  };

  template <class T, u32 R>
  Element_Empty<T,R> Element_Empty<T,R>::THE_INSTANCE;
}

#endif /* ELEMENT_EMPTY_H */
