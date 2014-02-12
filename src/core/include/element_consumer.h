#include "element.h"       /* -*- C++ -*- */
#include "eventwindow.h"
#include "elementtype.h"
#include "elementtable.h"
#include "itype.h"
#include "p1atom.h"

namespace MFM
{


  template <class T, u32 R>
  class Element_Consumer : public Element<T,R>
  {
    //static bool element_dreg_registered;

  public:
    static Element_Consumer CONSUMER_INSTANCE;

    Element_Consumer() {}
    
    typedef u32 (* StateFunction )(T* atom);

    virtual void Behavior(EventWindow<T,R>& window, StateFunction f)
    {
      ReproduceVertically(window, f, ELEMENT_CONSUMER);

      Point<s32> consPt;
      ManhattanDir<R>::get().FillRandomSingleDir(consPt);
      
      if(f(&window.GetRelativeAtom(consPt)) == ELEMENT_DATA)
      {
	u32 val = window.GetRelativeAtom(consPt).ReadLowerBits();
	u32 bnum = window.GetCenterAtom().ReadLowerBits();

	// something sort of constant at equil.?
	printf("[%3d]Export!: %d sum %d\n", bnum, val, 3*bnum+val); 
	window.SetRelativeAtom(consPt, T(ELEMENT_NOTHING));
      }
    }

    static void Needed();
    
  };

  template <class T, u32 R>
  Element_Consumer<T,R> Element_Consumer<T,R>::CONSUMER_INSTANCE;

  template <class T, u32 R>
  void Element_Consumer<T,R>::Needed()
  {
    ElementTable<T,R>::get().RegisterElement(ELEMENT_CONSUMER, &Element_Consumer<T,R>::CONSUMER_INSTANCE);
  }
}
