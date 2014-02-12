#include "element.h"       /* -*- C++ -*- */
#include "eventwindow.h"
#include "elementtype.h"
#include "elementtable.h"
#include "itype.h"
#include "p1atom.h"

namespace MFM
{

#define DREG_RES_ODDS 20
#define DREG_DEL_ODDS 10
#define DREG_DRG_ODDS 1000
#define DREG_DDR_ODDS 10 /*Deleting DREGs*/

  template <class T, u32 R>
  class Element_Dreg : public Element<T,R>
  {
    //static bool element_dreg_registered;

  public:
    static Element_Dreg THE_INSTANCE;

    Element_Dreg() {}
    
    typedef u32 (* StateFunction )(T* atom);

    virtual void Behavior(EventWindow<T,R>& window, StateFunction f)
    {
      Random & random = window.GetRandom();

      SPoint dir;
      ManhattanDir<R>::get().FillRandomSingleDir(dir);
      
      u32 state = f(&window.GetRelativeAtom(dir));
      
      ElementType newType = (ElementType)-1;
      
      if(state == ELEMENT_NOTHING)
      {
	if(random.OneIn(DREG_DRG_ODDS))
	  {
	    newType = ELEMENT_DREG;
	  }
	else if(random.OneIn(DREG_RES_ODDS))
	  {
	    newType = ELEMENT_RES;
	  }
      }
    else if(state == ELEMENT_DREG)
      {
	if(random.OneIn(DREG_DDR_ODDS))
	  {
	    newType = ELEMENT_NOTHING;
	  }
      }
    else if(random.OneIn(DREG_DEL_ODDS))
      {
	newType = ELEMENT_NOTHING;
      }

    if(newType >= 0)
      {
	window.SetRelativeAtom(dir, P1Atom(newType));
      }
    }

    static void Needed();
    
  };

  template <class T, u32 R>
  Element_Dreg<T,R> Element_Dreg<T,R>::THE_INSTANCE;

  template <class T, u32 R>
  void Element_Dreg<T,R>::Needed()
  {
    ElementTable<T,R>::get().RegisterElement(ELEMENT_DREG, &Element_Dreg<T,R>::THE_INSTANCE);
  }
}
