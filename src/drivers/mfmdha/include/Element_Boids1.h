#ifndef ELEMENT_BOIDS1_H   /* -*- C++ -*- */
#define ELEMENT_BOIDS1_H

#include "Element_Boids.h"    

namespace MFM
{

  template <class T, u32 R>
  class Element_Boids1 : public Element_Boids<T,R>
  {
  public:
    const char* GetName() const { return "B1"; }

    static Element_Boids1 THE_INSTANCE;
    static const u32 SUB_TYPE = 1;
    static const u32 TYPE = Element_Boids<T,R>::TYPE|(SUB_TYPE<<Element_Boids<T,R>::TYPE_BITS);

    Element_Boids1() { }

    virtual const T & GetDefaultAtom() const 
    {
      static T defaultAtom(TYPE,0,0,Element_Boids<T,R>::STATE_BITS);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0xff00f0f0;
    }
  };

  template <class T, u32 R>
  Element_Boids1<T,R> Element_Boids1<T,R>::THE_INSTANCE;

}

#endif /* ELEMENT_BOIDS1_H */
