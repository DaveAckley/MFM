#ifndef ELEMENT_RES_H       /* -*- C++ -*- */
#define ELEMENT_RES_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{

  template <class T, u32 R>
  class Element_Res : public Element<T,R>
  {
  public:
    const char* GetName() const { return "Res"; }

    static Element_Res THE_INSTANCE;
    static const u32 TYPE = 0xa11ce11;             // We are stem cells able to become anything

    Element_Res() {}
    
    virtual const T & GetDefaultAtom() const 
    {
      static T defaultAtom(TYPE,0,0,0);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0xffffff00;
    }

    virtual void Behavior(EventWindow<T,R>& window) const
    {
      this->Diffuse(window);
    }

    static void Needed();    
  };

  template <class T, u32 R>
  Element_Res<T,R> Element_Res<T,R>::THE_INSTANCE;

  /*
  template <class T, u32 R>
  void Element_Res<T,R>::Needed()
  {
    ElementTable<T,R>::get().RegisterElement(Element_Res<T,R>::THE_INSTANCE);
  }
  */
}

#endif /* ELEMENT_RES_H */
