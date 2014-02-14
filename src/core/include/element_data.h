#include "element.h"       /* -*- C++ -*- */
#include "eventwindow.h"
#include "elementtype.h"
#include "elementtable.h"
#include "itype.h"
#include "p1atom.h"

namespace MFM
{

  template <class T, u32 R>
  class Element_Data : public Element<T,R>
  {
  public:
    static Element_Data DATA_INSTANCE;

    Element_Data() {}
    
    typedef u32 (* StateFunction )(T* atom);

    virtual void Behavior(EventWindow<T,R>& window, StateFunction f)
    {
      Diffuse(window, f);
    }

    static void Needed();    
  };

  template <class T, u32 R>
  Element_Data<T,R> Element_Data<T,R>::DATA_INSTANCE;

  template <class T, u32 R>
  void Element_Data<T,R>::Needed()
  {
    ElementTable<T,R>::get().RegisterElement(ELEMENT_DATA, &Element_Data<T,R>::DATA_INSTANCE);
  }
}
