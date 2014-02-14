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

  protected:
    
    void ReproduceVertically(EventWindow<T,R>& window, StateFunction f,
			     ElementType type);

    u32 FillAvailableVNNeighbors(EventWindow<T,R>& window, StateFunction f, SPoint* points);

    void Diffuse(EventWindow<T,R>& window, StateFunction f);

  public:
    
    Element() { }
    
    virtual void Behavior(EventWindow<T,R>& window, StateFunction f) = 0;
    
  };
}
  
#include "element.tcc"

#endif /* ELEMENT_H */
