#ifndef ELEMENT_EMITTER_H
#define ELEMENT_EMITTER_H

#include "element.h"       /* -*- C++ -*- */
#include "eventwindow.h"
#include "elementtype.h"
#include "elementtable.h"
#include "element_data.h"
#include "element_reprovert.h"
#include "itype.h"
#include "p1atom.h"

namespace MFM
{

#define DATA_CREATE_ODDS 11
#define DATA_MAXVAL 100
#define DATA_MINVAL 1

  template <class T, u32 R>
  class Element_Emitter : public Element_Reprovert<T,R>
  {
    //static bool element_dreg_registered;

  public:
    static Element_Emitter THE_INSTANCE;
    static const u32 TYPE = 0xdada1;

    Element_Emitter() {}
    
    virtual const T & GetDefaultAtom() const 
    {
      static T defaultAtom(TYPE,0,0,Element_Reprovert<T,R>::STATE_BITS);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0xff808080;
    }

    virtual void Behavior(EventWindow<T,R>& window) const
    {
      Random & random = window.GetRandom();

      this->ReproduceVertically(window);
      
      /* Create some data */
      SPoint repPt;
      ManhattanDir<R>::get().FillRandomSingleDir(repPt);
      
      if(random.OneIn(DATA_CREATE_ODDS))
      {
	if(window.GetRelativeAtom(repPt).GetType() == ELEMENT_NOTHING)
	{
	  T atom = Element_Data<T,R>::THE_INSTANCE.GetDefaultAtom();
          Element_Data<T,R>::THE_INSTANCE.SetDatum(atom,random.Between(DATA_MINVAL, DATA_MAXVAL));
	  window.SetRelativeAtom(repPt, atom);
	}
      }
    }

    static void Needed();
    
  };

  template <class T, u32 R>
  Element_Emitter<T,R> Element_Emitter<T,R>::THE_INSTANCE;

  /*
  template <class T, u32 R>
  void Element_Emitter<T,R>::Needed()
  {
    ElementTable<T,R>::get().RegisterElement(Element_Emitter<T,R>::THE_INSTANCE);
  }
  */
}

#endif /* ELEMENT_EMITTER_H */
