#include "element.h"       /* -*- C++ -*- */
#include "eventwindow.h"
#include "elementtype.h"
#include "elementtable.h"
#include "itype.h"
#include "p1atom.h"

namespace MFM
{

#define DATA_CREATE_ODDS 8
#define DATA_MAXVAL 100
#define DATA_MINVAL 1

  template <class T, u32 R>
  class Element_Emitter : public Element<T,R>
  {
    //static bool element_dreg_registered;

  public:
    static Element_Emitter THE_INSTANCE;

    Element_Emitter() {}
    
    typedef u32 (* StateFunction )(T* atom);

    virtual void Behavior(EventWindow<T,R>& window, StateFunction f)
    {
      Random & random = window.GetRandom();

      ReproduceVertically(window, f, ELEMENT_EMITTER);
      
      /* Create some data */
      SPoint repPt;
      ManhattanDir<R>::get().FillRandomSingleDir(repPt);
      
      if(random.OneIn(DATA_CREATE_ODDS))
      {
	if(f(&window.GetRelativeAtom(repPt)) == ELEMENT_NOTHING)
	{
	  T atom = T(ELEMENT_DATA);
	  atom.WriteLowerBits(random.Between(DATA_MINVAL, DATA_MAXVAL));
	  window.SetRelativeAtom(repPt, atom);
	}
      }
    }

    static void Needed();
    
  };

  template <class T, u32 R>
  Element_Emitter<T,R> Element_Emitter<T,R>::THE_INSTANCE;

  template <class T, u32 R>
  void Element_Emitter<T,R>::Needed()
  {
    ElementTable<T,R>::get().RegisterElement(ELEMENT_EMITTER, &Element_Emitter<T,R>::THE_INSTANCE);
  }
}
