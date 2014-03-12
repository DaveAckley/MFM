#ifndef ELEMENT_CONSUMER_H
#define ELEMENT_CONSUMER_H

#include "element.h"       /* -*- C++ -*- */
#include "eventwindow.h"
#include "elementtype.h"
#include "elementtable.h"
#include "element_data.h"
#include "element_empty.h"
#include "element_reprovert.h"
#include "itype.h"
#include "p1atom.h"

namespace MFM
{


  template <class T, u32 R>
  class Element_Consumer : public Element_Reprovert<T,R>
  {

  public:
    static Element_Consumer THE_INSTANCE;
    static const u32 TYPE = 0xdada0;

    Element_Consumer() { }

    virtual const T & GetDefaultAtom() const 
    {
      static T defaultAtom(TYPE,0,0,Element_Reprovert<T,R>::STATE_BITS);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const 
    {
      return 0xff101010;
    }
    
    virtual void Behavior(EventWindow<T,R>& window) const
    {
      this->ReproduceVertically(window);

      Point<s32> consPt;
      ManhattanDir<R>::get().FillRandomSingleDir(consPt);
      
      if(window.GetRelativeAtom(consPt).GetType() == Element_Data<T,R>::TYPE)
      {
	u32 val = Element_Data<T,R>::THE_INSTANCE.GetDatum(window.GetRelativeAtom(consPt),0);
	u32 bnum = this->GetVertPos(window.GetCenterAtom(),0);

	// something sort of constant at equil.?
	printf("[%3d]Export!: %d sum %d\n", bnum, val, 3*bnum+val); 
	window.SetRelativeAtom(consPt, Element_Empty<T,R>::THE_INSTANCE.GetDefaultAtom());
      }
    }

    static void Needed();
    
  };

  template <class T, u32 R>
  Element_Consumer<T,R> Element_Consumer<T,R>::THE_INSTANCE;

  /*
  template <class T, u32 R>
  void Element_Consumer<T,R>::Needed()
  {
    ElementTable<T,R>::get().RegisterElement(Element_Consumer<T,R>::THE_INSTANCE);
  }
  */
}

#endif /* ELEMENT_CONSUMER_H */
