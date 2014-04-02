#ifndef ELEMENT_CONSUMER_H
#define ELEMENT_CONSUMER_H

#include "Element.h"       /* -*- C++ -*- */
#include "EventWindow.h"
#include "ElementTable.h"
#include "Element_Data.h"
#include "Element_Empty.h"
#include "Element_Reprovert.h"
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{
  template <class CC>
  class Element_Consumer : public Element_Reprovert<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  public:
    const char* GetName() const { return "Consumer"; }

    static Element_Consumer THE_INSTANCE;
    static const u32 TYPE = 0xdada0;

    Element_Consumer() { }

    virtual const T & GetDefaultAtom() const 
    {
      static T defaultAtom(TYPE,0,0,Element_Reprovert<CC>::STATE_BITS);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0xff101010;
    }
    
    virtual void Behavior(EventWindow<CC>& window) const
    {
      Random & random = window.GetRandom();

      this->ReproduceVertically(window);

      Point<s32> consPt;
      MDist<R>::get().FillRandomSingleDir(consPt, random);
      
      if(window.GetRelativeAtom(consPt).GetType() == Element_Data<CC>::TYPE)
      {
	/*
	u32 val = Element_Data<CC>::THE_INSTANCE.GetDatum(window.GetRelativeAtom(consPt),0);
	u32 bnum = this->GetVertPos(window.GetCenterAtom(),0);

	// something sort of constant at equil.?
	printf("[%3d]Export!: %d sum %d\n", bnum, val, 3*bnum+val); 
	*/
	window.SetRelativeAtom(consPt, Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
      }
    }

    static void Needed();
    
  };

  template <class CC>
  Element_Consumer<CC> Element_Consumer<CC>::THE_INSTANCE;

}

#endif /* ELEMENT_CONSUMER_H */
