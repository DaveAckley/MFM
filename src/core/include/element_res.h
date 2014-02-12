#include "element.h"       /* -*- C++ -*- */
#include "eventwindow.h"
#include "elementtype.h"
#include "elementtable.h"
#include "itype.h"
#include "p1atom.h"

namespace MFM
{

  template <class T, u32 R>
  class Element_Res : public Element<T,R>
  {
  public:
    static Element_Res THE_INSTANCE;

    Element_Res() {}
    
    typedef u32 (* StateFunction )(T* atom);

    virtual void Behavior(EventWindow<T,R>& window, StateFunction f)
    {
      
    }

    static void Needed();    
  };

  template <class T, u32 R>
  Element_Res<T,R> Element_Res<T,R>::THE_INSTANCE;

  template <class T, u32 R>
  void Element_Res<T,R>::Needed()
  {
    ElementTable<T,R>::get().RegisterElement(ELEMENT_RES, &Element_Res<T,R>::THE_INSTANCE);
  }
}
