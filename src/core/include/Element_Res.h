#ifndef ELEMENT_RES_H       /* -*- C++ -*- */
#define ELEMENT_RES_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{

#define RES_VERSION 1

  template <class CC>
  class Element_Res : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;

  public:

    static Element_Res THE_INSTANCE;
    static const u32 TYPE() {
      return THE_INSTANCE.GetType();
    }

    Element_Res() : Element<CC>(MFM_UUID_FOR("Res", RES_VERSION)) {}

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,0);
      return defaultAtom;
    }

    virtual u32 PercentMovable(const T& you,
			       const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff676700;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      this->Diffuse(window);
    }
  };

  template <class CC>
  Element_Res<CC> Element_Res<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_RES_H */
