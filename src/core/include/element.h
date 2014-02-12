#ifndef ELEMENT_H /* -*- C++ -*- */
#define ELEMENT_H

#include "itype.h"
#include "eventwindow.h"
#include "elementtype.h"

namespace MFM
{
  template <class T, u32 R>
  class Element
  {
  private:
    
    typedef u32 (* StateFunction )(T* atom);
    
    ElementType m_type;
    
  public:
    
    Element() { }
    
    virtual void Behavior(EventWindow<T,R>& window, StateFunction f) = 0;
    
  };
}
  
#include "element.tcc"

#endif /* ELEMENT_H */
