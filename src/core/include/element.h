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
    
  protected:

    static SPoint VNNeighbors[4];
    
    void FlipSEPointToCorner(SPoint& readPt, SPoint& outPt, EuclidDir corner) const;

    bool FillAvailableVNNeighbor(EventWindow<T,R>& window, SPoint& pt) const;

    bool FillPointWithType(EventWindow<T,R>& window, 
			   SPoint& pt, SPoint* relevants, u32 relevantCount,
			   EuclidDir rotation, ElementType type) const;
      
    void Diffuse(EventWindow<T,R>& window) const;

  public:

    Element() 
    { }

    u32 GetType() const { return GetDefaultAtom().GetType(); }
    
    virtual void Behavior(EventWindow<T,R>& window) const = 0;

    virtual const T & GetDefaultAtom() const = 0;

    virtual u32 DefaultPhysicsColor() const = 0;
    
  };
}
  
#include "element.tcc"

#endif /* ELEMENT_H */
