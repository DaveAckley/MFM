#ifndef ELEMENT_BOIDS2_H   /* -*- C++ -*- */
#define ELEMENT_BOIDS2_H

#include "Element_Boids.h"    

namespace MFM
{

  template <class CC>
  class Element_Boids2 : public Element_Boids<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
  public:
    const char* GetName() const { return "B2"; }

    static Element_Boids2 THE_INSTANCE;
    static const u32 SUB_TYPE = 2;
    static const u32 TYPE = Element_Boids<CC>::TYPE|(SUB_TYPE<<Element_Boids<CC>::TYPE_BITS);

    Element_Boids2() { }

    virtual const T & GetDefaultAtom() const 
    {
      static T defaultAtom(TYPE,0,0,Element_Boids<CC>::STATE_BITS);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0xfff00000;
    }
  };

  template <class CC>
  Element_Boids2<CC> Element_Boids2<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_BOIDS2_H */
