#ifndef ELEMENT_H /* -*- C++ -*- */
#define ELEMENT_H

#include "itype.h"
#include "eventwindow.h"
#include "elementtype.h"
#include "eucliddir.h"

namespace MFM
{
  template <class T, u32 R>
  class Element
  {
  private:
    
    typedef u32 (* StateFunction )(T* atom);
    
    ElementType m_type;

  protected:

    static SPoint VNNeighbors[4];
    
    void ReproduceVertically(EventWindow<T,R>& window, StateFunction f,
			     ElementType type);

    void FlipSEPointToCorner(SPoint& readPt, SPoint& outPt, EuclidDir corner);

    bool FillAvailableVNNeighbor(EventWindow<T,R>& window, StateFunction f, SPoint& pt);

    bool FillPointWithType(EventWindow<T,R>& window, StateFunction f,
			   SPoint& pt, SPoint* relevants, u32 relevantCount,
			   EuclidDir rotation, ElementType type);
      
    void Diffuse(EventWindow<T,R>& window, StateFunction f);

  public:
    
    Element() { }
    
    virtual void Behavior(EventWindow<T,R>& window, StateFunction f) = 0;
    
  };
}
  
#include "element.tcc"

#endif /* ELEMENT_H */
