#ifndef ELEMENT_RES_H       /* -*- C++ -*- */
#define ELEMENT_RES_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{

  template <class C>
  class Element_Res : public Element<C>
  {
    // Extract short names for parameter types
    typedef typename C::ATOM_TYPE T;

  public:
    const char* GetName() const { return "Res"; }

    static Element_Res THE_INSTANCE;
    static const u32 TYPE = 0xce11;             // We are stem cells able to become anything

    Element_Res() {}

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE,0,0,0);
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

    virtual void Behavior(EventWindow<C>& window) const
    {
      this->Diffuse(window);
    }

    static void Needed();
  };

  template <class C>
  Element_Res<C> Element_Res<C>::THE_INSTANCE;

  /*
  template <class C>
  void Element_Res<C>::Needed()
  {
    ElementTable<C>::get().RegisterElement(Element_Res<C>::THE_INSTANCE);
  }
  */
}

#endif /* ELEMENT_RES_H */
