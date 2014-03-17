#ifndef ELEMENT_BOIDS2_H   /* -*- C++ -*- */
#define ELEMENT_BOIDS2_H

#include "Element_Boids.h"    

namespace MFM
{

  template <class T, u32 R>
  class Element_Boids2 : public Element_Boids<T,R>
  {
  public:
    const char* GetName() const { return "B2"; }

    static Element_Boids2 THE_INSTANCE;
    static const u32 SUB_TYPE = 2;
    static const u32 TYPE = Element_Boids<T,R>::TYPE|(SUB_TYPE<<Element_Boids<T,R>::TYPE_BITS);

    Element_Boids2() { }

    virtual const T & GetDefaultAtom() const 
    {
      static T defaultAtom(TYPE,0,0,Element_Boids<T,R>::STATE_BITS);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0xfff00000;
    }
  };

  template <class T, u32 R>
  Element_Boids2<T,R> Element_Boids2<T,R>::THE_INSTANCE;

}

#endif /* ELEMENT_BOIDS2_H */
