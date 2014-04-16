/* -*- C++ -*- */
#ifndef ELEMENT_EMITTER_H
#define ELEMENT_EMITTER_H

#include "Element.h"       
#include "EventWindow.h"
#include "ElementTable.h"
#include "Element_Data.h"
#include "Element_Reprovert.h"
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{

#define DATA_CREATE_ODDS 12

  template <class CC>
  class Element_Emitter : public Element_Reprovert<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  public:
    const char* GetName() const { return "Emitter"; }

    static Element_Emitter THE_INSTANCE;
    static const u32 TYPE = 0xdada1;

    Element_Emitter() {}
    
    virtual const T & GetDefaultAtom() const 
    {
      static T defaultAtom(TYPE,0,0,Element_Reprovert<CC>::STATE_BITS);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0xff808080;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {
      Random & random = window.GetRandom();

      this->ReproduceVertically(window, 0);
      
      /* Create some data */
      SPoint repPt;
      MDist<R>::get().FillRandomSingleDir(repPt, random);
      
      if(random.OneIn(DATA_CREATE_ODDS))
      {
	if(window.GetRelativeAtom(repPt).GetType() == ELEMENT_EMPTY)
	{
	  T atom = Element_Data<CC>::THE_INSTANCE.GetDefaultAtom();
          Element_Data<CC>::THE_INSTANCE.SetDatum(atom,random.Between(DATA_MINVAL, DATA_MAXVAL));
	  window.SetRelativeAtom(repPt, atom);
	}
      }
    }

    static void Needed();
    
  };

  template <class CC>
  Element_Emitter<CC> Element_Emitter<CC>::THE_INSTANCE;

  /*
  template <class CC>
  void Element_Emitter<CC>::Needed()
  {
    ElementTable<CC>::get().RegisterElement(Element_Emitter<CC>::THE_INSTANCE);
  }
  */
}

#endif /* ELEMENT_EMITTER_H */
